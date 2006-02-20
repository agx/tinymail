/*
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

#include <tny-summary-window.h>
#include <tny-summary-window-iface.h>
#include <tny-account-store-iface.h>
#include <tny-account-store.h>

#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>

int 
main (int argc, char **argv)
{
	GtkWindow *window = NULL;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);

	gnome_vfs_init ();

	window = GTK_WINDOW (tny_summary_window_new ());

	gtk_widget_show (GTK_WIDGET (window));

	tny_summary_window_iface_set_account_store (TNY_SUMMARY_WINDOW_IFACE (window),
		TNY_ACCOUNT_STORE_IFACE (tny_account_store_get_instance ()));

	
	g_signal_connect (window, "destroy",
		G_CALLBACK (gtk_exit), 0);

	gtk_main();

	return 0;
}
