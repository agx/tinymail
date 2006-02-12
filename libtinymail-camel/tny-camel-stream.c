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
#include <glib.h>

#include <string.h>

#include <tny-stream-iface.h>
#include <tny-camel-stream.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-camel-session.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyCamelStreamPriv TnyCamelStreamPriv;

struct _TnyCamelStreamPriv
{
	CamelStream *stream;
};

#define TNY_CAMEL_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_CAMEL_STREAM_TYPE, TnyCamelStreamPriv))


static ssize_t
tny_camel_stream_read  (TnyStreamIface *self, char *buffer, size_t n)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_read (priv->stream, buffer, n);
}

static ssize_t
tny_camel_stream_write (TnyStreamIface *self, const char *buffer, size_t n)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_write (priv->stream, buffer, n);
}

static gint
tny_camel_stream_flush (TnyStreamIface *self)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_flush (priv->stream);
}

static gint
tny_camel_stream_close (TnyStreamIface *self)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_close (priv->stream);
}

static gboolean
tny_camel_stream_eos   (TnyStreamIface *self)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_eos (priv->stream);
}

static gint
tny_camel_stream_reset (TnyStreamIface *self)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	return camel_stream_reset (priv->stream);
}

void
tny_camel_stream_set_stream (TnyCamelStream *self, CamelStream *stream)
{
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	camel_object_unref (CAMEL_OBJECT (priv->stream));
	camel_object_ref (CAMEL_OBJECT (stream));

	priv->stream = stream;

	return;
}


TnyCamelStream*
tny_camel_stream_new (CamelStream *stream)
{
	TnyCamelStream *self = g_object_new (TNY_CAMEL_STREAM_TYPE, NULL);
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	camel_object_ref (CAMEL_OBJECT (stream));
	priv->stream = stream;

	return self;
}

static void
tny_camel_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyCamelStream *self = (TnyCamelStream *)instance;
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	priv->stream = NULL;

	return;
}

static void
tny_camel_stream_finalize (GObject *object)
{
	TnyCamelStream *self = (TnyCamelStream *)object;	
	TnyCamelStreamPriv *priv = TNY_CAMEL_STREAM_GET_PRIVATE (self);

	camel_object_unref (CAMEL_OBJECT (priv->stream));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStreamIfaceClass *klass = (TnyStreamIfaceClass *)g_iface;

	klass->read_func = tny_camel_stream_read;
	klass->write_func = tny_camel_stream_write;
	klass->flush_func = tny_camel_stream_flush;
	klass->close_func = tny_camel_stream_close;
	klass->eos_func = tny_camel_stream_eos;
	klass->reset_func = tny_camel_stream_reset;

	return;
}

static void 
tny_camel_stream_class_init (TnyCamelStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_camel_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyCamelStreamPriv));

	return;
}

GType 
tny_camel_stream_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyCamelStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_camel_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyCamelStream),
		  0,      /* n_preallocs */
		  tny_camel_stream_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_iface_info = 
		{
		  (GInterfaceInitFunc) tny_stream_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyCamelStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_STREAM_IFACE_TYPE, 
			&tny_stream_iface_info);
	}

	return type;
}

