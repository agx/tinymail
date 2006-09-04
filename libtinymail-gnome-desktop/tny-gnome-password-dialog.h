#ifndef TNY_GNOME_PASSWORD_DIALOG_H
#define TNY_GNOME_PASSWORD_DIALOG_H

/* tinymail - Tiny Mail
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <gtk/gtk.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_GNOME_PASSWORD_DIALOG             (tny_gnome_password_dialog_get_type ())
#define TNY_GNOME_PASSWORD_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_GNOME_PASSWORD_DIALOG, TnyGnomePasswordDialog))
#define TNY_GNOME_PASSWORD_DIALOG_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_GNOME_PASSWORD_DIALOG, TnyGnomePasswordDialogClass))
#define TNY_IS_GNOME_PASSWORD_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_GNOME_PASSWORD_DIALOG))
#define TNY_IS_GNOME_PASSWORD_DIALOG_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_GNOME_PASSWORD_DIALOG))
#define TNY_GNOME_PASSWORD_DIALOG_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_GNOME_PASSWORD_DIALOG, TnyGnomePasswordDialogClass))

typedef struct _TnyGnomePasswordDialog TnyGnomePasswordDialog;
typedef struct _TnyGnomePasswordDialogClass TnyGnomePasswordDialogClass;

struct _TnyGnomePasswordDialog
{
	GtkDialog parent;
};

struct _TnyGnomePasswordDialogClass
{
	GtkDialogClass parent_class;
};

GType tny_gnome_password_dialog_get_type (void);
GtkDialog* tny_gnome_password_dialog_new (void);

const gchar* tny_gnome_password_dialog_get_password (TnyGnomePasswordDialog *self);
void tny_gnome_password_dialog_set_prompt (TnyGnomePasswordDialog *self, const gchar *prompt);

G_END_DECLS

#endif
