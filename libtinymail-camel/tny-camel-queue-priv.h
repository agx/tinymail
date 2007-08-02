#ifndef TNY_CAMEL_QUEUE_H
#define TNY_CAMEL_QUEUE_H

/* libtinymail-camel - The Tiny Mail base library for Camel
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

#include <glib.h>
#include <glib-object.h>

#include <tny-camel-store-account.h>

G_BEGIN_DECLS

#define TNY_TYPE_CAMEL_QUEUE             (tny_camel_queue_get_type ())
#define TNY_CAMEL_QUEUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_CAMEL_QUEUE, TnyCamelQueue))
#define TNY_CAMEL_QUEUE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_CAMEL_QUEUE, TnyCamelQueueClass))
#define TNY_IS_CAMEL_QUEUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_CAMEL_QUEUE))
#define TNY_IS_CAMEL_QUEUE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_CAMEL_QUEUE))
#define TNY_CAMEL_QUEUE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_CAMEL_QUEUE, TnyCamelQueueClass))

typedef struct _TnyCamelQueue TnyCamelQueue;
typedef struct _TnyCamelQueueClass TnyCamelQueueClass;

struct _TnyCamelQueue
{
	GObject parent;

	TnyCamelStoreAccount *account;
	GList *list;
	GThread *thread;
	GStaticRecMutex *lock;
	gboolean stopped;
	gpointer current;
};

struct _TnyCamelQueueClass 
{
	GObjectClass parent;
};


typedef enum {
	TNY_CAMEL_QUEUE_NORMAL_ITEM = 1<<0,
	TNY_CAMEL_QUEUE_RECONNECT_ITEM = 1<<1,
	TNY_CAMEL_QUEUE_CANCELLABLE_ITEM = 1<<2,
	TNY_CAMEL_QUEUE_PRIORITY_ITEM = 1<<3,
	TNY_CAMEL_QUEUE_GET_HEADERS_ITEM = 1<<3,
} TnyCamelQueueItemFlags;

GType tny_camel_queue_get_type (void);

TnyCamelQueue* _tny_camel_queue_new (TnyCamelStoreAccount *account);
void _tny_camel_queue_launch_wflags (TnyCamelQueue *queue, GThreadFunc func, gpointer data, TnyCamelQueueItemFlags flags, const gchar *name);
void _tny_camel_queue_launch (TnyCamelQueue *queue, GThreadFunc func, gpointer data, const gchar *name);
void _tny_camel_queue_remove_items (TnyCamelQueue *queue, TnyCamelQueueItemFlags flags);
void _tny_camel_queue_cancel_remove_items (TnyCamelQueue *queue, TnyCamelQueueItemFlags flags);

G_END_DECLS

#endif
