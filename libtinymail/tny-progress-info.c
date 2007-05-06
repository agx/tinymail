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

#define TINYMAIL_ENABLE_PRIVATE_API
#include "tny-common-priv.h"
#undef TINYMAIL_ENABLE_PRIVATE_API

struct _TnyProgressInfo 
{
	GObject *self;
	TnyStatusCallback status_callback;
	TnyStatusDomain domain;
	TnyStatusCode code;
	gchar *what;
	gint sofar;
	gint oftotal;
	TnyIdleStopper* stopper;
	gpointer user_data;
};

TnyProgressInfo*
tny_progress_info_new (GObject *self, TnyStatusCallback status_callback, TnyStatusDomain domain, TnyStatusCode code, const gchar *what, gint sofar, gint oftotal, TnyIdleStopper* stopper, gpointer user_data)
{
	TnyProgressInfo *info = g_slice_new (TnyProgressInfo);

	info->domain = domain;
	info->code = code;
	info->status_callback = status_callback;
	info->what = g_strdup (what);
	info->self = g_object_ref (self);
	info->user_data = user_data;
	info->oftotal = oftotal;

	/* Share the TnyIdleStopper, so one callback can tell the other to stop,
	 * because they may be called in an unexpected sequence: 
	 * This is destroyed in the idle GDestroyNotify callback. */

	info->stopper = tny_idle_stopper_copy (stopper);

	if (info->oftotal < 1)
		info->oftotal = 1;

	if (sofar < 1)
		info->sofar = 1;
	else 
		if (sofar > info->oftotal)
			info->sofar = info->oftotal;
		else
			info->sofar = sofar;

	return info;
}



void 
tny_progress_info_destroy (gpointer data)
{
	TnyProgressInfo *info = data;

	g_object_unref (G_OBJECT (info->self));
	g_free (info->what);
	tny_idle_stopper_destroy (info->stopper);
	info->stopper = NULL;

	g_slice_free (TnyProgressInfo, data);

	return;
}

gboolean 
tny_progress_info_idle_func (gpointer data)
{
	TnyProgressInfo *info = data;

	/* Do not call the status callback after the main callback 
	 * has already been called, because we should assume that 
	 * the user_data is invalid after that time: */
	if (tny_idle_stopper_is_stopped (info->stopper))
		return FALSE;

	if (info && info->status_callback)
	{
		TnyStatus *status = tny_status_new (info->domain, 
			info->code, info->sofar, info->oftotal, 
			info->what);

		info->status_callback (G_OBJECT (info->self), status, 
			info->user_data);

		tny_status_free (status);
	}

	return FALSE;
} 
