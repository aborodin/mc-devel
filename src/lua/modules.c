/*
   All C modules have to be registered here.

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

#include <config.h>

#include "lib/global.h"
#include "lib/event.h"          /* mc_event_add() */
#include "lib/lua/capi.h"
#include "lib/lua/plumbing.h"   /* MCEVENT_GROUP_LUA */

#include "modules.h"
#include "pre-init.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* As to why we expose some modules as "c.NAME", see "Writing hybrid
 * modules" in doc/HACKING. */
/* *INDENT-OFF* */
static const struct luaL_Reg mods[] =
{
    { "conf",         luaopen_conf },
    { "c.fs",         luaopen_fs },
    { "fs.dir",       luaopen_fs_dir },
    { "fs.filedes",   luaopen_fs_filedes },
    { NULL,           luaopen_fs_statbuf },
    { NULL,           luaopen_fs_vpath },
    { "internal",     luaopen_internal },
    { "locale",       luaopen_locale },
    { "mc",           luaopen_mc },
    { "prompts",      luaopen_prompts },
    { "tty",          luaopen_tty },
    { "utils.bit32",  luaopen_utils_bit32 },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * "Loads" all our C modules.
 */
static gboolean
mc_lua_open_c_modules (void)
{
    const luaL_Reg *mod;

    for (mod = mods; mod->func != NULL; mod++)
        if (mod->name != NULL)
            luaMC_requiref (Lg, mod->name, mod->func);
        else
        {
            lua_pushcfunction (Lg, mod->func);
            lua_call (Lg, 0, 0);
        }

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Our Lua integration is split into the 'lib' and the 'src' trees. The
 * modules are here, in the 'src' tree.
 *
 * This poses a problem: since code in 'lib' is not supposed to call code in
 * 'src', there's no way for Lua's initialization routine (in 'lib') to load
 * our modules! The solution: main() calls the following function, which
 * registers itself to trigger when Lua initializes.
 *
 * It wouldn't have worked for main() to call mc_lua_open_c_modules() directly
 * because it (mc_lua_open_c_modules) has to be called also when Lua restarts,
 * a mechanism which main() knows nothing about.
 */
void
mc_lua_pre_init (void)
{
    mc_event_add (MCEVENT_GROUP_LUA, "init", (mc_event_callback_func_t) mc_lua_open_c_modules, NULL,
                  NULL);
    /* Note: It's OK that mc_lua_open_c_modules() doesn't declare in its
     * signature all the parameters mc_event_raise() sends it: we have this
     * scenario when we pass g_free (and others) to g_list_foreach(). */
}

/* --------------------------------------------------------------------------------------------- */
