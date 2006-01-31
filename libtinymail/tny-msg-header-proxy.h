#ifndef TNY_MSG_HEADER_PROXY_H
#define TNY_MSG_HEADER_PROXY_H

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
#include <tny-msg-header-iface.h>
#include <tny-msg-header.h>

G_BEGIN_DECLS

#define TNY_MSG_HEADER_PROXY_TYPE             (tny_msg_header_proxy_get_type ())
#define TNY_MSG_HEADER_PROXY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_MSG_HEADER_PROXY_TYPE, TnyMsgHeaderProxy))
#define TNY_MSG_HEADER_PROXY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_MSG_HEADER_PROXY_TYPE, TnyMsgHeaderProxyClass))
#define TNY_IS_MSG_PROXY_HEADER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_MSG_HEADER_PROXY_TYPE))
#define TNY_IS_MSG_HEADER_PROXY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_MSG_HEADER_PROXY_TYPE))
#define TNY_MSG_HEADER_PROXY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_MSG_HEADER_PROXY_TYPE, TnyMsgHeaderProxyClass))

typedef struct _TnyMsgHeaderProxy TnyMsgHeaderProxy;
typedef struct _TnyMsgHeaderProxyClass TnyMsgHeaderProxyClass;

struct _TnyMsgHeaderProxy 
{
	GObject parent;

	/* Shouldn't this be private? */
	TnyMsgHeader *real;
};

struct _TnyMsgHeaderProxyClass
{
	GObjectClass parent;
};

GType               tny_msg_header_proxy_get_type  (void);
TnyMsgHeaderProxy*  tny_msg_header_proxy_new       (void);

G_END_DECLS

#endif
