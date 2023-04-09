/*
   A viewer widget.

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
 * A viewer widget.
 *
 * This widget doesn't currently expose many exciting properties and methods.
 * You can use @{ui.bind|ui.Panel.bind} and @{~mod:ui*widget:command|:command},
 * though.
 *
 * @classmod ui.Viewer
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"
#include "lib/lua/capi.h"
#include "lib/lua/ui-impl.h"    /* luaUI_*() */

#include "src/viewer/internal.h"

#include "../modules.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define LUA_TO_VIEWER(L, i) ((WView *) (luaUI_check_widget (L, i)))

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

static int l_view_get_filename (lua_State * L);
static int l_view_get_top_line (lua_State * L);

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct luaL_Reg ui_viewer_static_lib[] = {
    { NULL, NULL }
};

static const struct luaL_Reg ui_viewer_lib[] = {
    { "get_filename", l_view_get_filename },
    { "get_top_line", l_view_get_top_line },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Properties
 * @section
 */

/**
 * The filename associated with the buffer.
 *
 * Returns **nil** if no filename is associated with the buffer (e.g., if the
 * data comes from a pipe).
 *
 * @function filename
 * @property r
 */
static int
l_view_get_filename (lua_State * L)
{
    WView *view;

    view = LUA_TO_VIEWER (L, 1);

    /* Should we return a real VPath? C.f. ui.Editbox.filename. */
    lua_pushstring (L, vfs_path_as_str (view->filename_vpath)); /* pushes nil if NULL */

    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The number of the first line displayed.
 *
 * Example:
 *
 *
 *    -- Launches the editor with F4.
 *
 *    ui.Viewer.bind("f4", function(vwr)
 *      mc.edit(
 *        abortive(vwr.filename, T"You cannot edit a pipe!"),
 *        vwr.top_line
 *      )
 *    end)
 *
 *    -- See a more complete implementation at snippets/viewer_edit.lua.
 *
 * @function top_line
 * @property r
 */
static int
l_view_get_top_line (lua_State * L)
{
    WView *view;

    off_t line, column;

    view = LUA_TO_VIEWER (L, 1);

    mcview_offset_to_coord (view, &line, &column, view->dpy_start);

    lua_pushi (L, line + 1);    /* Our API is 1-based. */

    return 1;
}

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_ui_viewer (lua_State * L)
{
    create_widget_metatable (L, "Viewer", ui_viewer_lib, ui_viewer_static_lib, "Widget");
    return 0;                   /* Nothing to return! */
}

/* --------------------------------------------------------------------------------------------- */
