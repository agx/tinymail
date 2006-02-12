#ifndef TNY_CAMEL_SESSION_H
#define TNY_CAMEL_SESSION_H

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
#include <camel/camel-session.h>
#include <tny-shared.h>


G_BEGIN_DECLS

#define TNY_CAMEL_SESSION_TYPE     (tny_camel_session_get_type ())
#define TNY_CAMEL_SESSION(obj)     (CAMEL_CHECK_CAST((obj), TNY_CAMEL_SESSION_TYPE, TnyCamelSession))
#define TNY_CAMEL_SESSION_CLASS(k) (CAMEL_CHECK_CLASS_CAST ((k), TNY_CAMEL_SESSION_TYPE, TnyCamelSessionClass))
#define TNY_IS_CAMEL_SESSION(o)    (CAMEL_CHECK_TYPE((o), TNY_CAMEL_SESSION_TYPE))

typedef struct _TnyCamelSession TnyCamelSession;
typedef struct _TnyCamelSessionClass TnyCamelSessionClass;

struct _TnyCamelSession
{
        CamelSession parent_object;
        gboolean interactive;
};

struct _TnyCamelSessionClass
{
        CamelSessionClass parent_class;

        void        (*set_pass_func_func) (TnyCamelSession *self, TnyAccountIface *account, GetPassFunc get_pass_func);
        void        (*set_forget_pass_func_func) (TnyCamelSession *self, TnyAccountIface *account, ForgetPassFunc forget_pass_func);

};

CamelType         tny_camel_session_get_type      (void);
TnyCamelSession*  tny_camel_session_get_instance  (void);

void              tny_camel_session_set_pass_func (TnyCamelSession *self, TnyAccountIface *account, GetPassFunc get_pass_func);
void              tny_camel_session_set_forget_pass_func (TnyCamelSession *self, TnyAccountIface *account, ForgetPassFunc get_forget_pass_func);

G_END_DECLS

#endif
