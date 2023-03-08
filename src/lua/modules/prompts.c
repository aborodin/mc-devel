/*
   Common dialog boxes.

   Copyright (C) 2016-2023
   Free Software Foundation, Inc.

   Written by:
   Moffie <mooffie@gmail.com> 2016

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
 * Common dialog boxes.
 *
 * @module prompts
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"         /* message() */
#include "lib/lua/capi.h"
#include "lib/lua/plumbing.h"   /* mc_lua_ui_is_ready() */

#include "../modules.h"
#include "tty.h"                /* luaTTY_assert_ui_is_ready() */

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

static int l_alert (lua_State * L);
static int l_confirm (lua_State * L);

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct luaL_Reg prompts_lib[] = {
    { "confirm", l_confirm },
    { "alert", l_alert },
    { NULL, NULL }
};

static const struct luaL_Reg prompts_global_lib[] = {
    { "alert", l_alert },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Displays a message to the user.
 *
 * Note-short: if you wish to examine a variable's value, use @{devel.view}
 * instead.
 *
 * This function, for the sake of convenience, is also exposed in the global
 * namespace.
 *
 * Tip: This function's name (and that of `confirm`) was taken from the
 *  JavaScript world. `input`'s from Python's.
 *
 * - This function is @{mc.is_background|background}-safe.
 * - You may use this function even when the UI is @{tty.is_ui_ready|not ready}:
 *   the message in this case will be written to stdout.
 *
 * @function alert
 * @args (message[, title])
 */
static int
l_alert (lua_State * L)
{
    const char *text, *title;

    /* We use luaMC_tolstring() here, not lua_tostring(), because the user is
     * like to (improperly) use this function to inspect variables, so we need
     * something that can handle any type: tables, booleans, nils, functions,
     * etc. */
    text = luaMC_tolstring (L, 1, NULL);
    title = luaL_optstring (L, 2, "");

    if (mc_lua_ui_is_ready ())
        message (D_NORMAL, title, "%s", text);
    else
    {
        puts (text);
        puts ("\n");
    }

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Displays a yes/no question to the user.
 *
 * Choosing "yes" returns *true*. Choosing "no", or pressing ESC, returns *false*.
 *
 *    if prompts.confirm(T"Delete this file?") then
 *      mc.rm(file)
 *    end
 *
 * (This function is *not* @{mc.is_background|background}-safe.)
 *
 * @function confirm
 * @args (question[, title])
 */
static int
l_confirm (lua_State * L)
{
    const char *text, *title;

    text = luaL_optstring (L, 1, "");
    title = luaL_optstring (L, 2, "");

    luaTTY_assert_ui_is_ready (L);

    lua_pushboolean (L, !query_dialog (title, text, D_NORMAL, 2, _("&Yes"), _("&No")));

    return 1;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_prompts (lua_State * L)
{
    luaMC_register_globals (L, prompts_global_lib);
    luaL_newlib (L, prompts_lib);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */
