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

#include <tny-summary-window.h>
#include <tny-summary-window-iface.h>
#include <tny-platform-factory-iface.h>
#include <tny-platform-factory.h>

#ifdef GNOME
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#endif


/**
 * main:
 * @argc: Amount of arguments
 * @argv: The arguments
 *
 * Main entry point of the application
 *
 * Return value: application return value (0 on success)
 **/
int 
main (int argc, char **argv)
{
	GtkWindow *window = NULL;
	TnyPlatformFactoryIface *platfact;

	gtk_init (&argc, &argv);
	g_thread_init (NULL);
	gdk_threads_init ();

#ifdef GNOME
	gnome_vfs_init ();
#endif

	platfact = TNY_PLATFORM_FACTORY_IFACE 
			(tny_platform_factory_get_instance ());

	window = GTK_WINDOW (tny_summary_window_new ());

	gtk_widget_show (GTK_WIDGET (window));

	tny_summary_window_iface_set_account_store (
		TNY_SUMMARY_WINDOW_IFACE (window),
		tny_platform_factory_iface_new_account_store (platfact));
	
	g_signal_connect (window, "destroy",
		G_CALLBACK (gtk_exit), 0);

	gtk_main();

	return 0;
}
