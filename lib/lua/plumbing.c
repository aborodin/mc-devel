/*
   Functions that plug Lua and MC together.

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

/*
 * This file contains only high-level functions. None of them accepts a
 * lua_State argument (if it does, it means it's low level and belongs in
 * capi[-safecall].c, not here).
 */

#include <config.h>

#include <stdio.h>

#include "lib/global.h"
#include "lib/mcconfig.h"       /* mc_config_get_data_path() */
#include "lib/event.h"
#include "lib/widget.h"         /* message(), Widget */

#include "capi.h"
#include "capi-safecall.h"

#ifdef HAVE_LUAJIT
#include <luajit.h>             /* LUAJIT_VERSION */
#endif

#include "plumbing.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/* The path of the bootstrap file, relative to mc_lua_system_dir(). */
#define BOOTSTRAP_FILE "modules/core/_bootstrap.lua"

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

static gboolean ui_is_ready = FALSE;
static gboolean lua_core_found = FALSE;

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* "ui is ready" event handler */
static gboolean
ui_is_ready_handler (const gchar * event_group_name, const gchar * event_name,
                     gpointer init_data, gpointer data)
{
    (void) event_group_name;
    (void) event_name;
    (void) init_data;
    (void) data;

    ui_is_ready = TRUE;

    mc_lua_replay_first_error ();

    if (!lua_core_found)
    {
        message (D_ERROR, _("Lua error"),
                 _("I can't find the Lua core scripts. Most probably you haven't\n"
                   "installed MC correctly.\n"
                   "\n"
                   "Did you remember to do \"make install\"? This will create the folder\n"
                   "%s and populate it with some scripts.\n"
                   "\n"
                   "Alternatively, if you don't wish to install MC, you may point me\n"
                   "to the Lua folder by the %s environment variable."),
                 mc_lua_system_dir (), MC_LUA_SYSTEM_DIR__ENVAR);
    }

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

static void
create_argv (lua_State * L, const char *script_path, int argc, char **argv, int offs)
{
    g_assert (offs <= argc);

    /* Push argv onto the stack, as a table. */
    luaMC_push_argv (L, argv + offs, TRUE);

    /* Add to that table, at index #0, the script name. */
    lua_pushstring (L, script_path);
    lua_rawseti (L, -2, 0);

    lua_setglobal (L, "argv");
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* -------------------------- Meta information ---------------------------- */

const char *
mc_lua_engine_name (void)
{
#ifdef HAVE_LUAJIT
    return LUAJIT_VERSION;
#else
    return LUA_RELEASE;
#endif
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Where system scripts are stored.
 */
const char *
mc_lua_system_dir (void)
{
    static const char *dir = NULL;

    if (dir == NULL)
    {
        /* getenv()'s returned pointer may be overwritten (by next getenv) or
         * invalidated (by putenv), so we make a copy with strdup(). */
        dir = g_strdup (g_getenv (MC_LUA_SYSTEM_DIR__ENVAR));
        if (dir == NULL)
            dir = MC_LUA_SYSTEM_DIR;    /* Defined in Makefile.am. It already has MC_LUA_API_VERSION embedded. */
    }

    return dir;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Where user scripts are stored.
 */
const char *
mc_lua_user_dir (void)
{
    static const char *dir = NULL;

    if (dir == NULL)
    {
        dir = g_strdup (g_getenv (MC_LUA_USER_DIR__ENVAR));
        if (dir == NULL)
            dir = g_build_filename (mc_config_get_data_path (), "lua-" MC_LUA_API_VERSION, NULL);
    }

    return dir;
}

/* --------------------------------------------------------------------------------------------- */

/* ----------------------------- Start/stop ------------------------------- */

/**
 * Initializes the Lua VM.
 */
void
mc_lua_init (void)
{
    Lg = luaL_newstate ();
    luaL_openlibs (Lg);
    /* The following line causes code in the 'src' tree to open our C modules. */
    mc_event_raise (MCEVENT_GROUP_LUA, "init", NULL);
    mc_event_add (MCEVENT_GROUP_CORE, "ui_is_ready", ui_is_ready_handler, NULL, NULL);
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Loads the core, and then the user scripts.
 */
void
mc_lua_load (void)
{
    /* Load core (which in turn loads user scripts). */
    lua_core_found = (luaMC_safe_dofile (Lg, mc_lua_system_dir (), BOOTSTRAP_FILE) != LUA_ERRFILE);

    g_assert (lua_gettop (Lg) == 0);    /* sanity check */
}

/* --------------------------------------------------------------------------------------------- */

void
mc_lua_shutdown (void)
{
    lua_close (Lg);
    Lg = NULL;                  /* For easier debugging, in case somebody tries to use Lua after shutdown. */
}

/* --------------------------------------------------------------------------------------------- */

/* ------------------------------- Runtime -------------------------------- */

gboolean
mc_lua_ui_is_ready (void)
{
    return ui_is_ready;
}

/* --------------------------------------------------------------------------------------------- */

/* --------------------------- mcscript-related --------------------------- */

/*
 * Runs a scripts.
 *
 * This is how 'mcscript' (or 'mc -L') runs a script. (It's just a wrapper
 * around luaMC_safe_dofile().)
 *
 * Returns TRUE if the script finished successfully (i.e.: script file found,
 * no exception raised, no syntax error). Otherwise, prints an error message
 * and returns FALSE.
 */
gboolean
mc_lua_run_script (const char *pathname)
{
    return (luaMC_safe_dofile (Lg, pathname, NULL) == 0);
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Exports argv to the Lua side, for user scripts that want to use it.
 *
 * argv[0] gets the script's name. The arguments then follow.
 */
void
mc_lua_create_argv (const char *script_path, int argc, char **argv, int offs)
{
    create_argv (Lg, script_path, argc, argv, offs);
}

/* --------------------------------------------------------------------------------------------- */