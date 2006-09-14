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

/* Original file: camel-stream-fs.c : file system based stream (Camel 
 * source code). Note that this implementation is heavily modified and that
 * therefore bugs aren't necessarily caused by the original authors.
 */

/*
 * Authors: Bertrand Guiheneuf <bertrand@helixcode.com>
 *	    Michael Zucchi <notzed@ximian.com>
 *
 * Copyright 1999-2003 Ximian, Inc. (www.ximian.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <tny-stream.h>
#include <tny-fs-stream.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyFsStreamPriv TnyFsStreamPriv;

struct _TnyFsStreamPriv
{
	int fd;
	gint offset;
	gboolean eos;
};

#define TNY_FS_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_FS_STREAM, TnyFsStreamPriv))


static gssize
tny_fs_stream_write_to_stream (TnyStream *self, TnyStream *output)
{
	char tmp_buf[4096];
	gssize total = 0;
	gssize nb_read;
	gssize nb_written;
	
	while (G_UNLIKELY (!tny_stream_eos (self))) {
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
tny_fs_stream_read  (TnyStream *self, char *buffer, gsize n)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);
	gssize nread;
	char b[1];
	
	if ((nread = read (priv->fd, buffer, n)) > 0)
		priv->offset += nread;
    
    	if (nread != n)
		priv->eos = TRUE;
	
    	if (read (priv->fd, b, 1) != 1)
		priv->eos = TRUE;
    
	priv->offset = lseek (priv->fd, priv->offset, SEEK_SET);
    
	return nread;

}

static gssize
tny_fs_stream_write (TnyStream *self, const char *buffer, gsize n)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);
	gssize nwritten;
		
	if ((nwritten = write (priv->fd, buffer, n)) > 0)
		priv->offset += nwritten;
	
    	priv->eos = FALSE;
    
	return nwritten;
}

static gint
tny_fs_stream_close (TnyStream *self)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);

	if (close (priv->fd) == -1)
		return -1;
	
	priv->fd = -1;

	return 0;
}


/**
 * tny_fs_stream_set_fd:
 * @self: A #TnyFsStream instance
 * @fd: The file descriptor to write to or read from
 *
 * Set the file descriptor to play adaptor for
 *
 **/
void
tny_fs_stream_set_fd (TnyFsStream *self, int fd)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);

	if (fd == -1)
		return;

	if (priv->fd != -1)
		close (priv->fd);

	priv->fd = fd;

	priv->offset = lseek (priv->fd, 0, SEEK_SET);

	if (priv->offset == -1)
		priv->offset = 0;

	priv->eos = FALSE;
    
	return;
}

/**
 * tny_fs_stream_new:
 * @fd: The file descriptor to write to or read from
 *
 * Create an adaptor instance between #TnyStream and a file descriptor. Note 
 * that you must not to close() fd yourself. The destructor will do that for
 * you.
 *
 * Therefore use it with care. It's more or less an exception in the framework.
 *
 * The the instance will on top of close() when destructing, also fsync() the
 * filedescriptor. It does this depending on its mood, the weather and your
 * wives periods using a complex algorithm that abuses your privacy and might
 * kill your cat and dog.
 *
 * Return value: a new #TnyStream instance
 **/
TnyStream*
tny_fs_stream_new (int fd)
{
	TnyFsStream *self = g_object_new (TNY_TYPE_FS_STREAM, NULL);

	tny_fs_stream_set_fd (self, fd);

	return TNY_STREAM (self);
}

static void
tny_fs_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyFsStream *self = (TnyFsStream *)instance;
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);

	priv->eos = TRUE;
	priv->fd = -1;
	priv->offset = 0;
    
	return;
}

static void
tny_fs_stream_finalize (GObject *object)
{
	TnyFsStream *self = (TnyFsStream *)object;	
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);

	if (priv->fd != -1)
 	{
		fsync (priv->fd);
		close (priv->fd);	     
	}
	priv->fd = -1;
	priv->eos = TRUE;
    
	(*parent_class->finalize) (object);

	return;
}

static gint 
tny_fs_flush (TnyStream *self)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);
	return fsync(priv->fd);
}

static gboolean 
tny_fs_eos (TnyStream *self)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);
	return priv->eos;
}

static gint 
tny_fs_reset (TnyStream *self)
{
	TnyFsStreamPriv *priv = TNY_FS_STREAM_GET_PRIVATE (self);
    	priv->offset = lseek (priv->fd, 0, SEEK_SET);
    	priv->eos = FALSE;
	return 0;
}

static void
tny_stream_init (gpointer g, gpointer iface_data)
{
	TnyStreamIface *klass = (TnyStreamIface *)g;

	klass->reset_func = tny_fs_reset;
	klass->flush_func = tny_fs_flush;
	klass->eos_func = tny_fs_eos;

	klass->read_func = tny_fs_stream_read;
	klass->write_func = tny_fs_stream_write;
	klass->close_func = tny_fs_stream_close;
	klass->write_to_stream_func = tny_fs_stream_write_to_stream;

	return;
}

static void 
tny_fs_stream_class_init (TnyFsStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_fs_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyFsStreamPriv));

	return;
}

GType 
tny_fs_stream_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyFsStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_fs_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyFsStream),
		  0,      /* n_preallocs */
		  tny_fs_stream_instance_init,   /* instance_init */
		  NULL
		};

		static const GInterfaceInfo tny_stream_info = 
		{
		  (GInterfaceInitFunc) tny_stream_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyFsStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM, 
			&tny_stream_info);
	}

	return type;
}

