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

#include <time.h>
#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_TYPE_MSG_HEADER_IFACE             (tny_msg_header_iface_get_type ())
#define TNY_MSG_HEADER_IFACE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_MSG_HEADER_IFACE, TnyMsgHeaderIface))
#define TNY_MSG_HEADER_IFACE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_MSG_HEADER_IFACE, TnyMsgHeaderIfaceClass))
#define TNY_IS_MSG_HEADER_IFACE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_MSG_HEADER_IFACE))
#define TNY_IS_MSG_HEADER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_MSG_HEADER_IFACE))
#define TNY_MSG_HEADER_IFACE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_MSG_HEADER_IFACE, TnyMsgHeaderIfaceClass))

#define TNY_TYPE_MSG_HEADER_FLAGS (tny_msg_header_flags_get_type())

enum _TnyMsgHeaderFlags 
{
	TNY_MSG_HEADER_FLAG_ANSWERED = 1<<0,
	TNY_MSG_HEADER_FLAG_DELETED = 1<<1,
	TNY_MSG_HEADER_FLAG_DRAFT = 1<<2,
	TNY_MSG_HEADER_FLAG_FLAGGED = 1<<3,
	TNY_MSG_HEADER_FLAG_SEEN = 1<<4,
	TNY_MSG_HEADER_FLAG_ATTACHMENTS = 1<<5,
	TNY_MSG_HEADER_FLAG_ANSWERED_ALL = 1<<6,
	TNY_MSG_HEADER_FLAG_JUNK = 1<<7,
	TNY_MSG_HEADER_FLAG_SECURE = 1<<8,
	TNY_MSG_HEADER_FLAG_FOLDER_FLAGGED = 1<<16,
	TNY_MSG_HEADER_FLAG_JUNK_LEARN = 1<<30,
	TNY_MSG_HEADER_FLAG_USER = 1<<31
};


struct _TnyMsgHeaderIfaceClass
{
	GTypeInterface g_iface;

	const gchar*   (*get_uid_func)            (TnyMsgHeaderIface *self);
	const gchar*   (*get_bcc_func)            (TnyMsgHeaderIface *self);
	const gchar*   (*get_cc_func)             (TnyMsgHeaderIface *self);
	const gchar*   (*get_subject_func)        (TnyMsgHeaderIface *self);
	const gchar*   (*get_to_func)             (TnyMsgHeaderIface *self);
	const gchar*   (*get_from_func)           (TnyMsgHeaderIface *self);
	const gchar*   (*get_replyto_func)        (TnyMsgHeaderIface *self);
	const gchar*   (*get_message_id_func)     (TnyMsgHeaderIface *self);

	time_t         (*get_date_received_func)  (TnyMsgHeaderIface *self);
	time_t         (*get_date_sent_func)      (TnyMsgHeaderIface *self);

	void           (*set_bcc_func)            (TnyMsgHeaderIface *self, const gchar *bcc);
	void           (*set_cc_func)             (TnyMsgHeaderIface *self, const gchar *cc);
	void           (*set_from_func)           (TnyMsgHeaderIface *self, const gchar *from);
	void           (*set_subject_func)        (TnyMsgHeaderIface *self, const gchar *subject);
	void           (*set_to_func)             (TnyMsgHeaderIface *self, const gchar *to);
	void           (*set_replyto_func)        (TnyMsgHeaderIface *self, const gchar *to);

        TnyFolderIface*
                       (*get_folder_func)         (TnyMsgHeaderIface *self);
        void           (*set_folder_func)         (TnyMsgHeaderIface *self, TnyFolderIface *folder);

	TnyMsgHeaderFlags 
		       (*get_flags_func)          (TnyMsgHeaderIface *self);
	void           (*set_flags_func)          (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask);
	void           (*unset_flags_func)        (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask);
};

GType          tny_msg_header_iface_get_type      (void);
GType          tny_msg_header_flags_get_type      (void);

const gchar*   tny_msg_header_iface_get_uid             (TnyMsgHeaderIface *self);

const gchar*   tny_msg_header_iface_get_bcc            (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_cc             (TnyMsgHeaderIface *self);
time_t         tny_msg_header_iface_get_date_received  (TnyMsgHeaderIface *self);
time_t         tny_msg_header_iface_get_date_sent      (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_message_id     (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_from           (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_to             (TnyMsgHeaderIface *self);
const gchar*   tny_msg_header_iface_get_subject        (TnyMsgHeaderIface *self);

const gchar*   tny_msg_header_iface_get_replyto        (TnyMsgHeaderIface *self);

void           tny_msg_header_iface_set_bcc            (TnyMsgHeaderIface *self, const gchar *bcc);
void           tny_msg_header_iface_set_cc             (TnyMsgHeaderIface *self, const gchar *cc);
void           tny_msg_header_iface_set_from           (TnyMsgHeaderIface *self, const gchar *from);
void           tny_msg_header_iface_set_subject        (TnyMsgHeaderIface *self, const gchar *subject);
void           tny_msg_header_iface_set_to             (TnyMsgHeaderIface *self, const gchar *to);
void           tny_msg_header_iface_set_replyto        (TnyMsgHeaderIface *self, const gchar *to);

TnyFolderIface*
               tny_msg_header_iface_get_folder         (TnyMsgHeaderIface *self);

TnyMsgHeaderFlags  
	       tny_msg_header_iface_get_flags          (TnyMsgHeaderIface *self);
void           tny_msg_header_iface_set_folder         (TnyMsgHeaderIface *self, TnyFolderIface *folder);
void           tny_msg_header_iface_set_flags          (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask);
void           tny_msg_header_iface_unset_flags        (TnyMsgHeaderIface *self, TnyMsgHeaderFlags mask);


G_END_DECLS

#endif
