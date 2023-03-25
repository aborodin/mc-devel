/*
   A langauge-neutral API for scripting.

   Copyright (C) 2015-2023
   Free Software Foundation, Inc.

   Written by:
   Moffie <mooffie@gmail.com> 2015

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * A langauge-neutral API for scripting.
 *
 * Use this API whenever you can. It insulates you from the scripting engine
 * used (whether it be Lua, Pythion, S-Lang, ...). It also makes your code
 * visually cleaner because you don't need to use #ifdef.
 *
 * For example, instead of:
 *
 *     #ifdef ENABLE_LUA
 *     #include "lib/lua/plumbing.h"
 *     #endif
 *     ...
 *     #ifdef ENABLE_LUA
 *     mc_lua_trigger_event ("some_event_name");
 *     #endif
 *
 * do:
 *
 *     #include "lib/scripting.h"
 *     ...
 *     scripting_trigger_event ("some_event_name");
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"         /* Widget type */
#ifdef ENABLE_LUA
#include "lib/lua/plumbing.h"
#endif

#include "scripting.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
scripting_trigger_event (const char *event_name)
{
#ifdef ENABLE_LUA
    mc_lua_trigger_event (event_name);
#else
    (void) event_name;
#endif
}

/* --------------------------------------------------------------------------------------------- */

void
scripting_trigger_widget_event (const char *event_name, Widget * w)
{
#ifdef ENABLE_LUA
    mc_lua_trigger_event__with_widget (event_name, w);
#else
    (void) event_name;
    (void) w;
#endif
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Inform script engines of dead widgets.
 */
void
scripting_notify_on_widget_destruction (Widget * w)
{
#ifdef ENABLE_LUA
    mc_lua_notify_on_widget_destruction (w);
#else
    (void) w;
#endif
}

/* --------------------------------------------------------------------------------------------- */
