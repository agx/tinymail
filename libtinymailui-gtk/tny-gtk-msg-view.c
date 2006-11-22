/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with self library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <glib/gi18n-lib.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <gtk/gtk.h>

#include <tny-list.h>
#include <tny-simple-list.h>
#include <tny-iterator.h>

#include <tny-gtk-msg-view.h>
#include <tny-gtk-text-buffer-stream.h>
#include <tny-gtk-attach-list-model.h>
#include <tny-header-view.h>
#include <tny-gtk-header-view.h>
#include <tny-gtk-text-mime-part-view.h>
#include <tny-gtk-attachment-mime-part-view.h>
#include <tny-mime-part-saver.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif

#include "tny-gtk-attach-list-model-priv.h"

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkMsgViewPriv TnyGtkMsgViewPriv;

struct _TnyGtkMsgViewPriv
{
	TnyMimePart *part;
	TnyHeaderView *headerview;
	GtkIconView *attachview;
	GtkWidget *attachview_sw;
	GList *unattached_views;

	gboolean display_one_body;
	gboolean display_html;
	gboolean display_plain;
	gboolean display_attachments;
	gboolean display_rfc822;

	TnyMimePartView *text_body_viewer;
};

typedef struct
{
	gulong signal;
	TnyMimePart *part;
} RealizePriv;

#define TNY_GTK_MSG_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_MSG_VIEW, TnyGtkMsgViewPriv))


static void tny_gtk_msg_view_display_parts (TnyMsgView *self, TnyList *parts);
static void remove_mime_part_viewer (TnyMimePartView *mpview, GtkContainer *mpviewers);

/**
 * tny_gtk_msg_view_get_display_one_body:
 * @self: A #TnyGtkMsgView instance
 *
 * Return value: whether or not to display only one text/html or only one text/plain mime part
 **/
gboolean 
tny_gtk_msg_view_get_display_one_body (TnyGtkMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return priv->display_one_body;
}

/**
 * tny_gtk_msg_view_get_display_html:
 * @self: A #TnyGtkMsgView instance
 *
 * Return value: whether or not to display text/html mime parts
 **/
gboolean 
tny_gtk_msg_view_get_display_html (TnyGtkMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return priv->display_html;
}

/**
 * tny_gtk_msg_view_get_display_rfc822:
 * @self: A #TnyGtkMsgView instance
 *
 * Return value: whether or not to display message/rfc822 mime parts
 **/
gboolean 
tny_gtk_msg_view_get_display_rfc822 (TnyGtkMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return priv->display_rfc822;
}

/**
 * tny_gtk_msg_view_get_display_attachments:
 * @self: A #TnyGtkMsgView instance
 *
 * Return value: whether or not to display attachments
 **/
gboolean 
tny_gtk_msg_view_get_display_attachments (TnyGtkMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return priv->display_attachments;
}

/**
 * tny_gtk_msg_view_get_display_plain:
 * @self: A #TnyGtkMsgView instance
 *
 * Return value: whether or not to display text/plain mime parts
 **/
gboolean 
tny_gtk_msg_view_get_display_plain (TnyGtkMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return priv->display_plain;
}



/**
 * tny_gtk_msg_view_set_display_one_body:
 * @self: A #TnyGtkMsgView instance
 * @setting: whether or not to display only one text/html or only one text/plain mime part
 *
 * With this setting will the default implementation of #TnyGtkMsgView display
 * only one text/html or only one text/plain mime part. Default value is FALSE.
 *
 * Note that these settings only affect the instance in case an overridden
 * implementation of tny_msg_view_create_mime_part_view_for doesn't handle
 * creating a viewer for a mime part.
 * 
 * So for example in case a more advanced implementation that inherits this
 * type implements viewing a text/html mime part, and will therefore not call
 * this types original tny_msg_view_create_mime_part_view_for method for the
 * mime part anymore, the setting isn't used.
 **/
void 
tny_gtk_msg_view_set_display_one_body (TnyGtkMsgView *self, gboolean setting)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	priv->display_one_body = setting;
	return;
}

/**
 * tny_gtk_msg_view_set_display_html:
 * @self: A #TnyGtkMsgView instance
 * @setting: whether or not to display text/html mime parts
 *
 * With this setting will the default implementation of #TnyGtkMsgView display
 * the HTML source code of text/html mime parts. Default is FALSE.
 *
 * Note that these settings only affect the instance in case an overridden
 * implementation of tny_msg_view_create_mime_part_view_for doesn't handle
 * creating a viewer for a mime part.
 * 
 * So for example in case a more advanced implementation that inherits this
 * type implements viewing a text/html mime part, and will therefore not call
 * this types original tny_msg_view_create_mime_part_view_for method for the
 * mime part anymore, the setting isn't used.
 **/
void 
tny_gtk_msg_view_set_display_html (TnyGtkMsgView *self, gboolean setting)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	priv->display_html = setting;
	return;
}

/**
 * tny_gtk_msg_view_set_display_rfc822:
 * @self: A #TnyGtkMsgView instance
 * @setting: whether or not to display message/rfc822 mime parts
 *
 * With this setting will the default implementation of #TnyGtkMsgView display
 * RFC822 inline message mime parts (forwards). Default is FALSE.
 *
 * (Note that this support is unimplemented at this moment)
 *
 * Note that these settings only affect the instance in case an overridden
 * implementation of tny_msg_view_create_mime_part_view_for doesn't handle
 * creating a viewer for a mime part.
 * 
 * So for example in case a more advanced implementation that inherits this
 * type implements viewing a text/html mime part, and will therefore not call
 * this types original tny_msg_view_create_mime_part_view_for method for the
 * mime part anymore, the setting isn't used.
 **/
void 
tny_gtk_msg_view_set_display_rfc822 (TnyGtkMsgView *self, gboolean setting)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	priv->display_rfc822 = setting;
	return;
}


/**
 * tny_gtk_msg_view_set_display_attachments:
 * @self: A #TnyGtkMsgView instance
 * @setting: whether or not to display attachment mime parts
 *
 * With this setting will the default implementation of #TnyGtkMsgView display
 * attachments using a #GtkIconList and the #TnyGtkAttachListModel at the bottom
 * of the #TnyGtkMsgView's scrollwindow. Default is TRUE.
 *
 * Note that these settings only affect the instance in case an overridden
 * implementation of tny_msg_view_create_mime_part_view_for doesn't handle
 * creating a viewer for a mime part.
 * 
 * So for example in case a more advanced implementation that inherits this
 * type implements viewing a text/html mime part, and will therefore not call
 * this types original tny_msg_view_create_mime_part_view_for method for the
 * mime part anymore, the setting isn't used.
 **/
void 
tny_gtk_msg_view_set_display_attachments (TnyGtkMsgView *self, gboolean setting)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	priv->display_attachments = setting;
	return;
}

/**
 * tny_gtk_msg_view_set_display_plain:
 * @self: A #TnyGtkMsgView instance
 * @setting: whether or not to display text/plain mime parts
 *
 * With this setting will the default implementation of #TnyGtkMsgView display
 * text/plain mime parts. Default is TRUE.
 *
 * Note that these settings only affect the instance in case an overridden
 * implementation of tny_msg_view_create_mime_part_view_for doesn't handle
 * creating a viewer for a mime part.
 * 
 * So for example in case a more advanced implementation that inherits this
 * type implements viewing a text/html mime part, and will therefore not call
 * this types original tny_msg_view_create_mime_part_view_for method for the
 * mime part anymore, the setting isn't used.
 **/
void 
tny_gtk_msg_view_set_display_plain (TnyGtkMsgView *self, gboolean setting)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	priv->display_plain = setting;
	return;
}



static void
tny_gtk_msg_view_set_unavailable (TnyMsgView *self)
{
	TNY_GTK_MSG_VIEW_GET_CLASS (self)->set_unavailable_func (self);
}

static void
tny_gtk_msg_view_set_unavailable_default (TnyMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkTextBuffer *buffer;

	tny_msg_view_clear (self);

	tny_header_view_clear (priv->headerview);
	gtk_widget_hide (GTK_WIDGET (priv->headerview));

	return;
}


static TnyMsg*
tny_gtk_msg_view_get_msg (TnyMsgView *self)
{
	return TNY_GTK_MSG_VIEW_GET_CLASS (self)->get_msg_func (self);
}

static TnyMsg* 
tny_gtk_msg_view_get_msg_default (TnyMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	return (priv->part && TNY_IS_MSG (priv->part))?TNY_MSG (g_object_ref (priv->part)):NULL;
}


static TnyMimePartView*
tny_gtk_msg_view_create_mime_part_view_for (TnyMsgView *self, TnyMimePart *part)
{
	return TNY_GTK_MSG_VIEW_GET_CLASS (self)->create_mime_part_view_for_func (self, part);
}

static TnyMimePartView*
tny_gtk_msg_view_create_mime_part_view_for_default (TnyMsgView *self, TnyMimePart *part)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	TnyMimePartView *retval = NULL;

	g_assert (TNY_IS_MIME_PART (part));

	/* PLAIN mime part */
	if (priv->display_plain && tny_mime_part_content_type_is (part, "text/plain"))
	{
		retval = tny_gtk_text_mime_part_view_new ();

	/* HTML mime part (shows HTML source code) (should be overridden in case there's
	   support for a HTML TnyMsgView (like the TnyMozEmbedMsgView) */
	} else if (priv->display_html && tny_mime_part_content_type_is (part, "text/html"))
	{
		retval = tny_gtk_text_mime_part_view_new ();

	/* Inline message RFC822 */
	} else if (priv->display_rfc822 && tny_mime_part_content_type_is (part, "message/rfc822"))
	{
		retval = TNY_MIME_PART_VIEW (tny_gtk_msg_view_new ());
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (retval),
					GTK_POLICY_NEVER, GTK_POLICY_NEVER);

	/* Attachments */
	} else if (priv->display_attachments && tny_mime_part_get_content_type (part) &&
				tny_mime_part_is_attachment (part))
	{
		static gboolean first = TRUE;
		GtkTreeModel *model;

		gtk_widget_show (priv->attachview_sw);
		gtk_widget_show (GTK_WIDGET (priv->attachview));

		if (first)
		{
			model = tny_gtk_attach_list_model_new ();
			gtk_icon_view_set_model (priv->attachview, model);
			first = FALSE;
		} else {
			model = gtk_icon_view_get_model (priv->attachview);
			if (!model || !TNY_IS_LIST (model))
			{
				model = tny_gtk_attach_list_model_new ();
				gtk_icon_view_set_model (priv->attachview, model);
			}
		}

		retval = tny_gtk_attachment_mime_part_view_new (TNY_GTK_ATTACH_LIST_MODEL (model));
	}

	return retval;
}


static void
tny_mime_part_view_proxy_func_set_part (TnyMimePartView *mpview, TnyMimePart *part)
{
		if (tny_mime_part_content_type_is (part, "message/rfc822"))
		{
			TnyList *list = tny_simple_list_new ();

			if (TNY_IS_MSG (part) && TNY_IS_GTK_MSG_VIEW (mpview))
			{
				TnyGtkMsgViewPriv *mppriv = TNY_GTK_MSG_VIEW_GET_PRIVATE (mpview);
				TnyHeader *header = TNY_HEADER (tny_msg_get_header (TNY_MSG (part)));
				if (header && TNY_IS_HEADER (header))
				{
					tny_header_view_set_header (mppriv->headerview, header);
					g_object_unref (G_OBJECT (header));
					gtk_widget_show (GTK_WIDGET (mppriv->headerview));
				}
			}

			tny_mime_part_get_parts (part, list);
			tny_gtk_msg_view_display_parts (TNY_MSG_VIEW (mpview), list);
			g_object_unref (G_OBJECT (list));
		} else
			tny_mime_part_view_set_part (mpview, part);
}

static void
on_mpview_realize (GtkWidget *widget, gpointer user_data)
{
	RealizePriv *prv = user_data;

	tny_mime_part_view_proxy_func_set_part (TNY_MIME_PART_VIEW (widget), prv->part);
	g_signal_handler_disconnect (widget, prv->signal);
	g_object_unref (prv->part);

	g_slice_free (RealizePriv, prv);
}


static void
tny_gtk_msg_view_display_part (TnyMsgView *self, TnyMimePart *part)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	TnyMimePartView *mpview = NULL;
	gboolean doit = TRUE;

	if (priv->display_one_body)
	{
		if (priv->text_body_viewer && tny_mime_part_content_type_is (part, "text/plain"))
			return;

		mpview = tny_msg_view_create_mime_part_view_for (TNY_MSG_VIEW (self), part);
		if (tny_mime_part_content_type_is (part, "text/html") &&
			!TNY_IS_GTK_TEXT_MIME_PART_VIEW (mpview))
		{   
			/* An enhanced HTML component? */
			if (priv->text_body_viewer)
			{
				GtkContainer *viewers = GTK_CONTAINER (TNY_GTK_MSG_VIEW (self)->viewers);
				remove_mime_part_viewer (priv->text_body_viewer, viewers);
			}
			priv->text_body_viewer = mpview;
		}
	}


	if (!mpview)
		mpview = tny_msg_view_create_mime_part_view_for (TNY_MSG_VIEW (self), part);

	if (mpview) 
	{
		if (GTK_IS_WIDGET (mpview))
		{

			if (TNY_IS_GTK_MSG_VIEW (mpview))
			{
				TnyMsg *msg = NULL;
				TnyHeader *header = NULL;
				gchar *str = NULL;
				GtkWidget *expander = NULL;

				msg = tny_msg_view_get_msg (TNY_MSG_VIEW (mpview));
				if (msg)
					header = tny_msg_get_header (msg);
				if (header)
					str = g_strdup_printf (_("Email message attachment, \"%s\""), 
								tny_header_get_subject (header));
				else
					str = g_strdup_printf (_("Email message attachment"));

				expander = gtk_expander_new (str);
				if (header)
					g_object_unref (G_OBJECT (header));
				if (msg)
					g_object_unref (G_OBJECT (msg));
				g_free (str);
				gtk_expander_set_expanded (GTK_EXPANDER (expander), FALSE);
				gtk_expander_set_spacing (GTK_EXPANDER (expander), 7);
				gtk_container_add (GTK_CONTAINER (expander), GTK_WIDGET (mpview));
				gtk_widget_show (GTK_WIDGET (expander));
				gtk_box_pack_end (GTK_BOX (TNY_GTK_MSG_VIEW (self)->viewers), 
					GTK_WIDGET (expander), TRUE, TRUE, 0);
			} else
				gtk_box_pack_end (GTK_BOX (TNY_GTK_MSG_VIEW (self)->viewers), 
						GTK_WIDGET (mpview), TRUE, TRUE, 0);

			gtk_widget_show (GTK_WIDGET (mpview));
			if (!GTK_WIDGET_REALIZED (mpview))
			{
				RealizePriv *prv = g_slice_new (RealizePriv);
				prv->part = g_object_ref (part);
				prv->signal = g_signal_connect (G_OBJECT (mpview),
					"realize", G_CALLBACK (on_mpview_realize), prv);
			} else
				tny_mime_part_view_proxy_func_set_part (mpview, part);
		} else if (TNY_IS_GTK_ATTACHMENT_MIME_PART_VIEW (mpview)) 
			tny_mime_part_view_proxy_func_set_part (mpview, part);
		else if (!TNY_IS_GTK_ATTACHMENT_MIME_PART_VIEW (mpview)) 
		{
			priv->unattached_views = g_list_prepend (priv->unattached_views, mpview);
			tny_mime_part_view_proxy_func_set_part (mpview, part);
		}
	}
}

static void
tny_gtk_msg_view_display_parts (TnyMsgView *self, TnyList *parts)
{
	TnyIterator *iterator = tny_list_create_iterator (parts);

	while (!tny_iterator_is_done (iterator))
	{
		TnyMimePart *part = (TnyMimePart*)tny_iterator_get_current (iterator);
		tny_gtk_msg_view_display_part (self, part);
		g_object_unref (G_OBJECT (part));
		tny_iterator_next (iterator);
	}

	g_object_unref (G_OBJECT (iterator));

}

static void
tny_gtk_msg_view_set_msg (TnyMsgView *self, TnyMsg *msg)
{
	TNY_GTK_MSG_VIEW_GET_CLASS (self)->set_msg_func (self, msg);
}

static void 
tny_gtk_msg_view_set_msg_default (TnyMsgView *self, TnyMsg *msg)
{

	tny_mime_part_view_set_part (TNY_MIME_PART_VIEW (self), TNY_MIME_PART (msg));

	return;
}

static void
remove_mime_part_viewer (TnyMimePartView *mpview, GtkContainer *mpviewers)
{
	gtk_container_remove (mpviewers, GTK_WIDGET (mpview));
	return;
}

static void
tny_gtk_msg_view_clear (TnyMsgView *self)
{
	TNY_GTK_MSG_VIEW_GET_CLASS (self)->clear_func (self);
}

static void
clear_prv (TnyGtkMsgViewPriv *priv)
{
	g_list_foreach (priv->unattached_views, (GFunc)g_object_unref, NULL);
	g_list_free (priv->unattached_views);
	priv->unattached_views = NULL;

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));
	priv->part = NULL;
}

static void
tny_gtk_msg_view_clear_default (TnyMsgView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkContainer *viewers = GTK_CONTAINER (TNY_GTK_MSG_VIEW (self)->viewers);
	GList *kids = gtk_container_get_children (viewers);

	priv->text_body_viewer = NULL;
	g_list_foreach (kids, (GFunc)remove_mime_part_viewer, viewers);
	g_list_free (kids);

	clear_prv (priv);

	gtk_icon_view_set_model (priv->attachview, tny_gtk_attach_list_model_new ());
	gtk_widget_hide (priv->attachview_sw);
	tny_header_view_set_header (priv->headerview, NULL);
	gtk_widget_hide (GTK_WIDGET (priv->headerview));

	return;
}



static void 
tny_gtk_msg_view_mp_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TNY_GTK_MSG_VIEW_GET_CLASS (self)->set_part_func (self, part);
	return;
}


static void 
tny_gtk_msg_view_mp_set_part_default (TnyMimePartView *self, TnyMimePart *part)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	tny_msg_view_clear (self);

	if (part)
	{
		TnyIterator *iterator;
		TnyList *list;

		g_assert (TNY_IS_MIME_PART (part));

		if (TNY_IS_MSG (part))
		{
			TnyHeader *header = TNY_HEADER (tny_msg_get_header (TNY_MSG (part)));
			if (header && TNY_IS_HEADER (header))
			{
				tny_header_view_set_header (priv->headerview, header);
				g_object_unref (G_OBJECT (header));
				gtk_widget_show (GTK_WIDGET (priv->headerview));
			}
		}

		list = tny_simple_list_new ();
		priv->part = g_object_ref (G_OBJECT (part));

		tny_gtk_msg_view_display_part (TNY_MSG_VIEW (self), part);
		tny_mime_part_get_parts (part, list);
		tny_gtk_msg_view_display_parts (TNY_MSG_VIEW (self), list);
		g_object_unref (G_OBJECT (list));
	}

	return;
}


static TnyMimePart* 
tny_gtk_msg_view_mp_get_part (TnyMimePartView *self)
{
	return TNY_GTK_MSG_VIEW_GET_CLASS (self)->get_part_func (self);
}


static TnyMimePart* 
tny_gtk_msg_view_mp_get_part_default (TnyMimePartView *self)
{
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	return TNY_MIME_PART (g_object_ref (priv->part));
}

static void 
tny_gtk_msg_view_mp_clear (TnyMimePartView *self)
{
	tny_msg_view_clear (TNY_MSG_VIEW (self));
}


/**
 * tny_gtk_msg_view_new:
 *
 * Return value: a new #TnyMsgView instance implemented for Gtk+
 **/
TnyMsgView*
tny_gtk_msg_view_new (void)
{
	TnyGtkMsgView *self = g_object_new (TNY_TYPE_GTK_MSG_VIEW, NULL);

	return TNY_MSG_VIEW (self);
}

static void
tny_gtk_msg_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkMsgView *self = (TnyGtkMsgView *)instance;
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);
	GtkWidget *vbox = gtk_vbox_new (FALSE, 0);

	/* Defaults */
	priv->display_html = FALSE;
	priv->display_plain = TRUE;
	priv->display_attachments = TRUE;
	priv->display_rfc822 = TRUE;
	priv->display_one_body = FALSE;

	priv->text_body_viewer = NULL;

	priv->unattached_views = NULL;
	priv->part = NULL;

	gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (self), NULL);
	gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (self), NULL);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self), 
			GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	priv->headerview = tny_gtk_header_view_new ();
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (priv->headerview), FALSE, FALSE, 0);

	TNY_GTK_MSG_VIEW (self)->viewers = GTK_CONTAINER (gtk_vbox_new (FALSE, 0));
	gtk_box_pack_start (GTK_BOX (vbox), 
		GTK_WIDGET (TNY_GTK_MSG_VIEW (self)->viewers), FALSE, FALSE, 0);
	gtk_widget_show (GTK_WIDGET (TNY_GTK_MSG_VIEW (self)->viewers));

	priv->attachview_sw = gtk_scrolled_window_new (NULL, NULL);

	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (self),
					GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (priv->attachview_sw),
					GTK_SHADOW_NONE);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (priv->attachview_sw),
					GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	priv->attachview = GTK_ICON_VIEW (gtk_icon_view_new ());

	gtk_icon_view_set_selection_mode (priv->attachview, GTK_SELECTION_SINGLE);

	gtk_icon_view_set_text_column (priv->attachview,
			TNY_GTK_ATTACH_LIST_MODEL_FILENAME_COLUMN);

	gtk_icon_view_set_pixbuf_column (priv->attachview,
			TNY_GTK_ATTACH_LIST_MODEL_PIXBUF_COLUMN);

	gtk_icon_view_set_columns (priv->attachview, -1);
	gtk_icon_view_set_item_width (priv->attachview, 100);
	gtk_icon_view_set_column_spacing (priv->attachview, 10);

	gtk_box_pack_start (GTK_BOX (vbox), priv->attachview_sw, FALSE, TRUE, 0);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (self), 
			GTK_WIDGET (vbox));
	gtk_container_add (GTK_CONTAINER (priv->attachview_sw), GTK_WIDGET (priv->attachview));

	gtk_widget_show (GTK_WIDGET (vbox));
	gtk_widget_hide (GTK_WIDGET (priv->headerview));
	gtk_widget_show (GTK_WIDGET (priv->attachview));

	return;
}

static void
tny_gtk_msg_view_finalize (GObject *object)
{
	TnyGtkMsgView *self = (TnyGtkMsgView *)object;	
	TnyGtkMsgViewPriv *priv = TNY_GTK_MSG_VIEW_GET_PRIVATE (self);

	clear_prv (priv);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_msg_view_init (gpointer g, gpointer iface_data)
{
	TnyMsgViewIface *klass = (TnyMsgViewIface *)g;

	klass->get_msg_func = tny_gtk_msg_view_get_msg;
	klass->set_msg_func = tny_gtk_msg_view_set_msg;
	klass->set_unavailable_func = tny_gtk_msg_view_set_unavailable;
	klass->clear_func = tny_gtk_msg_view_clear;
	klass->create_mime_part_view_for_func = tny_gtk_msg_view_create_mime_part_view_for;

	return;
}


static void
tny_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->get_part_func = tny_gtk_msg_view_mp_get_part;
	klass->set_part_func = tny_gtk_msg_view_mp_set_part;
	klass->clear_func = tny_gtk_msg_view_mp_clear;

	return;
}


static void 
tny_gtk_msg_view_class_init (TnyGtkMsgViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_msg_view_finalize;

	class->get_part_func = tny_gtk_msg_view_mp_get_part_default;
	class->set_part_func = tny_gtk_msg_view_mp_set_part_default;
	class->get_msg_func = tny_gtk_msg_view_get_msg_default;
	class->set_msg_func = tny_gtk_msg_view_set_msg_default;
	class->set_unavailable_func = tny_gtk_msg_view_set_unavailable_default;
	class->clear_func = tny_gtk_msg_view_clear_default;
	class->create_mime_part_view_for_func = tny_gtk_msg_view_create_mime_part_view_for_default;
	
	g_type_class_add_private (object_class, sizeof (TnyGtkMsgViewPriv));

	return;
}

GType 
tny_gtk_msg_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkMsgViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_msg_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkMsgView),
		  0,      /* n_preallocs */
		  tny_gtk_msg_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_msg_view_info = 
		{
		  (GInterfaceInitFunc) tny_msg_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		static const GInterfaceInfo tny_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_SCROLLED_WINDOW,
			"TnyGtkMsgView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_mime_part_view_info);

		g_type_add_interface_static (type, TNY_TYPE_MSG_VIEW, 
			&tny_msg_view_info);
	}

	return type;
}
