#ifndef TNY_MSG_HEADER_IFACE_H
#define TNY_MSG_HEADER_IFACE_H

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

/* 
 * Whether or not this type should be implemented using GTypeInterface,
 * is indeed questionable. Mainly because a lot instances of this type
 * will be created during the lifetime of the application. GObject 
 * is known to be rather slow.
 *
 * It could easily be done as a more simple C-Interface. But let's first
 * measure the performance, before starting to make decisions not to utilise
 * GObject standard techniques.
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_MSG_HEADER_IFACE_TYPE             (tny_msg_header_iface_get_type ())
#define TNY_MSG_HEADER_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_HEADER_IFACE_TYPE, TnyMsgHeaderIface))
#define TNY_MSG_HEADER_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_HEADER_IFACE_TYPE, TnyMsgHeaderIfaceClass))
#define TNY_IS_MSG_HEADER_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_HEADER_IFACE_TYPE))
#define TNY_IS_MSG_HEADER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_HEADER_IFACE_TYPE))
#define TNY_MSG_HEADER_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_MSG_HEADER_IFACE_TYPE, TnyMsgHeaderIfaceClass))


struct _TnyMsgHeaderIfaceClass
{
	GTypeInterface g_iface;

	const gchar*   (*get_subject_func)        (TnyMsgHeaderIface *self);
	const gchar*   (*get_to_func)             (TnyMsgHeaderIface *self);
	const gchar*   (*get_from_func)           (TnyMsgHeaderIface *self);
	const gchar*   (*get_id_func)             (TnyMsgHeaderIface *self);

         
        const TnyMsgFolderIface*
                       (*get_folder_func)         (TnyMsgHeaderIface *self);
        void           (*set_folder_func)         (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder);

	void           (*set_subject_func)        (TnyMsgHeaderIface *self, const gchar *subject);
	void           (*set_to_func)             (TnyMsgHeaderIface *self, const gchar *to);
	void           (*set_from_func)           (TnyMsgHeaderIface *self, const gchar *from);
	void           (*set_id_func)             (TnyMsgHeaderIface *self, const gchar *id);

	void           (*uncache_func)            (TnyMsgHeaderIface *self);
	const gboolean (*has_cache_func)          (TnyMsgHeaderIface *self);
};

GType          tny_msg_header_iface_get_type      (void);

const gchar*   tny_msg_header_iface_get_id        (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_from      (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_to        (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_subject   (TnyMsgHeaderIface *self);

void           tny_msg_header_iface_set_id        (TnyMsgHeaderIface *self, const gchar *id);
void           tny_msg_header_iface_set_from      (TnyMsgHeaderIface *self, const gchar *from);
void           tny_msg_header_iface_set_to        (TnyMsgHeaderIface *self, const gchar *to);
void           tny_msg_header_iface_set_subject   (TnyMsgHeaderIface *self, const gchar *subject);


const TnyMsgFolderIface*
               tny_msg_header_iface_get_folder    (TnyMsgHeaderIface *self);
void           tny_msg_header_iface_set_folder    (TnyMsgHeaderIface *self, const TnyMsgFolderIface *folder);

void           tny_msg_header_iface_uncache       (TnyMsgHeaderIface *self);
const gboolean tny_msg_header_iface_has_cache     (TnyMsgHeaderIface *self);

G_END_DECLS

#endif
