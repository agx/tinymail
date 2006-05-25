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
#include <stdlib.h>
#include <gtk/gtk.h>

#include <tny-stream-iface.h>
#include <tny-moz-embed-stream.h>
#include <gtkembedmoz/gtkmozembed.h>

static GObjectClass *parent_class = NULL;


typedef struct _TnyMozEmbedStreamPriv TnyMozEmbedStreamPriv;

struct _TnyMozEmbedStreamPriv
{
	GtkMozEmbed *embed;
	gchar *filename;
	gboolean htmlext;
};

#define TNY_MOZ_EMBED_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_MOZ_EMBED_STREAM, TnyMozEmbedStreamPriv))

static gint tny_moz_embed_stream_close (TnyStreamIface *self);

static ssize_t
tny_moz_embed_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;
	
	while (G_LIKELY (!tny_stream_iface_eos (self))) 
	{
		nb_read = tny_stream_iface_read (self, tmp_buf, sizeof (tmp_buf));
		if (G_UNLIKELY (nb_read < 0))
			return -1;
		else if (G_LIKELY (nb_read > 0)) {
			nb_written = 0;
	
			while (G_LIKELY (nb_written < nb_read))
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
tny_moz_embed_stream_read  (TnyStreamIface *self, char *data, size_t n)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);
	ssize_t retval = -1;

	/* TODO: Will always return the beginning of the file (but atm unused) */

	if (priv->filename)
	{
		FILE *file = fopen (priv->filename, "r");
		if (file)
		{
			retval = fread (data, 1, n, file);
			fclose (file);
		}
	}

	return retval;
}

static gint
tny_moz_embed_stream_reset (TnyStreamIface *self)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	if (priv->filename)
	{
		g_unlink (priv->filename);
		g_free (priv->filename);
	}

	priv->htmlext = FALSE;
	priv->filename = NULL;

	return 0;
}

static ssize_t
tny_moz_embed_stream_write (TnyStreamIface *self, const char *data, size_t n)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);
	FILE *file = NULL;

	if (!priv->filename || !g_file_test  (priv->filename, G_FILE_TEST_EXISTS))
	{
		gint ofile;
		gchar *tmpl, *filename;

		if (priv->filename)
			g_free (priv->filename);

		tmpl = g_strdup_printf ("tinymail.tnymozembedstream.%s.XXXXXX", g_get_user_name ());

		ofile = g_file_open_tmp ((const gchar*)tmpl, &filename, NULL);

		priv->filename = filename;
		priv->htmlext = FALSE;

		g_free (tmpl);


		file = fdopen (ofile, "a");
	}

	if (!file && priv->filename)
		file = fopen (priv->filename, "a");

	if (file && priv->filename)
	{

		/* Dear free software world, do you NOW see we are fucking
			things up?! This is insane! */

		gchar *str;

		fputs (data, file);
		fclose (file);

		if (!priv->htmlext)
		{
			gchar *better = g_strdup_printf ("%s.html", priv->filename);

			if (g_rename (priv->filename, better) != 0)
				g_warning ("Can't rename %s to %s\n", priv->filename, better);

			g_free (priv->filename);
			priv->filename = better;
			priv->htmlext = TRUE;
		}

		str = g_strdup_printf ("file://%s", priv->filename);
		
		gtk_moz_embed_load_url (priv->embed, (const gchar*)str);
		
		g_free (str);

	} else {
		g_warning (_("Can't write %s\n"), priv->filename);
	}

	return (ssize_t) n;
}

static gint
tny_moz_embed_stream_flush (TnyStreamIface *self)
{
	return 0;
}

static gint
tny_moz_embed_stream_close (TnyStreamIface *self)
{
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	tny_moz_embed_stream_reset (self);

	return 0;
}

static gboolean
tny_moz_embed_stream_eos   (TnyStreamIface *self)
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

	return;
}

/**
 * tny_moz_embed_stream_new:
 * @embed: The #GtkMozEmbed to write to or read from
 *
 * Create an adaptor instance between #TnyStreamIface and #GtkMozEmbed
 *
 * Return value: a new #TnyStreamIface instance
 **/
TnyMozEmbedStream*
tny_moz_embed_stream_new (GtkMozEmbed *embed)
{
	TnyMozEmbedStream *self = g_object_new (TNY_TYPE_MOZ_EMBED_STREAM, NULL);

	tny_moz_embed_stream_set_moz_embed (self, embed);

	return self;
}

static void
tny_moz_embed_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyMozEmbedStream *self = (TnyMozEmbedStream *)instance;
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	priv->embed = NULL;
	priv->filename = NULL;

	return;
}

static void
tny_moz_embed_stream_finalize (GObject *object)
{
	TnyMozEmbedStream *self = (TnyMozEmbedStream *)object;	
	TnyMozEmbedStreamPriv *priv = TNY_MOZ_EMBED_STREAM_GET_PRIVATE (self);

	tny_moz_embed_stream_reset (TNY_STREAM_IFACE (self));

	if (priv->embed)
		g_object_unref (G_OBJECT (priv->embed));

	(*parent_class->finalize) (object);

	return;
}

static void
tny_stream_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStreamIfaceClass *klass = (TnyStreamIfaceClass *)g_iface;

	klass->read_func = tny_moz_embed_stream_read;
	klass->write_func = tny_moz_embed_stream_write;
	klass->flush_func = tny_moz_embed_stream_flush;
	klass->close_func = tny_moz_embed_stream_close;
	klass->eos_func = tny_moz_embed_stream_eos;
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

		static const GInterfaceInfo tny_stream_iface_info = 
		{
		  (GInterfaceInitFunc) tny_stream_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyMozEmbedStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM_IFACE, 
			&tny_stream_iface_info);
	}

	return type;
}
