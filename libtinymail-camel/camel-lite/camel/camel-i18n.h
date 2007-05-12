/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Authors: Jeffrey Stedfast <fejj@ximian.com>
 *
 *  Copyright 2002 Ximian, Inc. (www.ximian.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef CAMEL_DISABLE_DEPRECATED

#ifndef __CAMEL_I18N_H__
#define __CAMEL_I18N_H__

#include <glib.h>

#ifdef ENABLE_NLS
#    include <libintl.h>
#    ifdef TRANSDOM
#        undef _
#        define _(String) dgettext (TRANSDOM, String)
#    else 
#        define _(String) gettext (String)
#    endif
#    ifdef gettext_noop
#        define N_(String) gettext_noop (String)
#    else
#        define N_(String) (String)
#    endif
#else
/* Stubs that do something close enough.  */
#    define textdomain(String) (String
#ifndef gettext
#    define gettext(String) (String)
#endif
#ifndef dgettext
#    define dgettext(Domain,Message) (Message)
#endif
#ifndef dcgettext
#    define dcgettext(Domain,Message,Type) (Message)
#endif
#    define bindtextdomain(Domain,Directory) (Domain)
#ifndef _
#    define _(String) (String)
#endif
#    define N_(String) (String)
#endif

#endif /* __CAMEL_I18N_H__ */

#endif /* CAMEL_DISABLE_DEPRECATED */
