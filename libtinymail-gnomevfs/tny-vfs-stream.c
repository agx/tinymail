/* libtinymail-gnomevfs - The Tiny Mail base library for GnomeVFS
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

/*  Original file: em-vfs-stream.c (Evolution source code)
 *
 *  Authors:  Michael Zucchi <notzed@ximian.com>
 *
 *  Copyright 2004 Ximian, Inc. (www.ximian.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * A GnomeVFS to CamelStream mapper.
 */


#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <tny-stream-iface.h>
#include <tny-vfs-stream.h>

static GObjectClass *parent_class = NULL;

typedef struct _TnyVfsStreamPriv TnyVfsStreamPriv;

struct _TnyVfsStreamPriv
{
	GnomeVFSHandle *handle;
	gboolean eos;
};

#define TNY_VFS_STREAM_GET_PRIVATE(o)	\
	(G_TYPE_INSTANCE_GET_PRIVATE ((o), TNY_TYPE_VFS_STREAM, TnyVfsStreamPriv))


static void
tny_vfs_stream_set_errno (GnomeVFSResult res)
{
	switch(res) {
	case GNOME_VFS_OK:
		g_warning("tny-vfs-stream: calling set_errno with no error");
		break;
	case GNOME_VFS_ERROR_NOT_FOUND:
	case GNOME_VFS_ERROR_HOST_NOT_FOUND:
	case GNOME_VFS_ERROR_INVALID_HOST_NAME:
	case GNOME_VFS_ERROR_HOST_HAS_NO_ADDRESS:
	case GNOME_VFS_ERROR_SERVICE_NOT_AVAILABLE:
		errno = ENOENT;
		break;
	case GNOME_VFS_ERROR_GENERIC:
	case GNOME_VFS_ERROR_INTERNAL:
	case GNOME_VFS_ERROR_IO:
	case GNOME_VFS_ERROR_EOF: /* will be caught by read before here anyway */
	case GNOME_VFS_ERROR_SERVICE_OBSOLETE:
	case GNOME_VFS_ERROR_PROTOCOL_ERROR:
	default:
		errno = EIO;
		break;
	case GNOME_VFS_ERROR_BAD_PARAMETERS:
	case GNOME_VFS_ERROR_NOT_SUPPORTED:
	case GNOME_VFS_ERROR_INVALID_URI:
	case GNOME_VFS_ERROR_NOT_OPEN:
	case GNOME_VFS_ERROR_INVALID_OPEN_MODE:
	case GNOME_VFS_ERROR_NOT_SAME_FILE_SYSTEM:
		errno = EINVAL;
		break;
	case GNOME_VFS_ERROR_CORRUPTED_DATA: /* not sure about these */
	case GNOME_VFS_ERROR_WRONG_FORMAT:
	case GNOME_VFS_ERROR_BAD_FILE:
		errno = EBADF;
		break;
	case GNOME_VFS_ERROR_TOO_BIG:
		errno = E2BIG;
		break;
	case GNOME_VFS_ERROR_NO_SPACE:
		errno = ENOSPC;
		break;
	case GNOME_VFS_ERROR_READ_ONLY:
	case GNOME_VFS_ERROR_READ_ONLY_FILE_SYSTEM:
		errno = EROFS;
		break;
	case GNOME_VFS_ERROR_TOO_MANY_OPEN_FILES:
		errno = EMFILE;
		break;
	case GNOME_VFS_ERROR_NOT_A_DIRECTORY:
		errno = ENOTDIR;
		break;
	case GNOME_VFS_ERROR_IN_PROGRESS:
		errno = EINPROGRESS;
		break;
	case GNOME_VFS_ERROR_INTERRUPTED:
		errno = EINTR;
		break;
	case GNOME_VFS_ERROR_FILE_EXISTS:
		errno = EEXIST;
	case GNOME_VFS_ERROR_LOOP:
		errno = ELOOP;
		break;
	case GNOME_VFS_ERROR_ACCESS_DENIED:
	case GNOME_VFS_ERROR_NOT_PERMITTED:
	case GNOME_VFS_ERROR_LOGIN_FAILED:
		errno = EPERM;
		break;
	case GNOME_VFS_ERROR_IS_DIRECTORY:
	case GNOME_VFS_ERROR_DIRECTORY_NOT_EMPTY: /* ?? */
		errno = EISDIR;
		break;
	case GNOME_VFS_ERROR_NO_MEMORY:
		errno = ENOMEM;
		break;
	case GNOME_VFS_ERROR_CANCELLED:
		errno = EINTR;
		break;
	case GNOME_VFS_ERROR_DIRECTORY_BUSY:
		errno = EBUSY;
		break;
	case GNOME_VFS_ERROR_TOO_MANY_LINKS:
		errno = EMLINK;
		break;
	case GNOME_VFS_ERROR_NAME_TOO_LONG:
		errno = ENAMETOOLONG;
		break;
	}
}

static ssize_t
tny_vfs_stream_write_to_stream (TnyStreamIface *self, TnyStreamIface *output)
{
	char tmp_buf[4096];
	ssize_t total = 0;
	ssize_t nb_read;
	ssize_t nb_written;
	
	while (G_UNLIKELY (!tny_stream_iface_eos (self))) {
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
tny_vfs_stream_read  (TnyStreamIface *self, char *buffer, size_t n)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	GnomeVFSFileSize count;
	GnomeVFSResult res;

	if (G_UNLIKELY (priv->handle == NULL))
	{
		errno = EINVAL;
		return -1;
	}

	res = gnome_vfs_read (priv->handle, buffer, n, &count);

	if (G_LIKELY (res == GNOME_VFS_OK))
	{
		priv->eos = FALSE;
		return (ssize_t)count;

	} else if (G_LIKELY (res == GNOME_VFS_ERROR_EOF))
	{
		priv->eos = TRUE;
		return (ssize_t)count;
	}

	tny_vfs_stream_set_errno (res);

	return -1;
}

static ssize_t
tny_vfs_stream_write (TnyStreamIface *self, const char *buffer, size_t n)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	GnomeVFSFileSize count;
	GnomeVFSResult res;

	if (G_UNLIKELY (priv->handle == NULL))
	{
		errno = EINVAL;
		return -1;
	}

	res = gnome_vfs_write (priv->handle, buffer, n, &count);
	if (G_LIKELY (res == GNOME_VFS_OK))
		return (ssize_t)count;

	tny_vfs_stream_set_errno (res);

	return -1;

}

static gint
tny_vfs_stream_close (TnyStreamIface *self)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	GnomeVFSResult res;

	if (priv->handle == NULL) 
	{
		errno = EINVAL;
		return -1;
	}

	res = gnome_vfs_close (priv->handle);

	priv->handle = NULL;

	if (res == GNOME_VFS_OK)
		return 0;

	tny_vfs_stream_set_errno (res);

	return -1;
}

/*

static off_t
tny_vfs_stream_seek (TnySeekableStream *stream, off_t offset, TnyStreamSeekPolicy policy)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	GnomeVFSSeekPosition vpolicy;
	GnomeVFSFileSize pos;
	GnomeVFSResult res;

	if (priv->handle == NULL) 
	{
		errno = EINVAL;
		return -1;
	}

	switch (policy) 
	{
		case TNY_STREAM_SET:
		default:
			vpolicy = GNOME_VFS_SEEK_START;
			break;
		case TNY_STREAM_CUR:
			vpolicy = GNOME_VFS_SEEK_CURRENT;
			break;
		case TNY_STREAM_END:
			vpolicy = GNOME_VFS_SEEK_END;
			break;
	}

	if ( (res = gnome_vfs_seek (priv->handle, vpolicy, offset)) == GNOME_VFS_OK
	     && (res = gnome_vfs_tell(priv->handle, &pos)) == GNOME_VFS_OK)
		return pos;

	tny_vfs_stream_set_errno (res);

	return -1;
}

static off_t
tny_vfs_stream_tell (TnySeekableStream *stream)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	GnomeVFSFileSize pos;
	GnomeVFSResult res;

	if (priv->handle == NULL) 
	{
		errno = EINVAL;
		return -1;
	}

	if ((res = gnome_vfs_tell (priv->handle, &pos)) == GNOME_VFS_OK)
		return pos;

	tny_vfs_stream_set_errno (res);

	return -1;
}

*/

/**
 * tny_vfs_stream_set_handle:
 * @self: A #TnyVfsStream instance
 * @handle: The #GnomeVFSHandle to write to or read from
 *
 * Set the #GnomeVFSHandle to play adaptor for
 *
 **/
void
tny_vfs_stream_set_handle (TnyVfsStream *self, GnomeVFSHandle *handle)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);

	if (priv->handle)
		gnome_vfs_close (priv->handle);

	priv->handle = handle;

	return;
}

/**
 * tny_vfs_stream_new:
 * @handle: The #GnomeVFSHandle to write to or read from
 *
 * Create an adaptor instance between #TnyStreamIface and #GnomeVFSHandle
 *
 * Return value: a new #TnyStreamIface instance
 **/
TnyVfsStream*
tny_vfs_stream_new (GnomeVFSHandle *handle)
{
	TnyVfsStream *self = g_object_new (TNY_TYPE_VFS_STREAM, NULL);

	tny_vfs_stream_set_handle (self, handle);

	return self;
}

static void
tny_vfs_stream_instance_init (GTypeInstance *instance, gpointer g_class)
{
	TnyVfsStream *self = (TnyVfsStream *)instance;
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);

	priv->eos = FALSE;
	priv->handle = NULL;

	return;
}

static void
tny_vfs_stream_finalize (GObject *object)
{
	TnyVfsStream *self = (TnyVfsStream *)object;	
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);

	if (G_LIKELY (priv->handle))
		gnome_vfs_close (priv->handle);

	(*parent_class->finalize) (object);

	return;
}

static gint 
tny_vfs_flush (TnyStreamIface *self)
{
	return 0;
}

static gboolean 
tny_vfs_eos (TnyStreamIface *self)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);

	return priv->eos;
}

static gint 
tny_vfs_reset (TnyStreamIface *self)
{
	TnyVfsStreamPriv *priv = TNY_VFS_STREAM_GET_PRIVATE (self);
	gint retval = -1;
	GnomeVFSResult res;

	if (priv->handle == NULL) 
	{
		errno = EINVAL;
		return -1;
	}

	res = gnome_vfs_seek (priv->handle, GNOME_VFS_SEEK_START, 0);

	if (res != GNOME_VFS_OK)
	{
		tny_vfs_stream_set_errno (res);
		retval = -1;
	}

	return retval;
}

static void
tny_stream_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyStreamIfaceClass *klass = (TnyStreamIfaceClass *)g_iface;

	klass->reset_func = tny_vfs_reset;
	klass->flush_func = tny_vfs_flush;
	klass->eos_func = tny_vfs_eos;

	klass->read_func = tny_vfs_stream_read;
	klass->write_func = tny_vfs_stream_write;
	klass->close_func = tny_vfs_stream_close;
	klass->write_to_stream_func = tny_vfs_stream_write_to_stream;

	return;
}

static void 
tny_vfs_stream_class_init (TnyVfsStreamClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->finalize = tny_vfs_stream_finalize;

	g_type_class_add_private (object_class, sizeof (TnyVfsStreamPriv));

	return;
}

GType 
tny_vfs_stream_get_type (void)
{
	static GType type = 0;

	if (G_UNLIKELY(type == 0))
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyVfsStreamClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_vfs_stream_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyVfsStream),
		  0,      /* n_preallocs */
		  tny_vfs_stream_instance_init    /* instance_init */
		};

		static const GInterfaceInfo tny_stream_iface_info = 
		{
		  (GInterfaceInitFunc) tny_stream_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyVfsStream",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_STREAM_IFACE, 
			&tny_stream_iface_info);
	}

	return type;
}
