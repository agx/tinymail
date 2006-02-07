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

#include <tny-msg-account-iface.h>

void
tny_msg_account_iface_set_forget_pass_func (TnyMsgAccountIface *self, ForgetPassFunc get_forget_pass_func)
{
	TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->set_forget_pass_func_func (self, get_forget_pass_func);
	return;
}

ForgetPassFunc
tny_msg_account_iface_get_forget_pass_func (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_forget_pass_func_func (self);
}

const GList*
tny_msg_account_iface_get_folders (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_folders_func (self);
}

void
tny_msg_account_iface_set_proto (TnyMsgAccountIface *self, const gchar *proto)
{
	TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->set_proto_func (self, proto);
	return;
}

void
tny_msg_account_iface_set_user (TnyMsgAccountIface *self, const gchar *user)
{
	TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->set_user_func (self, user);
	return;
}

void
tny_msg_account_iface_set_hostname (TnyMsgAccountIface *self, const gchar *host)
{
	TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->set_hostname_func (self, host);
	return;
}

void
tny_msg_account_iface_set_pass_func (TnyMsgAccountIface *self, GetPassFunc get_pass_func)
{
	TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->set_pass_func_func (self, get_pass_func);
	return;
}

const gchar*
tny_msg_account_iface_get_proto (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_proto_func (self);
}

const gchar*
tny_msg_account_iface_get_user (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_user_func (self);
}

const gchar*
tny_msg_account_iface_get_hostname (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_hostname_func (self);
}

GetPassFunc
tny_msg_account_iface_get_pass_func (TnyMsgAccountIface *self)
{
	return TNY_MSG_ACCOUNT_IFACE_GET_CLASS (self)->get_pass_func_func (self);
}


static void
tny_msg_account_iface_base_init (gpointer g_class)
{
	static gboolean initialized = FALSE;

	if (!initialized) {
		/* create interface signals here. */
		initialized = TRUE;
	}
}

GType
tny_msg_account_iface_get_type (void)
{
	static GType type = 0;
	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyMsgAccountIfaceClass),
		  tny_msg_account_iface_base_init,   /* base_init */
		  NULL,   /* base_finalize */
		  NULL,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  0,
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_INTERFACE, 
			"TnyMsgAccountIface", &info, 0);

		g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);
	}

	return type;
}


