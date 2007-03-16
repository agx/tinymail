#ifndef TNY_MAEMO_PASSWORD_DIALOG_H
#define TNY_MAEMO_PASSWORD_DIALOG_H

/* libtinymail-maemo - The Tiny Mail base library for Maemo
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
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_MAEMO_PASSWORD_DIALOG             (tny_maemo_password_dialog_get_type ())
#define TNY_MAEMO_PASSWORD_DIALOG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MAEMO_PASSWORD_DIALOG, TnyMaemoPasswordDialog))
#define TNY_MAEMO_PASSWORD_DIALOG_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MAEMO_PASSWORD_DIALOG, TnyMaemoPasswordDialogClass))
#define TNY_IS_MAEMO_PASSWORD_DIALOG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MAEMO_PASSWORD_DIALOG))
#define TNY_IS_MAEMO_PASSWORD_DIALOG_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MAEMO_PASSWORD_DIALOG))
#define TNY_MAEMO_PASSWORD_DIALOG_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_MAEMO_PASSWORD_DIALOG, TnyMaemoPasswordDialogClass))

typedef struct _TnyMaemoPasswordDialog TnyMaemoPasswordDialog;
typedef struct _TnyMaemoPasswordDialogClass TnyMaemoPasswordDialogClass;

struct _TnyMaemoPasswordDialog
{
	GtkDialog parent;
};

struct _TnyMaemoPasswordDialogClass
{
	GtkDialogClass parent_class;
};

GType tny_maemo_password_dialog_get_type (void);
GtkDialog* tny_maemo_password_dialog_new (void);

const gchar* tny_maemo_password_dialog_get_password (TnyMaemoPasswordDialog *self);
void tny_maemo_password_dialog_set_prompt (TnyMaemoPasswordDialog *self, const gchar *prompt);

G_END_DECLS

#endif
