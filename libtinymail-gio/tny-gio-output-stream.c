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

#include <tny-gio-output-stream.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyGioOutputStreamPriv TnyGioOutputStreamPriv;

struct _TnyGioOutputStreamPriv
{
        GOutputStream *output_stream;
	gboolean eos;
	off_t position;		/* current postion in the input stream */
	off_t bound_start;	/* first valid position */
	off_t bound_end;	/* first invalid position */
};

#define TNY_GIO_OUTPUT_STREAM_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GIO_OUTPUT_STREAM, TnyGioOutputStreamPriv))


static gssize
tny_gio_output_stream_write_to_stream (TnyStream *self, TnyStream *output)
{
	char tmp_buf[4096];
	gssize total = 0;
	gssize nb_read;
	gssize nb_written;

	g_assert (TNY_IS_STREAM (output));

	while (G_UNLIKELY (!tny_stream_is_eos (self))) {
		nb_read = tny_stream_read (self, tmp_buf, sizeof (tmp_buf));
		if (G_UNLIKELY (nb_read < 0))
			return -1;
		else if (G_LIKELY (nb_read > 0)) {
			nb_written = 0;

			while (G_LIKELY (nb_written < nb_read))
			{
				gssize len = tny_stream_write (output, tmp_buf + nb_written,
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

static gssize
tny_gio_output_stream_read  (TnyStream *self, char *buffer, gsize n)
{
        g_warning (  "You can't use the tny_stream_read API on output "
		     "streams. This problem indicates a bug in the software");

	return -1;
}

static gssize
tny_gio_output_stream_write (TnyStream *self, const char *buffer, gsize n)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

        gssize nwritten = 0;
        GError *error = NULL;

	if (priv->bound_end != (~0))
		n = MIN (priv->bound_end - priv->position, n);

        nwritten = g_output_stream_write (priv->output_stream, buffer, n, NULL, &error);

	if (nwritten > 0)
		priv->position += nwritten;
	else if (nwritten == 0)
                priv->eos = TRUE;

	return nwritten;
}

static gint
tny_gio_output_stream_close (TnyStream *self)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);
	GError *error;

        if (!g_output_stream_close (priv->output_stream, NULL, &error))
                return -1;

        priv->output_stream = NULL;
        priv->eos = TRUE;

	return 0;
}


/**
 * tny_gio_output_stream_set_stream:
 * @self: A #TnyGioStream instance
 * @output_stream: the stream to write to
 *
 * Set the output stream to play adaptor for
 *
 **/
void
tny_gio_output_stream_set_stream (TnyGioOutputStream *self, GOutputStream *output_stream)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);
        GError *error;

        if (!output_stream)
                return;

	if (priv->output_stream) {
                g_output_stream_close (priv->output_stream, NULL, &error);
		priv->output_stream = NULL;
	}

	priv->output_stream = output_stream;
	priv->eos = FALSE;
	priv->position = 0;

        g_seekable_seek (G_SEEKABLE (output_stream), priv->position,
                         G_SEEK_CUR, NULL, &error);

	return;
}

/**
 * tny_gio_output_stream_new:
 * @output_stream: The stream to read from
 *
 * Create an adaptor instance between #TnyStream and #Gio input stream
 *
 * Return value: a new #TnyStream instance
 **/
TnyStream*
tny_gio_output_stream_new (GOutputStream *output_stream)
{
	TnyGioOutputStream *self = g_object_new (TNY_TYPE_GIO_OUTPUT_STREAM, NULL);

	tny_gio_output_stream_set_stream (self, output_stream);

	return TNY_STREAM (self);
}

static void
tny_gio_output_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGioOutputStream *self = (TnyGioOutputStream *)instance;
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

        priv->output_stream = NULL;
	priv->eos = FALSE;
	priv->bound_start = 0;
	priv->bound_end = (~0);
	priv->position = 0;

	return;
}

static void
tny_gio_output_stream_finalize (GObject *object)
{
	TnyGioOutputStream *self = (TnyGioOutputStream *)object;
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);
        GError *error = NULL;

        if (G_LIKELY (priv->output_stream))
                g_output_stream_close (priv->output_stream, NULL, &error);

	(*parent_class->finalize) (object);

	return;
}

static gint
tny_gio_output_flush (TnyStream *self)
{
        TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);
        GError *error;

        if (!g_output_stream_flush (priv->output_stream, NULL, &error))
                return -1;

	return 0;
}

static gboolean
tny_gio_output_is_eos (TnyStream *self)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

	return priv->eos;
}

static gint
tny_gio_output_reset (TnyStream *self)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);
	gint retval = 0;
	GError *error;

	if (priv->output_stream == NULL)
	{
		errno = EINVAL;
		return -1;
	}

        if (!g_seekable_seek (G_SEEKABLE (priv->output_stream), 0, G_SEEK_SET, NULL, &error))
                return -1;

        priv->position = 0;
	priv->eos = FALSE;

	return 0;
}



static off_t
tny_gio_output_seek (TnySeekable *self, off_t offset, int policy)//TODO
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

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
		  if (!g_seekable_seek (G_SEEKABLE (priv->output_stream), (goffset) offset, G_SEEK_SET, NULL, &error))
                                return -1;
                        real = g_seekable_tell (G_SEEKABLE (priv->output_stream));
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

        if (!g_seekable_seek (G_SEEKABLE (priv->output_stream), real, G_SEEK_SET, NULL, &error))
                return -1;

	if (real != priv->position && priv->eos)
		priv->eos = FALSE;

	priv->position = real;

	return real;
}

static off_t
tny_gio_output_tell (TnySeekable *self)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

	return priv->position;
}

static gint
tny_gio_output_set_bounds (TnySeekable *self, off_t start, off_t end)
{
	TnyGioOutputStreamPriv *priv = TNY_GIO_OUTPUT_STREAM_GET_PRIVATE (self);

	priv->bound_end = end;
	priv->bound_start = start;

	return 0;
}

static void
tny_stream_init (gpointer g, gpointer iface_data)
{
	TnyStreamIface *klass = (TnyStreamIface *)g;

	klass->reset= tny_gio_output_reset;
	klass->flush= tny_gio_output_flush;
	klass->is_eos= tny_gio_output_is_eos;
	klass->read= tny_gio_output_stream_read;
	klass->write= tny_gio_output_stream_write;
	klass->close= tny_gio_output_stream_close;
	klass->write_to_stream= tny_gio_output_stream_write_to_stream;

	return;
}


static void
tny_seekable_init (gpointer g, gpointer iface_data)
{
	TnySeekableIface *klass = (TnySeekableIface *)g;

	klass->seek= tny_gio_output_seek;
	klass->tell= tny_gio_output_tell;
	klass->set_bounds= tny_gio_output_set_bounds;

	return;
}

static void
tny_gio_output_stream_class_init (TnyGioOutputStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gio_output_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGioOutputStreamPriv));

	return;
}

static gpointer
tny_gio_output_stream_register_type (gpointer notused)
{
	GType type = 0;

	static const GTypeInfo info =
		{
			sizeof (TnyGioOutputStreamClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			(GClassInitFunc) tny_gio_output_stream_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (TnyGioOutputStream),
			0,      /* n_preallocs */
			tny_gio_output_stream_instance_init,/* instance_init */
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
tny_gio_output_stream_get_type (void)
{
	static GOnce once = G_ONCE_INIT;
	g_once (&once, tny_gio_output_stream_register_type, NULL);
	return GPOINTER_TO_UINT (once.retval);
}
