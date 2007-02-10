#ifndef TNY_HEADER_H
#define TNY_HEADER_H

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

#define TNY_TYPE_HEADER             (tny_header_get_type ())
#define TNY_HEADER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_HEADER, TnyHeader))
#define TNY_IS_HEADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_HEADER))
#define TNY_HEADER_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), TNY_TYPE_HEADER, TnyHeaderIface))

#define TNY_TYPE_HEADER_FLAGS (tny_header_flags_get_type())

#ifndef TNY_SHARED_H
typedef struct _TnyHeader TnyHeader;
typedef struct _TnyHeaderIface TnyHeaderIface;
typedef enum _TnyHeaderFlags TnyHeaderFlags;
#endif

enum _TnyHeaderFlags 
{
	TNY_HEADER_FLAG_ANSWERED = 1<<0,
	TNY_HEADER_FLAG_DELETED = 1<<1,
	TNY_HEADER_FLAG_DRAFT = 1<<2,
	TNY_HEADER_FLAG_FLAGGED = 1<<3,
	TNY_HEADER_FLAG_SEEN = 1<<4,
	TNY_HEADER_FLAG_ATTACHMENTS = 1<<5,
	TNY_HEADER_FLAG_CACHED = 1<<6,
	TNY_HEADER_FLAG_PARTIAL = 1<<7,
	TNY_HEADER_FLAG_EXPUNGED = 1<<7

	/* Keep below 1<<12 (internally used bits) */
};


struct _TnyHeaderIface
{
	GTypeInterface g;

	const gchar* (*get_uid_func) (TnyHeader *self);
	const gchar* (*get_bcc_func) (TnyHeader *self);
	const gchar* (*get_cc_func) (TnyHeader *self);
	const gchar* (*get_subject_func) (TnyHeader *self);
	const gchar* (*get_to_func) (TnyHeader *self);
	const gchar* (*get_from_func) (TnyHeader *self);
	const gchar* (*get_replyto_func) (TnyHeader *self);
	const gchar* (*get_message_id_func) (TnyHeader *self);
	guint (*get_message_size_func) (TnyHeader *self);
	time_t (*get_date_received_func) (TnyHeader *self);
	time_t (*get_date_sent_func) (TnyHeader *self);
	void (*set_bcc_func) (TnyHeader *self, const gchar *bcc);
	void (*set_cc_func) (TnyHeader *self, const gchar *cc);
	void (*set_from_func) (TnyHeader *self, const gchar *from);
	void (*set_subject_func) (TnyHeader *self, const gchar *subject);
	void (*set_to_func) (TnyHeader *self, const gchar *to);
	void (*set_replyto_func) (TnyHeader *self, const gchar *to);
        TnyFolder* (*get_folder_func) (TnyHeader *self);
	TnyHeaderFlags (*get_flags_func) (TnyHeader *self);
	void (*set_flags_func) (TnyHeader *self, TnyHeaderFlags mask);
	void (*unset_flags_func) (TnyHeader *self, TnyHeaderFlags mask);
};

GType tny_header_get_type (void);
GType tny_header_flags_get_type (void);

const gchar* tny_header_get_uid (TnyHeader *self);
const gchar* tny_header_get_bcc (TnyHeader *self);
const gchar* tny_header_get_cc (TnyHeader *self);
time_t tny_header_get_date_received (TnyHeader *self);
time_t tny_header_get_date_sent (TnyHeader *self);
const gchar* tny_header_get_message_id (TnyHeader *self);
guint tny_header_get_message_size (TnyHeader *self);
const gchar* tny_header_get_from (TnyHeader *self);
const gchar* tny_header_get_to (TnyHeader *self);
const gchar* tny_header_get_subject (TnyHeader *self);
const gchar* tny_header_get_replyto (TnyHeader *self);
void tny_header_set_bcc (TnyHeader *self, const gchar *bcc);
void tny_header_set_cc (TnyHeader *self, const gchar *cc);
void tny_header_set_from (TnyHeader *self, const gchar *from);
void tny_header_set_subject (TnyHeader *self, const gchar *subject);
void tny_header_set_to (TnyHeader *self, const gchar *to);
void tny_header_set_replyto (TnyHeader *self, const gchar *to);
TnyFolder* tny_header_get_folder (TnyHeader *self);
TnyHeaderFlags tny_header_get_flags (TnyHeader *self);
void tny_header_set_flags (TnyHeader *self, TnyHeaderFlags mask);
void tny_header_unset_flags (TnyHeader *self, TnyHeaderFlags mask);


G_END_DECLS

#endif
