/*
   Bit operations.

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
 * Bit operations.
 *
 * This module implements some of the functions in Lua 5.2's
 * [bit32 module](http://www.lua.org/manual/5.2/manual.html#6.7). Since that
 * module isn't shipped with Lua 5.1 and 5.3, the following module is a
 * portable replacement.
 *
 * Info-short: Only a handful of functions are implemented. More may be added
 * if the need arises.
 *
 * @module utils.bit32
 */

#include <config.h>

#include "lib/global.h"
#include "lib/lua/capi.h"

#include "../modules.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

static int l_bor (lua_State * L);
static int l_band (lua_State * L);

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct luaL_Reg utils_bit32_lib[] = {
    { "bor", l_bor },
    { "band", l_band },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Bitwise or.
 *
 * see Lua's [bit32.bor](http://www.lua.org/manual/5.2/manual.html#pdf-bit32.bor)
 *
 * @function bor
 * @args (...)
 */
static int
l_bor (lua_State * L)
{
    int i = lua_gettop (L);
    guint32 acc = 0;
    while (i > 0)
    {
        acc |= (guint32) luaL_checkunsigned (L, i);
        --i;
    }
    lua_pushunsigned (L, acc);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Bitwise and.
 *
 * see Lua's [bit32.band](http://www.lua.org/manual/5.2/manual.html#pdf-bit32.band)
 *
 * @function band
 * @args (...)
 */
static int
l_band (lua_State * L)
{
    int i = lua_gettop (L);
    /* Note: Calling band() with no arguments returns 0xffffffff for
     * compatibility with Lua 5.2's standard library. */
    guint32 acc = ~(guint32) 0;
    while (i > 0)
    {
        acc &= (guint32) luaL_checkunsigned (L, i);
        --i;
    }
    lua_pushunsigned (L, acc);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_utils_bit32 (lua_State * L)
{
    luaL_newlib (L, utils_bit32_lib);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */
