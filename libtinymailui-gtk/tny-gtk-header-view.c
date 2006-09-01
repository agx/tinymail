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

#include <string.h>
#include <gtk/gtk.h>
#include <tny-gtk-header-view.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkHeaderViewPriv TnyGtkHeaderViewPriv;

struct _TnyGtkHeaderViewPriv
{
	TnyHeaderIface *header;
	GtkWidget *from_label;
	GtkWidget *to_label;
	GtkWidget *subject_label;
	GtkWidget *date_label;
};

#define TNY_GTK_HEADER_VIEW_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_HEADER_VIEW, TnyGtkHeaderViewPriv))


/* TODO: refactor */
static gchar *
_get_readable_date (time_t file_time_raw)
{
	struct tm *file_time;
	gchar readable_date[64];
	gsize readable_date_size;

	file_time = localtime (&file_time_raw);

	readable_date_size = strftime (readable_date, 63, _("%Y-%m-%d, %-I:%M %p"), file_time);		
	
	return g_strdup (readable_date);
}


static void 
tny_gtk_header_view_set_header (TnyHeaderViewIface *self, TnyHeaderIface *header)
{
	TnyGtkHeaderViewPriv *priv = TNY_GTK_HEADER_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->header))
 		g_object_unref (G_OBJECT (priv->header));
	priv->header = NULL;

	if (header && G_IS_OBJECT (header))
	{
	    	gchar *str;
		g_object_ref (G_OBJECT (header)); 
		priv->header = header;

		gtk_label_set_text (GTK_LABEL (priv->to_label), tny_header_iface_get_to (header));
		gtk_label_set_text (GTK_LABEL (priv->from_label), tny_header_iface_get_from (header));
		gtk_label_set_text (GTK_LABEL (priv->subject_label), tny_header_iface_get_subject (header));

		str = _get_readable_date (tny_header_iface_get_date_sent (header));
		gtk_label_set_text (GTK_LABEL (priv->date_label), (const gchar*)str);
		g_free (str);
	}
    
	return;
}


static void 
tny_gtk_header_view_clear (TnyHeaderViewIface *self)
{
	TnyGtkHeaderViewPriv *priv = TNY_GTK_HEADER_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->header))
		g_object_unref (G_OBJECT (priv->header));
	priv->header = NULL;
    
	gtk_label_set_text (GTK_LABEL (priv->to_label), "");
	gtk_label_set_text (GTK_LABEL (priv->from_label), "");
	gtk_label_set_text (GTK_LABEL (priv->subject_label), "");
	gtk_label_set_text (GTK_LABEL (priv->date_label), "");
    
	return;
}


/**
 * tny_gtk_header_view_new:
 *
 * Return value: a new #TnyHeaderViewIface instance implemented for Gtk+
 **/
TnyHeaderViewIface*
tny_gtk_header_view_new (void)
{
	TnyGtkHeaderView *self = g_object_new (TNY_TYPE_GTK_HEADER_VIEW, NULL);

	return TNY_HEADER_VIEW_IFACE (self);
}

static void
tny_gtk_header_view_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkHeaderView *self = (TnyGtkHeaderView *)instance;
	TnyGtkHeaderViewPriv *priv = TNY_GTK_HEADER_VIEW_GET_PRIVATE (self);
	GtkWidget *label2, *label3, *label4, *label1;

    	priv->header = NULL;
    
	gtk_table_set_homogeneous (GTK_TABLE (self), FALSE);
	gtk_table_resize (GTK_TABLE (self), 4, 2);

	gtk_table_set_col_spacings (GTK_TABLE (self), 4);

	label2 = gtk_label_new (_("<b>to:</b>"));
	gtk_widget_show (label2);
	gtk_table_attach (GTK_TABLE (self), label2, 0, 1, 1, 2,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_use_markup (GTK_LABEL (label2), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);

	label3 = gtk_label_new (_("<b>Subject:</b>"));
	gtk_widget_show (label3);
	gtk_table_attach (GTK_TABLE (self), label3, 0, 1, 2, 3,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_use_markup (GTK_LABEL (label3), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

	label4 = gtk_label_new (_("<b>Date:</b>"));
	gtk_widget_show (label4);
	gtk_table_attach (GTK_TABLE (self), label4, 0, 1, 3, 4,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_use_markup (GTK_LABEL (label4), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);

	priv->from_label = gtk_label_new ("");
	gtk_widget_show (priv->from_label);
	gtk_table_attach (GTK_TABLE (self), priv->from_label, 1, 2, 0, 1,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (priv->from_label), 0, 0.5);

	priv->to_label = gtk_label_new ("");
	gtk_widget_show (priv->to_label);
	gtk_table_attach (GTK_TABLE (self), priv->to_label, 1, 2, 1, 2,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (priv->to_label), 0, 0.5);

	priv->subject_label = gtk_label_new ("");
	gtk_widget_show (priv->subject_label);
	gtk_table_attach (GTK_TABLE (self), priv->subject_label, 1, 2, 2, 3,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (priv->subject_label), 0, 0.5);

	priv->date_label = gtk_label_new ("");
	gtk_widget_show (priv->date_label);
	gtk_table_attach (GTK_TABLE (self), priv->date_label, 1, 2, 3, 4,
			  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_misc_set_alignment (GTK_MISC (priv->date_label), 0, 0.5);

	label1 = gtk_label_new (_("<b>From:</b>"));
	gtk_widget_show (label1);
	gtk_table_attach (GTK_TABLE (self), label1, 0, 1, 0, 1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_use_markup (GTK_LABEL (label1), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

	return;
}

static void
tny_gtk_header_view_finalize (GObject *object)
{
	TnyGtkHeaderView *self = (TnyGtkHeaderView *)object;	
	TnyGtkHeaderViewPriv *priv = TNY_GTK_HEADER_VIEW_GET_PRIVATE (self);

	if (G_LIKELY (priv->header))
		g_object_unref (G_OBJECT (priv->header));
	priv->header = NULL;
    
	(*parent_class->finalize) (object);

	return;
}

static void
tny_header_view_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyHeaderViewIfaceClass *klass = (TnyHeaderViewIfaceClass *)g_iface;

	klass->set_header_func = tny_gtk_header_view_set_header;
	klass->clear_func = tny_gtk_header_view_clear;
    
	return;
}

static void 
tny_gtk_header_view_class_init (TnyGtkHeaderViewClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_header_view_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkHeaderViewPriv));

	return;
}

GType 
tny_gtk_header_view_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkHeaderViewClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_header_view_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkHeaderView),
		  0,      /* n_preallocs */
		  tny_gtk_header_view_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_header_view_iface_info = 
		{
		  (GInterfaceInitFunc) tny_header_view_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (GTK_TYPE_TABLE,
			"TnyGtkHeaderView",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_HEADER_VIEW_IFACE, 
			&tny_header_view_iface_info);

	}

	return type;
}
