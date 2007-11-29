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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <gtk/gtk.h>

#include <tny-gtk-expander-mime-part-view.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkExpanderMimePartViewPriv TnyGtkExpanderMimePartViewPriv;

struct _TnyGtkExpanderMimePartViewPriv
{
	TnyMimePart *part;
	TnyMimePartView *decorated;
	gboolean must_set_on_expand;
};

#define TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_EXPANDER_MIME_PART_VIEW, TnyGtkExpanderMimePartViewPriv))


static TnyMimePart*
tny_gtk_expander_mime_part_view_get_part (TnyMimePartView *self)
{
	return TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_CLASS (self)->get_part_func (self);
}

static TnyMimePart*
tny_gtk_expander_mime_part_view_get_part_default (TnyMimePartView *self)
{
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);
	return tny_mime_part_view_get_part (priv->decorated);
}

static void 
tny_gtk_expander_mime_part_view_set_part (TnyMimePartView *self, TnyMimePart *part)
{
	TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_CLASS (self)->set_part_func (self, part);
	return;
}

static void
expander_callback (GtkExpander *self, GParamSpec *param_spec, gpointer user_data)
{
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->must_set_on_expand && gtk_expander_get_expanded (self)) {
		tny_mime_part_view_set_part (priv->decorated, priv->part);
		priv->must_set_on_expand = FALSE;
	}

	return;
}

static void 
tny_gtk_expander_mime_part_view_set_part_default (TnyMimePartView *self, TnyMimePart *part)
{
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);

	priv->part = TNY_MIME_PART (g_object_ref (part));
	if (gtk_expander_get_expanded (GTK_EXPANDER (self))) {
		tny_mime_part_view_set_part (priv->decorated, priv->part);
		priv->must_set_on_expand = FALSE;
	} else
		priv->must_set_on_expand = TRUE;

	return;
}

static void
tny_gtk_expander_mime_part_view_clear (TnyMimePartView *self)
{
	TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_CLASS (self)->clear_func (self);
	return;
}

static void
tny_gtk_expander_mime_part_view_clear_default (TnyMimePartView *self)
{
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);
	tny_mime_part_view_clear (priv->decorated);
	return;
}

/**
 * tny_gtk_expander_mime_part_view_set_view:
 * @self: a #TnyGtkMsgView instance
 * @view: a #TnyMimePartView to decorate
 *
 * Set the @view to decorate with @self. The @view instance must inherit the 
 * #GtkWidget type.
 **/
void
tny_gtk_expander_mime_part_view_set_view (TnyGtkExpanderMimePartView *self, TnyMimePartView *view)
{
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->decorated)
		gtk_container_remove (GTK_CONTAINER (self), GTK_WIDGET (priv->decorated));

	priv->decorated = view;

	/* This adds a reference to msgview (it's a gtkwidget) */
	gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (view));

	gtk_widget_show (GTK_WIDGET (priv->decorated));

	return;
}

/**
 * tny_gtk_expander_mime_part_view_new:
 * @view: the #TnyMimePartView to make expandable
 *
 * Create a new #TnyMimePartView for Gtk+ that wraps another mime part view with
 * an expander (#GtkExpander). The @view instance must inherit the 
 * #GtkWidget type. The returned value will inherit #GtkExpander.
 *
 * Return value: a new #TnyMimePartView instance implemented for Gtk+
 **/
TnyMimePartView*
tny_gtk_expander_mime_part_view_new (TnyMimePartView *view)
{
	TnyGtkExpanderMimePartView *self = g_object_new (TNY_TYPE_GTK_EXPANDER_MIME_PART_VIEW, NULL);

	tny_gtk_expander_mime_part_view_set_view (self, view);

	return TNY_MIME_PART_VIEW (self);
}

static void
tny_gtk_expander_mime_part_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkExpanderMimePartView *self = (TnyGtkExpanderMimePartView *)instance;
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);

	priv->part = NULL;
	priv->decorated = NULL;
	priv->must_set_on_expand = FALSE;

	g_signal_connect (self, "notify::expanded",
		G_CALLBACK (expander_callback), self);

	return;
}

static void
tny_gtk_expander_mime_part_view_finalize (GObject *object)
{
	TnyGtkExpanderMimePartView *self = (TnyGtkExpanderMimePartView *)object;
	TnyGtkExpanderMimePartViewPriv *priv = TNY_GTK_EXPANDER_MIME_PART_VIEW_GET_PRIVATE (self);

	if (priv->part)
		g_object_unref (priv->part);

	(*parent_class->finalize) (object);

	return;
}

static void
tny_mime_part_view_init (gpointer g, gpointer iface_data)
{
	TnyMimePartViewIface *klass = (TnyMimePartViewIface *)g;

	klass->get_part_func = tny_gtk_expander_mime_part_view_get_part;
	klass->set_part_func = tny_gtk_expander_mime_part_view_set_part;
	klass->clear_func = tny_gtk_expander_mime_part_view_clear;

	return;
}

static void 
tny_gtk_expander_mime_part_view_class_init (TnyGtkExpanderMimePartViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	class->get_part_func = tny_gtk_expander_mime_part_view_get_part_default;
	class->set_part_func = tny_gtk_expander_mime_part_view_set_part_default;
	class->clear_func = tny_gtk_expander_mime_part_view_clear_default;

	object_class->finalize = tny_gtk_expander_mime_part_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkExpanderMimePartViewPriv));

	return;
}

GType 
tny_gtk_expander_mime_part_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkExpanderMimePartViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_expander_mime_part_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkExpanderMimePartView),
		  0,      /* n_preallocs */
		  tny_gtk_expander_mime_part_view_instance_init,    /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_mime_part_view_info = 
		{
		  (GInterfaceInitFunc) tny_mime_part_view_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_EXPANDER,
			"TnyGtkExpanderMimePartView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_MIME_PART_VIEW, 
			&tny_mime_part_view_info);

	}

	return type;
}
