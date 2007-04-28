/* tinymail - Tiny Mail
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gtk.org>
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

/* TODO: Refactory this type to libtinymailui-gtk */
#include <config.h>
#include <string.h>

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>
#include <tny-gtk-password-dialog.h>

#include <tny-account.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGtkPasswordDialogPriv TnyGtkPasswordDialogPriv;

struct _TnyGtkPasswordDialogPriv
{
	GtkEntry *pwd_entry;
	GtkLabel *prompt_label;
};

#define TNY_GTK_PASSWORD_DIALOG_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_PASSWORD_DIALOG, TnyGtkPasswordDialogPriv))

static GHashTable *passwords = NULL;



static const gchar*
tny_gtk_password_dialog_get_password (TnyPasswordGetter *self, const gchar *aid, const gchar *prompt, gboolean *cancel)
{
	TnyGtkPasswordDialogPriv *priv = TNY_GTK_PASSWORD_DIALOG_GET_PRIVATE (self);
	GtkDialog *dialog = (GtkDialog *) self;
	const gchar *accountid = aid;
	const gchar *retval = NULL;

	if (G_UNLIKELY (!passwords))
		passwords = g_hash_table_new (g_str_hash, g_str_equal);
	retval = g_hash_table_lookup (passwords, accountid);

	if (G_UNLIKELY (!retval))
	{
		gtk_label_set_text (priv->prompt_label, prompt);

		if (G_LIKELY (gtk_dialog_run (dialog) == GTK_RESPONSE_OK))
		{
			const gchar *pwd = gtk_entry_get_text (priv->pwd_entry);
			retval = g_strdup (pwd);
			mlock (pwd, strlen (pwd));
			mlock (retval, strlen (retval));
			*cancel = FALSE;
		} else {
			*cancel = TRUE;
		}
		gtk_widget_destroy (GTK_WIDGET (dialog));
		while (gtk_events_pending ())
			gtk_main_iteration ();
	} else {
			*cancel = FALSE;
	}

	return retval;
}

static void 
tny_gtk_password_dialog_forget_password (TnyPasswordGetter *self, const gchar *aid)
{
	if (G_LIKELY (passwords))
	{
		const gchar *accountid = aid;

		gchar *pwd = g_hash_table_lookup (passwords, accountid);

		if (G_LIKELY (pwd))
		{
			memset (pwd, 0, strlen (pwd));
			/* g_free (pwd); uhm, crashed once */
			g_hash_table_remove (passwords, accountid);
		}

	}
}

/**
 * tny_gtk_password_dialog_new:
 * 
 * Create a dialog window that will ask the user for a password
 *
 * Return value: A new #GtkDialog password dialog instance implemented for Gtk+
 **/
GtkDialog*
tny_gtk_password_dialog_new (void)
{
	TnyGtkPasswordDialog *self = g_object_new (TNY_TYPE_GTK_PASSWORD_DIALOG, NULL);

	return GTK_DIALOG (self);
}

static void
tny_gtk_password_dialog_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkPasswordDialog *self = (TnyGtkPasswordDialog *)instance;
	TnyGtkPasswordDialogPriv *priv = TNY_GTK_PASSWORD_DIALOG_GET_PRIVATE (self);

	gtk_dialog_add_buttons (GTK_DIALOG (self), GTK_STOCK_OK, GTK_RESPONSE_OK,
				GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

	gtk_window_set_title (GTK_WINDOW (self), _("Password input"));

	/* TODO: Add key icon or something */

	priv->pwd_entry = GTK_ENTRY (gtk_entry_new ());
	priv->prompt_label = GTK_LABEL (gtk_label_new (""));

	gtk_entry_set_visibility (priv->pwd_entry, FALSE);

	gtk_widget_show (GTK_WIDGET (priv->pwd_entry));
	gtk_widget_show (GTK_WIDGET (priv->prompt_label));

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (self)->vbox), 
		GTK_WIDGET (priv->prompt_label), TRUE, TRUE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (self)->vbox), 
		GTK_WIDGET (priv->pwd_entry), TRUE, TRUE, 0);

	return;
}

static void
tny_gtk_password_dialog_finalize (GObject *object)
{
	(*parent_class->finalize) (object);

	return;
}



static void
tny_password_getter_init (gpointer g, gpointer iface_data)
{
	TnyPasswordGetterIface *klass = (TnyPasswordGetterIface *)g;

	klass->forget_password_func = tny_gtk_password_dialog_forget_password;
	klass->get_password_func = tny_gtk_password_dialog_get_password;
}


static void 
tny_gtk_password_dialog_class_init (TnyGtkPasswordDialogClass *class)
{
	GObjectClass *object_class;

	/* parent_class = g_type_class_peek_parent (class); */

	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_password_dialog_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkPasswordDialogPriv));

	return;
}

GType 
tny_gtk_password_dialog_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkPasswordDialogClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_password_dialog_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkPasswordDialog),
		  0,      /* n_preallocs */
		  tny_gtk_password_dialog_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
			"TnyGtkPasswordDialog",
			&info, 0);

		static const GInterfaceInfo tny_password_getter_info = 
		{
		  (GInterfaceInitFunc) tny_password_getter_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		g_type_add_interface_static (type, TNY_TYPE_PASSWORD_GETTER, 
			&tny_password_getter_info);

		parent_class = g_type_class_ref (gtk_dialog_get_type());
	}

	return type;
}
