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

#include <config.h>

#include <tny-save-strategy.h>

/**
 * tny_save_strategy_save:
 * @self: A #TnySaveStrategy instance
 * @part: The #TnyMimePart instance that must be saved
 *
 * Performs the saving of a mime part
 *
 * Example:
 * <informalexample><programlisting>
 * static void 
 * tny_my_msg_view_on_save_clicked (TnyMsgView *self_i, TnyMimePart *attachment)
 * {
 *     TnyMyMsgView *self = TNY_MY_MSG_VIEW (self_i);
 *     tny_save_strategy_save (self->save_strategy, attachment);
 * }
 * </programlisting></informalexample>
 *
 * Implementors: the idea is that devices can have a specific such strategy.
 * For example a strategy that sends it to another computer or a strategy that
 * saves it to a flash disk. 
 *
 * Example:
 * <informalexample><programlisting>
 * static void
 * tny_gtk_save_strategy_save (TnySaveStrategy *self, TnyMimePart *part)
 * {
 *      GtkFileChooserDialog *dialog;
 *      dialog = GTK_FILE_CHOOSER_DIALOG 
 *            (gtk_file_chooser_dialog_new (_("Save File"), NULL,
 *            GTK_FILE_CHOOSER_ACTION_SAVE,
 *            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, 
 *            GTK_RESPONSE_ACCEPT, NULL));
 *      gtk_file_chooser_set_current_name (dialog, 
 *            tny_mime_part_get_filename (part));
 * 	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
 *            gchar *uri; int fd;
 *            uri = gtk_file_chooser_get_filename (dialog);
 *            fd = open (uri, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
 *            if (fd != -1) {
 *                      TnyStream *stream = tny_fs_stream_new (fd);
 *                      tny_mime_part_decode_to_stream (part, TNY_STREAM (stream));
 *                      g_object_unref (G_OBJECT (stream));
 *            }		
 *      }
 *      gtk_widget_destroy (GTK_WIDGET (dialog));
 * }
 * </programlisting></informalexample>
 *
 * The method is typically called by the implementation of a #TnyMsgView.
 * For example a clicked handler of a popup menu of a attachment view in the
 * #TnyMsgView implementation.
 * 
 **/
void
tny_save_strategy_save (TnySaveStrategy *self, TnyMimePart *part)
{
#ifdef DEBUG
	if (!TNY_SAVE_STRATEGY_GET_IFACE (self)->save_func)
		g_critical ("You must implement tny_save_strategy_save\n");
#endif

	TNY_SAVE_STRATEGY_GET_IFACE (self)->save_func (self, part);
	return;
}

static void
tny_save_strategy_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) 
		initialized = TRUE;
}

GType
tny_save_strategy_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnySaveStrategyIface),
		  tny_save_strategy_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnySaveStrategy", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}
