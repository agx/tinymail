/* libtinymailui-gtk - The Tiny Mail UI library for Gtk+
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

#include <gtk/gtk.h>

#include <tny-stream-iface.h>
#include <tny-text-buffer-stream.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyTextBufferStreamPriv TnyTextBufferStreamPriv;

struct _TnyTextBufferStreamPriv
{
	GtkTextBuffer *buffer;
	GtkTextIter cur;

};

#define TNY_TEXT_BUFFER_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TEXT_BUFFER_STREAM_TYPE, TnyTextBufferStreamPriv))


static ssize_t
tny_text_buffer_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;
	
	while (!tny_stream_iface_eos (self)) {
		nb_read = tny_stream_iface_read (self, tmp_buf, sizeof (tmp_buf));
		if (nb_read < 0)
			return -1;
		else if (nb_read > 0) {
			nb_written = 0;
	
			while (nb_written < nb_read) {
				ssize_t len = tny_stream_iface_write (output, tmp_buf + nb_written,
								  nb_read - nb_written);
				if (len < 0)
					return -1;
				nb_written += len;
			}
			total += nb_written;
		}
	}
	return total;
}

static ssize_t
tny_text_buffer_stream_read  (TnyStreamIface *self, char *buffer, size_t n)
{
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);
	GtkTextIter dest, end;
	gint cur_offset, end_offset, rlength;

	gtk_text_buffer_get_end_iter (priv->buffer, &end);

	cur_offset = gtk_text_iter_get_offset (&(priv->cur));
	end_offset = gtk_text_iter_get_offset (&end);

	if (cur_offset + (gint)n > end_offset)
		rlength = end_offset - cur_offset;
	else rlength = (gint)n;

	gtk_text_iter_set_offset (&dest, rlength);

	buffer = gtk_text_buffer_get_text (priv->buffer, &(priv->cur), &dest, TRUE);

	return (ssize_t) rlength;
}

static ssize_t
tny_text_buffer_stream_write (TnyStreamIface *self, const char *buffer, size_t n)
{
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	gtk_text_buffer_insert (priv->buffer, &(priv->cur), buffer, (gint)n);

	return (ssize_t) n;
}

static gint
tny_text_buffer_stream_flush (TnyStreamIface *self)
{
	return 0;
}

static gint
tny_text_buffer_stream_close (TnyStreamIface *self)
{
	tny_text_buffer_stream_flush (self);

	return 0;
}

static gboolean
tny_text_buffer_stream_eos   (TnyStreamIface *self)
{
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);
	GtkTextIter end;

	gtk_text_buffer_get_end_iter (priv->buffer, &end);

	return gtk_text_iter_equal (&(priv->cur), &end);
}

static gint
tny_text_buffer_stream_reset_priv (TnyTextBufferStreamPriv *priv)
{
	gtk_text_buffer_get_start_iter (priv->buffer, &(priv->cur));

	return 0;
}

static gint
tny_text_buffer_stream_reset (TnyStreamIface *self)
{
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	return tny_text_buffer_stream_reset_priv (priv);
}


/**
 * tny_text_buffer_stream_set_text_buffer:
 * @self: A #TnyTextBufferStream instance
 * @buffer: The #GtkTextBuffer to write to or read from
 *
 * Set the #GtkTextBuffer to play adaptor for
 *
 **/
void
tny_text_buffer_stream_set_text_buffer (TnyTextBufferStream *self, GtkTextBuffer *buffer)
{
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	if (priv->buffer)
		g_object_unref (G_OBJECT (priv->buffer));

	g_object_ref (G_OBJECT (buffer));
	priv->buffer = buffer;

	tny_text_buffer_stream_reset_priv (priv);

	return;
}

/**
 * tny_text_buffer_stream_new:
 * @buffer: The #GtkTextBuffer to write to or read from
 *
 * Create an adaptor instance between #TnyStreamIface and #GtkTextBuffer
 *
 * Return value: a new #TnyStreamIface instance
 **/
TnyTextBufferStream*
tny_text_buffer_stream_new (GtkTextBuffer *buffer)
{
	TnyTextBufferStream *self = g_object_new (TNY_TEXT_BUFFER_STREAM_TYPE, NULL);

	tny_text_buffer_stream_set_text_buffer (self, buffer);

	return self;
}

static void
tny_text_buffer_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyTextBufferStream *self = (TnyTextBufferStream *)instance;
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	priv->buffer = NULL;

	return;
}

static void
tny_text_buffer_stream_finalize (GObject *object)
{
	TnyTextBufferStream *self = (TnyTextBufferStream *)object;	
	TnyTextBufferStreamPriv *priv = TNY_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	if (priv->buffer)
		g_object_unref (G_OBJECT (priv->buffer));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStreamIfaceClass *klass = (TnyStreamIfaceClass *)g_iface;

	klass->read_func = tny_text_buffer_stream_read;
	klass->write_func = tny_text_buffer_stream_write;
	klass->flush_func = tny_text_buffer_stream_flush;
	klass->close_func = tny_text_buffer_stream_close;
	klass->eos_func = tny_text_buffer_stream_eos;
	klass->reset_func = tny_text_buffer_stream_reset;
	klass->write_to_stream_func = tny_text_buffer_write_to_stream;

	return;
}

static void 
tny_text_buffer_stream_class_init (TnyTextBufferStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_text_buffer_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyTextBufferStreamPriv));

	return;
}

GType 
tny_text_buffer_stream_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyTextBufferStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_text_buffer_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyTextBufferStream),
		  0,      /* n_preallocs */
		  tny_text_buffer_stream_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_iface_info = 
		{
		  (GInterfaceInitFunc) tny_stream_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyTextBufferStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_STREAM_IFACE_TYPE, 
			&tny_stream_iface_info);
	}

	return type;
}
