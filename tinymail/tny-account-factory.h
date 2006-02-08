#ifndef TNY_ACCOUNT_FACTORY_H
#define TNY_ACCOUNT_FACTORY_H

/*
 * Copyright (C) 2006-2007 Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with self program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <glib.h>
#include <glib-object.h>
#include <tny-shared.h>

G_BEGIN_DECLS

#define TNY_ACCOUNT_FACTORY_TYPE             (tny_account_factory_get_type ())
#define TNY_ACCOUNT_FACTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_ACCOUNT_FACTORY_TYPE, TnyAccountFactory))
#define TNY_ACCOUNT_FACTORY_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_ACCOUNT_FACTORY_TYPE, TnyAccountFactoryClass))
#define TNY_IS_ACCOUNT_FACTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_ACCOUNT_FACTORY_TYPE))
#define TNY_IS_ACCOUNT_FACTORY_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_ACCOUNT_FACTORY_TYPE))
#define TNY_ACCOUNT_FACTORY_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_ACCOUNT_FACTORY_TYPE, TnyAccountFactoryClass))

typedef struct _TnyAccountFactory TnyAccountFactory;
typedef struct _TnyAccountFactoryClass TnyAccountFactoryClass;

struct _TnyAccountFactory
{
	GObject parent;
};

struct _TnyAccountFactoryClass
{
	GObjectClass parent;
};

GType               tny_account_factory_get_type       (void);
TnyAccountFactory*  tny_account_factory_new            (void);

GList*              tny_account_factory_get_accounts   (TnyAccountFactory *self);
void                tny_account_factory_add_account    (TnyAccountFactory *self, TnyAccountIface *account);

G_END_DECLS

#endif
