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
#include <glib/gi18n-lib.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include <tny-stream.h>
#include <tny-moz-embed-stream.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyMozEmbedStreamPriv TnyMozEmbedStreamPriv;

struct _TnyMozEmbedStreamPriv
{
	GtkMozEmbed *embed;
	gboolean stream_open;
};

#define TNY_MOZ_EMBED_STREAM_GET_PRIVATE(o) \
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MOZ_EMBED_STREAM, TnyMozEmbedStreamPriv))

static gint tny_moz_embed_stream_close (TnyStream *self);

static ssize_t
tny_moz_embed_write_to_stream (TnyStream *self, TnyStream *output)
{
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;

	g_assert (TNY_IS_STREAM (output));

	while (G_LIKELY (!tny_stream_is_eos (self))) 
	{
		nb_read = tny_stream_read (self, tmp_buf, sizeof (tmp_buf));
		if (G_UNLIKELY (nb_read < 0))
			return -1;
		else if (G_LIKELY (nb_read > 0)) {
			nb_written = 0;
	
			while (G_LIKELY (nb_written < nb_read))
			{
				ssize_t len = tny_stream_write (output, tmp_buf + nb_written,
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
tny_moz_embed_stream_read  (TnyStream *self, char *data, size_t n)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);
	ssize_t retval = -1;

	/* TODO (atm this is unused, would only be useful for a writable
	   HTML component, for example for editing E-mails with it) */

	return retval;
}

static gint
tny_moz_embed_stream_reset (TnyStream *self)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	if (priv->stream_open)
		gtk_moz_embed_close_stream (priv->embed);
	gtk_moz_embed_render_data (priv->embed, "", 0, "file:///", "text/html");
	if (priv->stream_open)
		gtk_moz_embed_open_stream (priv->embed, "file:///", "text/html");

	return 0;
}

static ssize_t
tny_moz_embed_stream_write (TnyStream *self, const char *data, size_t n)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	if (!priv->stream_open)
		gtk_moz_embed_open_stream (priv->embed, "file:///", "text/html");

	gtk_moz_embed_append_data (priv->embed, data, n);

	return (ssize_t) n;
}

static gint
tny_moz_embed_stream_flush (TnyStream *self)
{
	return 0;
}

static gint
tny_moz_embed_stream_close (TnyStream *self)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	gtk_moz_embed_close_stream (priv->embed);
	priv->stream_open = FALSE;

	return 0;
}

static gboolean
tny_moz_embed_stream_is_eos   (TnyStream *self)
{
	return TRUE;
}


/**
 * tny_moz_embed_stream_set_moz_embed:
 * @self: A #TnyMozEmbedStream instance
 * @embed: The #GtkMozEmbed to write to or read from
 *
 * Set the #GtkMozEmbed to play adaptor for
 *
 **/
void
tny_moz_embed_stream_set_moz_embed (TnyMozEmbedStream *self, GtkMozEmbed *embed)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	if (priv->embed)
		g_object_unref (G_OBJECT (priv->embed));
	g_object_ref (G_OBJECT (embed));

	priv->embed = embed;
	priv->stream_open = FALSE;

	return;
}

/**
 * tny_moz_embed_stream_new:
 * @embed: The #GtkMozEmbed to write to or read from
 *
 * Create an adaptor instance between #TnyStream and #GtkMozEmbed
 *
 * Return value: a new #TnyStream instance
 **/
TnyStream*
tny_moz_embed_stream_new (GtkMozEmbed *embed)
{
	TnyMozEmbedStream *self = g_object_new (TNY_TYPE_MOZ_EMBED_STREAM, NULL);

	tny_moz_embed_stream_set_moz_embed (self, embed);

	return TNY_STREAM (self);
}

static void
tny_moz_embed_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMozEmbedStream *self = (TnyMozEmbedStream *)instance;
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	priv->embed = NULL;
	priv->stream_open = FALSE;

	return;
}

static void
tny_moz_embed_stream_finalize (GObject *object)
{
	TnyMozEmbedStream *self = (TnyMozEmbedStream *)object;	
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	if (priv->embed)
	{
		if (priv->stream_open)
			tny_moz_embed_stream_close (TNY_STREAM (self));
		g_object_unref (G_OBJECT (priv->embed));
	}

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_init (gpointer g, gpointer iface_data)
{
	TnyStreamIface *klass = (TnyStreamIface *)g;

	klass->read_func = tny_moz_embed_stream_read;
	klass->write_func = tny_moz_embed_stream_write;
	klass->flush_func = tny_moz_embed_stream_flush;
	klass->close_func = tny_moz_embed_stream_close;
	klass->is_eos_func = tny_moz_embed_stream_is_eos;
	klass->reset_func = tny_moz_embed_stream_reset;
	klass->write_to_stream_func = tny_moz_embed_write_to_stream;

	return;
}

static void 
tny_moz_embed_stream_class_init (TnyMozEmbedStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_moz_embed_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyMozEmbedStreamPriv));

	return;
}

GType 
tny_moz_embed_stream_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMozEmbedStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_moz_embed_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyMozEmbedStream),
		  0,      /* n_preallocs */
		  tny_moz_embed_stream_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_info = 
		{
		  (GInterfaceInitFunc) tny_stream_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMozEmbedStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM, 
			&tny_stream_info);
	}

	return type;
}
