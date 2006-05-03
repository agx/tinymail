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

#include <string.h>

#include <tny-stream-iface.h>
#include <tny-stream-camel.h>
#include <tny-msg-folder-iface.h>
#include <tny-msg-folder.h>

#include <camel/camel.h>
#include <camel/camel-session.h>
#include <camel/camel-store.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include <tny-session-camel.h>

#include <tny-camel-shared.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyStreamCamelPriv TnyStreamCamelPriv;

struct _TnyStreamCamelPriv
{
	CamelStream *stream;
};

#define TNY_STREAM_CAMEL_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_STREAM_CAMEL, TnyStreamCamelPriv))

static ssize_t
tny_stream_camel_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);
	CamelStream *stream = priv->stream;
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;

	g_return_val_if_fail (CAMEL_IS_STREAM (stream), -1);
	g_return_val_if_fail (TNY_IS_STREAM_IFACE (output), -1);

	while (G_LIKELY (!camel_stream_eos (stream))) 
	{
		nb_read = camel_stream_read (stream, tmp_buf, sizeof (tmp_buf));
		if (G_UNLIKELY (nb_read < 0))
			return -1;
		else if (G_LIKELY (nb_read > 0)) 
		{
			nb_written = 0;
	
			while (G_UNLIKELY (nb_written < nb_read)) 
			{
				ssize_t len = tny_stream_iface_write (output, tmp_buf + nb_written,
								  nb_read - nb_written);
				if (G_UNLIKELY (len < 0))
					return -1;
				nb_written += len;
			}
			total += nb_written;
		}
	}
	return total;
}

static ssize_t
tny_stream_camel_read  (TnyStreamIface *self, char *buffer, size_t n)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_read (priv->stream, buffer, n);
}

static ssize_t
tny_stream_camel_write (TnyStreamIface *self, const char *buffer, size_t n)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_write (priv->stream, buffer, n);
}

static gint
tny_stream_camel_flush (TnyStreamIface *self)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_flush (priv->stream);
}

static gint
tny_stream_camel_close (TnyStreamIface *self)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_close (priv->stream);
}

static gboolean
tny_stream_camel_eos   (TnyStreamIface *self)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_eos (priv->stream);
}

static gint
tny_stream_camel_reset (TnyStreamIface *self)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	return camel_stream_reset (priv->stream);
}

/**
 * tny_stream_camel_set_stream:
 * @self: A #TnyStreamCamel object
 * @stream: A #CamelStream object
 *
 * Set the stream to play proxy for
 *
 **/
void
tny_stream_camel_set_stream (TnyStreamCamel *self, CamelStream *stream)
{
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	camel_object_unref (CAMEL_OBJECT (priv->stream));
	camel_object_ref (CAMEL_OBJECT (stream));

	priv->stream = stream;

	return;
}

/**
 * tny_stream_camel_new:
 * @stream: A #CamelStream stream to play proxy for
 *
 * Return value: A new #TnyStreamCamel instance implemented as a proxy
 * for a #CamelStream
 **/
TnyStreamCamel*
tny_stream_camel_new (CamelStream *stream)
{
	TnyStreamCamel *self = g_object_new (TNY_TYPE_STREAM_CAMEL, NULL);
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	camel_object_ref (CAMEL_OBJECT (stream));
	priv->stream = stream;

	return self;
}

static void
tny_stream_camel_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyStreamCamel *self = (TnyStreamCamel *)instance;
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	priv->stream = NULL;

	return;
}

static void
tny_stream_camel_finalize (GObject *object)
{
	TnyStreamCamel *self = (TnyStreamCamel *)object;	
	TnyStreamCamelPriv *priv = TNY_STREAM_CAMEL_GET_PRIVATE (self);

	if (G_LIKELY (priv->stream))
		camel_object_unref (CAMEL_OBJECT (priv->stream));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStreamIfaceClass *klass = (TnyStreamIfaceClass *)g_iface;

	klass->read_func = tny_stream_camel_read;
	klass->write_func = tny_stream_camel_write;
	klass->flush_func = tny_stream_camel_flush;
	klass->close_func = tny_stream_camel_close;
	klass->eos_func = tny_stream_camel_eos;
	klass->reset_func = tny_stream_camel_reset;
	klass->write_to_stream_func = tny_stream_camel_write_to_stream;

	return;
}

static void 
tny_stream_camel_class_init (TnyStreamCamelClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_stream_camel_finalize;

	g_type_class_add_private (object_class, sizeof (TnyStreamCamelPriv));

	return;
}

GType 
tny_stream_camel_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY (!camel_type_init_done))
	{
		camel_type_init ();
		camel_type_init_done = TRUE;
	}

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyStreamCamelClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_stream_camel_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyStreamCamel),
		  0,      /* n_preallocs */
		  tny_stream_camel_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_iface_info = 
		{
		  (GInterfaceInitFunc) tny_stream_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyStreamCamel",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM_IFACE, 
			&tny_stream_iface_info);
	}

	return type;
}

