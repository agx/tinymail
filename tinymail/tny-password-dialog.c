/* libtinymail - The Tiny Mail base library
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

#include <gtk/gtk.h>
#include <tny-password-dialog.h>

static GObjectClass *parent_class = NULL;

const gchar*
tny_password_dialog_get_password (TnyPasswordDialog *self)
{
	return gtk_entry_get_text (self->pwd_entry);
}


TnyPasswordDialog*
tny_password_dialog_new (void)
{
	TnyPasswordDialog *self = g_object_new (TNY_PASSWORD_DIALOG_TYPE, NULL);

	return self;
}

static void
tny_password_dialog_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyPasswordDialog *self = (TnyPasswordDialog *)instance;

	gtk_dialog_add_buttons (GTK_DIALOG (self), GTK_STOCK_OK, GTK_RESPONSE_OK,
				GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);

	gtk_window_set_title (GTK_WINDOW (self), "Password input");

	self->pwd_entry = GTK_ENTRY (gtk_entry_new ());
	gtk_entry_set_visibility (self->pwd_entry, FALSE);
	gtk_widget_show (GTK_WIDGET (self->pwd_entry));

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (self)->vbox), 
		GTK_WIDGET (self->pwd_entry), TRUE, TRUE, 0);

	return;
}

static void
tny_password_dialog_finalize (GObject *object)
{
	TnyPasswordDialog *self = (TnyPasswordDialog *)object;	
	
	(*parent_class->finalize) (object);

	return;
}


static void 
tny_password_dialog_class_init (TnyPasswordDialogClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_password_dialog_finalize;

	return;
}

GType 
tny_password_dialog_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyPasswordDialogClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_password_dialog_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyPasswordDialog),
		  0,      /* n_preallocs */
		  tny_password_dialog_instance_init    /* instance_init */
		};

		type = g_type_register_static (GTK_TYPE_DIALOG,
			"TnyPasswordDialog",
			&info, 0);

	}

	return type;
}
