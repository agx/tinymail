/* libtinymailui-mozembed - The Tiny Mail UI library for Gtk+
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

#include "mozilla-preferences.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <string.h>
#include <gtk/gtk.h>

#include <tny-moz-embed-html-mime-part-view.h>
#include <tny-moz-embed-stream.h>

#ifdef GNOME
#include <tny-vfs-stream.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#else
#include <tny-fs-stream.h>
#endif

static GObjectClass *parent_class = NULL;

typedef struct _TnyMozEmbedHtmlMimePartViewPriv TnyMozEmbedHtmlMimePartViewPriv;

struct _TnyMozEmbedHtmlMimePartViewPriv
{
	TnyMimePart *part;
};

#define TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MOZ_EMBED_HTML_MIME_PART_VIEW, TnyMozEmbedHtmlMimePartViewPriv))


static TnyMimePart*
tny_moz_embed_html_mime_part_view_get_part (TnyMimePartView *self)
{
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);
	return (priv->part)?TNY_MIME_PART (g_object_ref (priv->part)):NULL;
}


static void 
tny_moz_embed_html_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	if (part)
	{
		TnyStream *dest;

		dest = tny_moz_embed_stream_new (GTK_MOZ_EMBED (self));
		tny_stream_reset (dest);
		tny_mime_part_decode_to_stream (part, dest);
		g_object_unref (G_OBJECT (dest));

		g_object_ref (G_OBJECT (part));
		priv->part = part;
	}

	return;
}

static void
tny_moz_embed_html_mime_part_view_clear (TnyMimePartView *self)
{

	gtk_moz_embed_render_data (GTK_MOZ_EMBED (self), "",  0, "file:///", "text/html");

	return;
}

/**
 * tny_moz_embed_html_mime_part_view_new:
 *
 * Return value: a new #TnyMimePartView instance implemented for Gtk+
 **/
TnyMimePartView*
tny_moz_embed_html_mime_part_view_new (void)
{
	TnyMozEmbedHtmlMimePartView *self = g_object_new (TNY_TYPE_MOZ_EMBED_HTML_MIME_PART_VIEW, NULL);

	return TNY_MIME_PART_VIEW (self);
}

static gint 
open_uri_cb (GtkMozEmbed *embed, const char *uri, gpointer data)
{
	return 1;
}

static void 
new_window_cb (GtkMozEmbed *embed, GtkMozEmbed **retval, guint chromemask, gpointer data)
{
	*retval = NULL;
}

static void
tny_moz_embed_html_mime_part_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMozEmbedHtmlMimePartView *self  = (TnyMozEmbedHtmlMimePartView*) instance;
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);
	
	gtk_moz_embed_set_chrome_mask (GTK_MOZ_EMBED (self), 
			GTK_MOZ_EMBED_FLAG_DEFAULTCHROME|GTK_MOZ_EMBED_FLAG_SCROLLBARSON);

	g_signal_connect (G_OBJECT (self), "new_window",
		G_CALLBACK (new_window_cb), self);

	g_signal_connect (G_OBJECT (self), "open_uri",
		G_CALLBACK (open_uri_cb), self);

	return;
}

static void
tny_moz_embed_html_mime_part_view_finalize (GObject *object)
{
	TnyMozEmbedHtmlMimePartView *self = (TnyMozEmbedHtmlMimePartView *)object;	
	TnyMozEmbedHtmlMimePartViewPriv *priv = TNY_MOZ_EMBED_HTML_MIME_PART_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->part))
		g_object_unref (G_OBJECT (priv->part));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->get_part_func = tny_moz_embed_html_mime_part_view_get_part;
	klass->set_part_func = tny_moz_embed_html_mime_part_view_set_part;
	klass->clear_func = tny_moz_embed_html_mime_part_view_clear;

	return;
}

static void 
tny_moz_embed_html_mime_part_view_class_init (TnyMozEmbedHtmlMimePartViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moz_embed_html_mime_part_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMozEmbedHtmlMimePartViewPriv));

	gtk_moz_embed_push_startup ();

	mozilla_preference_set_int ("permissions.default.image", 2);
	mozilla_preference_set_int ("permissions.default.script", 2);
	mozilla_preference_set_boolean ("security.checkloaduri", FALSE);
	mozilla_preference_set ("general.useragent.misc", "Tinymail/" VERSION);
	mozilla_preference_set ("network.proxy.no_proxies_on", "localhost");


	return;
}

GType 
tny_moz_embed_html_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMozEmbedHtmlMimePartViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_moz_embed_html_mime_part_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMozEmbedHtmlMimePartView),
		  0,      /* n_preallocs */
		  tny_moz_embed_html_mime_part_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_MOZ_EMBED,
			"TnyMozEmbedHtmlMimePartView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_mime_part_view_info);
	}

	return type;
}

