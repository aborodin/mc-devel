/*
   Functions for running Lua code.

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
 * Provides functions for running Lua code.
 *
 * It handles errors occurring in such code.
 */

#include <config.h>

#include <stdio.h>

#include "lib/global.h"
#include "lib/widget.h"         /* message() */

#include "capi.h"
#include "plumbing.h"           /* mc_lua_ui_is_ready() */
#include "utilx.h"              /* E_() */

#include "capi-safecall.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

static const char *first_error = NULL;

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* ------------------------- Displaying errors ---------------------------- */

static void
display_error (lua_State * L)
{
    const char *error_message;

    error_message = lua_tostring (L, -1);
    if (error_message != NULL)
    {
        if (mc_lua_ui_is_ready ())
            message (D_ERROR, _("Lua error"), "%s", error_message);
        else
            fprintf (stderr, E_ ("LUA EXCEPTION: %s\n"), error_message);
    }
}

/* --------------------------------------------------------------------------------------------- */

/* -------------------------- Running Lua code ---------------------------- */

static void
record_first_error (lua_State * L)
{
    if (first_error == NULL)
        first_error = g_strdup (lua_tostring (L, -1));
}

/* --------------------------------------------------------------------------------------------- */

static void
handle_error (lua_State * L)
{
    if (lua_isstring (L, -1) == 0)
    {
        /* We don't know how to display non-string exceptions. */
        lua_pop (L, 1);
        lua_pushstring (L, E_ ("(error object is not a string)"));
    }
    record_first_error (L);
    display_error (L);

    lua_pop (L, 1);             /* the error */
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * "Safely" calls a Lua function. In case of error, an alert is displayed
 * on the screen.
 *
 * There are two ways to call a Lua function: Either in unprotected mode
 * (via lua_call()) or in protected mode (aka "safe", via luaMC_pcall(),
 * which this function conveniently wraps).
 *
 * You'd use unprotected mode (lua_call()) when you're inside a Lua
 * function. That's because the top-level Lua function calling you is
 * _already_ protected. For example, l_gsub() calls a Lua function using
 * lua_call().
 *
 * OTOH, when you are *not* inside a Lua function (but at MC's top-level)
 * you'd use *this* function to call Lua functions. This function catches
 * any exceptions and displays them nicely to the user.
 *
 * WARNING: make sure to pop all the variables returned on the stack when this
 * function returns successfully. You don't want the global stack to fill up.
 */
gboolean
luaMC_safe_call (lua_State * L, int nargs, int nresults)
{
    gboolean success;

    LUAMC_GUARD (L);

    success = luaMC_pcall (L, nargs, nresults);

    if (!success)
        handle_error (L);

    LUAMC_UNGUARD_BY (L, success ? (-1 - nargs + nresults) : (-1 - nargs));

    return success;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Executes a script file.
 *
 * Returns 0 on success, or some other error code; most notables:
 *
 * LUA_ERRFILE   - can't open file.
 * LUA_ERRSYNTAX - syntax error.
 * LUA_ERRRUN    - runtime error.
 */
int
luaMC_safe_dofile (lua_State * L, const char *dirname, const char *basename)
{
    char *filename;
    gboolean error;

    filename = g_build_filename (dirname, basename, NULL);
    error = luaL_loadfile (L, filename);
    g_free (filename);

    if (error)
    {
        handle_error (L);
        return error;
    }

    /* Note: LUA_OK (0) isn't defined in Lua 5.1. So we use "0". */
    return luaMC_safe_call (L, 0, 0) ? 0 : LUA_ERRRUN;

    /* An alternative implementation is to have a Lua C function that does
     * `loadfile(#1)()`, push that function, and call it with luaMC_safe_call().
     * As it turns out, the above solution is a bit shorter. */
}

/* --------------------------------------------------------------------------------------------- */

/* Errors may occur before the UI is available. In such case they're
 * written to STDERR and the user may not notice them. So we "replay" them
 * when we have a nice UI where the user is sure to see them. */
void
mc_lua_replay_first_error (void)
{
    if (first_error != NULL)
    {
        lua_pushstring (Lg, first_error);
        handle_error (Lg);
    }
}

/* --------------------------------------------------------------------------------------------- */