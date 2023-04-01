/*
   The Lua 'luafs.gc' module.

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
 * The Lua 'luafs.gc' module. It is used in the Lua portion of LuaFS to
 * communicate with the GC mechanism of the VFS.
 */

#include <config.h>

#include "lib/global.h"
#include "lib/vfs/gc.h"         /* vfs_rmstamp() etc., debug__vfs_get_stamps() */
#include "lib/lua/capi.h"

#include "src/lua/modules.h"

#include "internal.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

static int l_stamp (lua_State * L);
static int l_rmstamp (lua_State * L);
static int l_stamp_create (lua_State * L);
static int l_get_vfs_stamps (lua_State * L);

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct luaL_Reg luafs_gc_lib[] = {
    { "stamp", l_stamp },
    { "rmstamp", l_rmstamp },
    { "stamp_create", l_stamp_create },
    { "get_vfs_stamps", l_get_vfs_stamps },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static int
l_stamp (lua_State * L)
{
    int id;

    id = luaL_checkint (L, 1);
    vfs_stamp (&vfs_luafs_ops, GINT_TO_POINTER (id));

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_rmstamp (lua_State * L)
{
    int id;

    id = luaL_checkint (L, 1);
    vfs_rmstamp (&vfs_luafs_ops, GINT_TO_POINTER (id));

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_stamp_create (lua_State * L)
{
    int id;

    id = luaL_checkint (L, 1);
    vfs_stamp_create (&vfs_luafs_ops, GINT_TO_POINTER (id));

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Show the VFS' stamps.
 *
 * This function is a **debugging aid only**, for programmers working on MC's
 * VFS implementation. It shows the VFS' "stamps". Background information
 * can be found in a comment in 'lib/vfs/gc.c'.
 *
 * Usage example:
 *
 * Enter a TAR archive in a panel. Exit the archive. `require("luafs.gc").get_vfs_stamps()`
 * would now show a "stamp" for this archive. Wait a minute (or do "Free VFSs now"),
 * and the "stamp" should disappear.
 *
 * @function get_vfs_stamps
 */
static int
l_get_vfs_stamps (lua_State * L)
{
    GSList *stamp;
    gint64 now;
    int i = 1;

    now = g_get_monotonic_time ();

    lua_newtable (L);

    for (stamp = debug__vfs_get_stamps (); stamp != NULL; stamp = g_slist_next (stamp))
    {
        struct vfs_stamping *vst = (struct vfs_stamping *) stamp->data;

        lua_newtable (L);

        lua_pushstring (L, vst->v->name);
        lua_setfield (L, -2, "vfs_class_name");

        lua_pushi (L, (long) vst->id);
        lua_setfield (L, -2, "vfs_id");

        lua_pushi (L, (now - vst->time) / G_USEC_PER_SEC);
        lua_setfield (L, -2, "seconds_ago");

        lua_rawseti (L, -2, i++);
    }

    return 1;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_luafs_gc (lua_State * L)
{
    luaL_newlib (L, luafs_gc_lib);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */
