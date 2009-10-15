/* tinymail - Tiny Mail
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
#include <glib.h>
#include <glib/gi18n-lib.h>
#include <string.h>

#include <tny-gnome-keyring-password-getter.h>

static GObjectClass *parent_class = NULL;

static const gchar*
tny_gnome_keyring_password_getter_get_password (TnyPasswordGetter *self, const gchar *aid, const gchar *prompt, gboolean *cancel)
{
	gchar *retval = NULL;
	GList *list;
	GnomeKeyringResult keyringret;
	gchar *keyring;

	gnome_keyring_get_default_keyring_sync (&keyring);

	keyringret = gnome_keyring_find_network_password_sync (aid, "Mail", 
		aid /* hostname */,
		"password", aid /* proto */, 
		"PLAIN", 0, &list);

	if ((keyringret != GNOME_KEYRING_RESULT_OK) || (list == NULL))
	{
		gint response;

		GtkWidget *dialog;
		GtkWidget *in_vbox;
		GtkWidget *in_label;
		GtkWidget *in_hbox;
		GtkWidget *in_icon;
		GtkWidget *password_hbox;
		GtkWidget *password_label;
		GtkWidget *password_entry;
		GtkWidget *remember_check_button;
		GtkSizeGroup *sizegroup;

		dialog = gtk_dialog_new_with_buttons (_("Enter password"),
						      NULL,
						      GTK_DIALOG_MODAL,
						      GTK_STOCK_CANCEL,
						      GTK_RESPONSE_REJECT,
						      GTK_STOCK_OK,
						      GTK_RESPONSE_ACCEPT,
						      NULL);
		gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
		gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (dialog)->vbox), 12);
		gtk_container_set_border_width (GTK_CONTAINER (dialog), 12);

		in_hbox = gtk_hbox_new (FALSE, 12);
		password_hbox = gtk_hbox_new (FALSE, 12);
		sizegroup = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

		in_icon = gtk_image_new_from_stock (GTK_STOCK_DIALOG_AUTHENTICATION,
						    GTK_ICON_SIZE_DIALOG);
		gtk_misc_set_alignment (GTK_MISC (in_icon), 1.0, 0.5);

		in_label = gtk_label_new (NULL);
		gtk_label_set_line_wrap (GTK_LABEL (in_label), TRUE);
		gtk_label_set_line_wrap_mode (GTK_LABEL (in_label), PANGO_WRAP_WORD_CHAR);
		gtk_label_set_markup (GTK_LABEL (in_label), prompt);
		gtk_misc_set_alignment (GTK_MISC (in_label), 0.0, 1.0);

		password_entry = gtk_entry_new ();
		gtk_entry_set_visibility (GTK_ENTRY (password_entry), FALSE);

		password_label = gtk_label_new (_("Password:"));
		gtk_misc_set_alignment (GTK_MISC (password_label), 0.0, 0.5);

		remember_check_button = gtk_check_button_new_with_label (_("Remember password"));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (remember_check_button), FALSE);

		gtk_box_pack_start (GTK_BOX (password_hbox), password_label, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (password_hbox), password_entry, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (in_hbox), in_icon, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (in_hbox), in_label, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), in_hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), password_hbox, FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), remember_check_button, FALSE, FALSE, 0);
		
		gtk_size_group_add_widget (sizegroup, password_label);
		g_object_unref (sizegroup);

		gtk_widget_show (in_hbox);
		gtk_widget_show (in_icon);
		gtk_widget_show (password_hbox);
		gtk_widget_show (in_label);
		gtk_widget_show (password_entry);
		gtk_widget_show (password_label);
		gtk_widget_show (remember_check_button);
		gtk_widget_show (GTK_DIALOG(dialog)->vbox);

		response = gtk_dialog_run (GTK_DIALOG (dialog));
		
		if (response == GTK_RESPONSE_ACCEPT)
		{
			guint32 item_id;
			gboolean remember;

			retval = g_strdup (gtk_entry_get_text (GTK_ENTRY (password_entry)));

			mlock (retval, strlen (retval));

			remember = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (remember_check_button));

			if (remember)
			{
				gnome_keyring_set_network_password_sync (keyring,
					aid /* user */,
					"Mail", aid /* hostname */,
					"password", aid /* proto */, 
					"PLAIN", 0, retval, &item_id);
			}
		} else retval = NULL;

		*cancel = (response != GTK_RESPONSE_ACCEPT);

		/* this causes warnings, but should be done afaik */
		gtk_widget_destroy (dialog);

		while (gtk_events_pending ())
			gtk_main_iteration ();

	} else {

		GnomeKeyringNetworkPasswordData *pwd_data;
		pwd_data = list->data;
		retval = g_strdup (pwd_data->password);

		*cancel = FALSE;

		gnome_keyring_network_password_list_free (list);
	}

	return retval;
}

static void
tny_gnome_keyring_password_getter_forget_password (TnyPasswordGetter *self, const gchar *aid)
{
	GList *list=NULL;
	GnomeKeyringResult keyringret;
	gchar *keyring;
	GnomeKeyringNetworkPasswordData *pwd_data;

	gnome_keyring_get_default_keyring_sync (&keyring);

	keyringret = gnome_keyring_find_network_password_sync (
		aid /* user */,
		"Mail", aid /* hostname */,
		"password", aid /* proto */, 
		"PLAIN", 0, &list);

	if (keyringret == GNOME_KEYRING_RESULT_OK && list)
	{
		pwd_data = list->data;
		gnome_keyring_item_delete_sync (keyring, pwd_data->item_id);
		gnome_keyring_network_password_list_free (list);
	}
	return;
}

static void
tny_gnome_keyring_password_getter_finalize (GObject *object)
{
	parent_class->finalize (object);
}

static void
tny_password_getter_init (TnyPasswordGetterIface *klass)
{
	klass->get_password= tny_gnome_keyring_password_getter_get_password;
	klass->forget_password= tny_gnome_keyring_password_getter_forget_password;
}

static void
tny_gnome_keyring_password_getter_class_init (TnyGnomeKeyringPasswordGetterClass *klass)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (klass);
	object_class = (GObjectClass*) klass;
	object_class->finalize = tny_gnome_keyring_password_getter_finalize;
}

static void 
tny_gnome_keyring_password_getter_instance_init (GTypeInstance *instance, gpointer g_class)
{
}


TnyPasswordGetter* tny_gnome_keyring_password_getter_new (void)
{
	TnyGnomeKeyringPasswordGetter *self = g_object_new (TNY_TYPE_GNOME_KEYRING_PASSWORD_GETTER, NULL);

	return TNY_PASSWORD_GETTER (self);
}

static gpointer
tny_gnome_keyring_password_getter_register_type (gpointer notused)
{
	GType type = 0;
	static const GTypeInfo info = 
		{
			sizeof (TnyGnomeKeyringPasswordGetterClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_gnome_keyring_password_getter_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGnomeKeyringPasswordGetter),
			0,      /* n_preallocs */
			tny_gnome_keyring_password_getter_instance_init,    /* instance_init */
			NULL
		};


	static const GInterfaceInfo tny_password_getter_info = 
		{
			(GInterfaceInitFunc) tny_password_getter_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

	type = g_type_register_static (G_TYPE_OBJECT,
				       "TnyGnomeKeyringPasswordGetter",
				       &info, 0);

	g_type_add_interface_static (type, TNY_TYPE_PASSWORD_GETTER,
				     &tny_password_getter_info);

	return GUINT_TO_POINTER (type);
}

GType
tny_gnome_keyring_password_getter_get_type (void)
{
	static GOnce once = G_ONCE_INIT;
	g_once (&once, tny_gnome_keyring_password_getter_register_type, NULL);
	return GPOINTER_TO_UINT (once.retval);
}
