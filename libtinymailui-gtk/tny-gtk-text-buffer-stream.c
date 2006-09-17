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
#include <config.h>
#include <string.h>
#include <glib.h>

#include <gtk/gtk.h>

#include <tny-stream.h>
#include <tny-gtk-text-buffer-stream.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyGtkTextBufferStreamPriv TnyGtkTextBufferStreamPriv;

struct _TnyGtkTextBufferStreamPriv
{
	GtkTextBuffer *buffer;
	GtkTextIter cur;

};

#define TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_GTK_TEXT_BUFFER_STREAM, TnyGtkTextBufferStreamPriv))


static gssize
tny_text_buffer_write_to_stream (TnyStream *self, TnyStream *output)
{
	char tmp_buf[4096];
	gssize total = 0;
	gssize nb_read;
	gssize nb_written;
	
	while (G_LIKELY (!tny_stream_is_eos (self))) 
	{
		nb_read = tny_stream_read (self, tmp_buf, sizeof (tmp_buf));
		if (G_UNLIKELY (nb_read < 0))
			return -1;
		else if (G_LIKELY (nb_read > 0)) {
			const gchar *end;
			if (!g_utf8_validate (tmp_buf, nb_read, &end)) 
				g_warning ("utf8 invalid: %d of %d", (gint)nb_read,
					   (gint)(end - tmp_buf));
				
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
tny_gtk_text_buffer_stream_read  (TnyStream *self, char *buffer, gsize n)
{
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);
	GtkTextIter dest, end;
	gchar *buf;
	gint cur_offset, end_offset, rlength;
	gtk_text_buffer_get_end_iter (priv->buffer, &end);

	cur_offset = gtk_text_iter_get_offset (&(priv->cur));    
	end_offset = gtk_text_iter_get_offset (&end);

	if (cur_offset + (gint)n > end_offset)
		rlength = end_offset - cur_offset;
	else rlength = (gint)n;

        gtk_text_buffer_get_start_iter (priv->buffer, &dest);
	gtk_text_iter_set_offset (&dest, rlength);

    
	buf = gtk_text_buffer_get_text (priv->buffer, &(priv->cur), &dest, TRUE);
    	strncpy (buffer, buf, rlength);
    	g_free (buf);
    	gtk_text_iter_set_offset (&(priv->cur), cur_offset + rlength);
    	
	return (gssize) rlength;
}

static gssize
tny_gtk_text_buffer_stream_write (TnyStream *self, const char *buffer, gsize n)
{
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	gtk_text_buffer_insert (priv->buffer, &(priv->cur), buffer, (gint)n);

	return (gssize) n;
}

static gint
tny_gtk_text_buffer_stream_flush (TnyStream *self)
{
	return 0;
}

static gint
tny_gtk_text_buffer_stream_close (TnyStream *self)
{
	tny_gtk_text_buffer_stream_flush (self);

	return 0;
}

static gboolean
tny_gtk_text_buffer_stream_is_eos   (TnyStream *self)
{
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);
	GtkTextIter end;
	gint end_offset, cur_offset;

	gtk_text_buffer_get_end_iter (priv->buffer, &end);

	cur_offset = gtk_text_iter_get_offset (&(priv->cur));
	end_offset = gtk_text_iter_get_offset (&end);
    
	return (cur_offset >= end_offset);
}

static gint
tny_gtk_text_buffer_stream_reset_priv (TnyGtkTextBufferStreamPriv *priv)
{
	gtk_text_buffer_get_start_iter (priv->buffer, &(priv->cur));

	return 0;
}

static gint
tny_gtk_text_buffer_stream_reset (TnyStream *self)
{
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	return tny_gtk_text_buffer_stream_reset_priv (priv);
}


/**
 * tny_gtk_text_buffer_stream_set_text_buffer:
 * @self: A #TnyGtkTextBufferStream instance
 * @buffer: The #GtkTextBuffer to write to or read from
 *
 * Set the #GtkTextBuffer to play adaptor for
 *
 **/
void
tny_gtk_text_buffer_stream_set_text_buffer (TnyGtkTextBufferStream *self, GtkTextBuffer *buffer)
{
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	if (priv->buffer)
		g_object_unref (G_OBJECT (priv->buffer));

	g_object_ref (G_OBJECT (buffer));
	priv->buffer = buffer;

	tny_gtk_text_buffer_stream_reset_priv (priv);

	return;
}

/**
 * tny_gtk_text_buffer_stream_new:
 * @buffer: The #GtkTextBuffer to write to or read from
 *
 * Create an adaptor instance between #TnyStream and #GtkTextBuffer
 *
 * Return value: a new #TnyStream instance
 **/
TnyStream*
tny_gtk_text_buffer_stream_new (GtkTextBuffer *buffer)
{
	TnyGtkTextBufferStream *self = g_object_new (TNY_TYPE_GTK_TEXT_BUFFER_STREAM, NULL);

	tny_gtk_text_buffer_stream_set_text_buffer (self, buffer);

	return TNY_STREAM (self);
}

static void
tny_gtk_text_buffer_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyGtkTextBufferStream *self = (TnyGtkTextBufferStream *)instance;
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	priv->buffer = NULL;

	return;
}

static void
tny_gtk_text_buffer_stream_finalize (GObject *object)
{
	TnyGtkTextBufferStream *self = (TnyGtkTextBufferStream *)object;	
	TnyGtkTextBufferStreamPriv *priv = TNY_GTK_TEXT_BUFFER_STREAM_GET_PRIVATE (self);

	if (priv->buffer)
		g_object_unref (G_OBJECT (priv->buffer));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_init (gpointer g, gpointer iface_data)
{
	TnyStreamIface *klass = (TnyStreamIface *)g;

	klass->read_func = tny_gtk_text_buffer_stream_read;
	klass->write_func = tny_gtk_text_buffer_stream_write;
	klass->flush_func = tny_gtk_text_buffer_stream_flush;
	klass->close_func = tny_gtk_text_buffer_stream_close;
	klass->is_eos_func = tny_gtk_text_buffer_stream_is_eos;
	klass->reset_func = tny_gtk_text_buffer_stream_reset;
	klass->write_to_stream_func = tny_text_buffer_write_to_stream;

	return;
}

static void 
tny_gtk_text_buffer_stream_class_init (TnyGtkTextBufferStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_gtk_text_buffer_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyGtkTextBufferStreamPriv));

	return;
}

GType 
tny_gtk_text_buffer_stream_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyGtkTextBufferStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_gtk_text_buffer_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyGtkTextBufferStream),
		  0,      /* n_preallocs */
		  tny_gtk_text_buffer_stream_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_info = 
		{
		  (GInterfaceInitFunc) tny_stream_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyGtkTextBufferStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM, 
			&tny_stream_info);
	}

	return type;
}
