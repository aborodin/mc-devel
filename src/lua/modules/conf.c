/*
   Configuration.

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
 * Configuration.
 *
 * Gives information about how MC is configured.
 *
 * @module conf
 */

#include <config.h>

#include "lib/global.h"
#include "lib/mcconfig.h"       /* mc_config_get_*() */
#include "lib/lua/capi.h"
#include "lib/lua/plumbing.h"

#include "../modules.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct luaL_Reg conf_lib[] =
{
    /* No functions are currently defined in this module. */
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Directories.
 *
 * This table contains paths of directories where MC-related configuration
 * and data are stored.
 *
 * @field user_config Location of user configuration.
 * @field user_data Location of user data.
 * @field user_cache Location of user cache.
 * @field user_lua Location of user Lua scripts.
 * @field system_config Location of system configuration.
 * @field system_data Location of system data.
 * @field system_lua Location of system Lua scripts.
 *
 * @table dirs
 */
static void
build_dirs_table (lua_State * L)
{
    lua_newtable (L);

    lua_pushstring (L, mc_config_get_path ());
    lua_setfield (L, -2, "user_config");

    lua_pushstring (L, mc_config_get_data_path ());
    lua_setfield (L, -2, "user_data");

    lua_pushstring (L, mc_config_get_cache_path ());
    lua_setfield (L, -2, "user_cache");

    lua_pushstring (L, mc_lua_user_dir ());
    lua_setfield (L, -2, "user_lua");

    lua_pushstring (L, mc_global.sysconfig_dir);
    lua_setfield (L, -2, "system_config");

    lua_pushstring (L, mc_global.share_data_dir);
    lua_setfield (L, -2, "system_data");

    lua_pushstring (L, mc_lua_system_dir ());
    lua_setfield (L, -2, "system_lua");
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_conf (lua_State * L)
{
    luaL_newlib (L, conf_lib);

    build_dirs_table (L);
    lua_setfield (L, -2, "dirs");

    return 1;
}

/* --------------------------------------------------------------------------------------------- */