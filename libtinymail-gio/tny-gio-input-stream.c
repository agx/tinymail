/* libtinymail-gio - The Tiny Mail base library for Gio
 * Copyright (C) 2009 Javier Jardón <javierjc1982@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with self library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * A Gio to CamelStream mapper.
 */

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <tny-gio-input-stream.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGioInputStreamPriv TnyGioInputStreamPriv;

struct _TnyGioInputStreamPriv
{
        GInputStream *input_stream;
	gboolean eos;
	off_t position;		/* current postion in the input stream */
	off_t bound_start;	/* first valid position */
	off_t bound_end;	/* first invalid position */
};

#define TNY_GIO_INPUT_STREAM_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GIO_INPUT_STREAM, TnyGioInputStreamPriv))


static gssize
tny_gio_input_stream_write_to_stream (TnyStream *self, TnyStream *output)
{
        g_warning (  "You can't use the tny_stream_write_to_stream API on input "
		     "streams. This problem indicates a bug in the software");

        return -1;
}

static gssize
tny_gio_input_stream_read  (TnyStream *self, char *buffer, gsize n)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

        gssize nread = 0;
        GError *error = NULL;

	if (priv->bound_end != (~0))
		n = MIN (priv->bound_end - priv->position, n);

        nread = g_input_stream_read (priv->input_stream, buffer, n, NULL, &error);

        if (nread > 0)
		priv->position += nread;
	else if (nread == 0)
                priv->eos = TRUE;

	return nread;
}

static gssize
tny_gio_input_stream_write (TnyStream *self, const char *buffer, gsize n)
{
        g_warning (  "You can't use the tny_stream_write API on input "
		     "streams. This problem indicates a bug in the software");

        return -1;
}

static gint
tny_gio_input_stream_close (TnyStream *self)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);
	GError *error;

        if (!g_input_stream_close (priv->input_stream, NULL, &error))
                return -1;

        priv->input_stream = NULL;
        priv->eos = TRUE;

	return 0;
}


/**
 * tny_gio_input_stream_set_stream:
 * @self: A #TnyGioStream instance
 * @input_stream: the stream to read from
 *
 * Set the input stream to play adaptor for
 *
 **/
void
tny_gio_input_stream_set_stream (TnyGioInputStream *self, GInputStream *input_stream)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);
        GError *error;

        if (!input_stream)
                return;

	if (priv->input_stream) {
                g_input_stream_close (priv->input_stream, NULL, &error);
		priv->input_stream = NULL;
	}

	priv->input_stream = input_stream;
	priv->eos = FALSE;
	priv->position = 0;

        g_seekable_seek (G_SEEKABLE (input_stream), priv->position,
                         G_SEEK_CUR, NULL, &error);

	return;
}

/**
 * tny_gio_input_stream_new:
 * @input_stream: The stream to read from
 *
 * Create an adaptor instance between #TnyStream and #Gio input stream
 *
 * Return value: a new #TnyStream instance
 **/
TnyStream*
tny_gio_input_stream_new (GInputStream *input_stream)
{
	TnyGioInputStream *self = g_object_new (TNY_TYPE_GIO_INPUT_STREAM, NULL);

	tny_gio_input_stream_set_stream (self, input_stream);

	return TNY_STREAM (self);
}

static void
tny_gio_input_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGioInputStream *self = (TnyGioInputStream *)instance;
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

        priv->input_stream = NULL;
	priv->eos = FALSE;
	priv->bound_start = 0;
	priv->bound_end = (~0);
	priv->position = 0;

	return;
}

static void
tny_gio_input_stream_finalize (GObject *object)
{
	TnyGioInputStream *self = (TnyGioInputStream *)object;
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);
        GError *error = NULL;

        if (G_LIKELY (priv->input_stream))
                g_input_stream_close (priv->input_stream, NULL, &error);

	(*parent_class->finalize) (object);

	return;
}

static gint
tny_gio_input_flush (TnyStream *self)
{
         g_warning (  "You can't use the tny_stream_write_to_stream API on input "
		     "streams. This problem indicates a bug in the software");

        return -1;
}

static gboolean
tny_gio_input_is_eos (TnyStream *self)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

	return priv->eos;
}

static gint
tny_gio_input_reset (TnyStream *self)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);
	GError *error;

	if (priv->input_stream == NULL)
	{
		errno = EINVAL;
		return -1;
	}

        if (!g_seekable_seek (G_SEEKABLE (priv->input_stream), 0, G_SEEK_SET, NULL, &error))
                return -1;

        priv->position = 0;
	priv->eos = FALSE;

	return 0;
}



static off_t
tny_gio_input_seek (TnySeekable *self, off_t offset, int policy)//TODO
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

        goffset real = 0;
        GError *error;

	switch (policy) {
	case SEEK_SET:
		real = offset;
		break;
	case SEEK_CUR:
		real = priv->position + offset;
		break;
	case SEEK_END:
		if (priv->bound_end == (~0)) {
			if (!g_seekable_seek (G_SEEKABLE (priv->input_stream), (goffset) offset, G_SEEK_SET, NULL, &error))
                                return -1;
                        real = g_seekable_tell (G_SEEKABLE (priv->input_stream));
			if (real != -1) {
				if (real<priv->bound_start)
					real = priv->bound_start;
				priv->position = real;
			}
			return real;
		}
		real = priv->bound_end + offset;
		break;
	}

	if (priv->bound_end != (~0))
		real = MIN (real, priv->bound_end);
	real = MAX (real, priv->bound_start);

        if (!g_seekable_seek (G_SEEKABLE (priv->input_stream), real, G_SEEK_SET, NULL, &error))
                return -1;

	if (real != priv->position && priv->eos)
		priv->eos = FALSE;

	priv->position = real;

	return real;
}

static off_t
tny_gio_input_tell (TnySeekable *self)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

	return priv->position;
}

static gint
tny_gio_input_set_bounds (TnySeekable *self, off_t start, off_t end)
{
	TnyGioInputStreamPriv *priv = TNY_GIO_INPUT_STREAM_GET_PRIVATE (self);

	priv->bound_end = end;
	priv->bound_start = start;

	return 0;
}

static void
tny_stream_init (gpointer g, gpointer iface_data)
{
	TnyStreamIface *klass = (TnyStreamIface *)g;

	klass->reset= tny_gio_input_reset;
	klass->flush= tny_gio_input_flush;
	klass->is_eos= tny_gio_input_is_eos;
	klass->read= tny_gio_input_stream_read;
	klass->write= tny_gio_input_stream_write;
	klass->close= tny_gio_input_stream_close;
	klass->write_to_stream= tny_gio_input_stream_write_to_stream;

	return;
}


static void
tny_seekable_init (gpointer g, gpointer iface_data)
{
	TnySeekableIface *klass = (TnySeekableIface *)g;

	klass->seek= tny_gio_input_seek;
	klass->tell= tny_gio_input_tell;
	klass->set_bounds= tny_gio_input_set_bounds;

	return;
}

static void
tny_gio_input_stream_class_init (TnyGioInputStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gio_input_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGioInputStreamPriv));

	return;
}

static gpointer
tny_gio_input_stream_register_type (gpointer notused)
{
	GType type = 0;

	static const GTypeInfo info =
		{
			sizeof (TnyGioInputStreamClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_gio_input_stream_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGioInputStream),
			0,      /* n_preallocs */
			tny_gio_input_stream_instance_init,/* instance_init */
			NULL
		};

	static const GInterfaceInfo tny_stream_info =
		{
			(GInterfaceInitFunc) tny_stream_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

	static const GInterfaceInfo tny_seekable_info =
		{
			(GInterfaceInitFunc) tny_seekable_init, /* interface_init */
			NULL,         /* interface_finalize */
			NULL          /* interface_data */
		};

	type = g_type_register_static (G_TYPE_OBJECT,
				       "TnyVfsStream",
				       &info, 0);

	g_type_add_interface_static (type, TNY_TYPE_STREAM,
				     &tny_stream_info);

	g_type_add_interface_static (type, TNY_TYPE_SEEKABLE,
				     &tny_seekable_info);

	return GUINT_TO_POINTER (type);
}

GType
tny_gio_input_stream_get_type (void)
{
	static GOnce once = G_ONCE_INIT;
	g_once (&once, tny_gio_input_stream_register_type, NULL);
	return GPOINTER_TO_UINT (once.retval);
}
