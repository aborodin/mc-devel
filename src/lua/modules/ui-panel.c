/*
   Panel widget.

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
 * A panel widget is the central component in MC's display. It lists files.
 *
 * Note: A pane has four modes: it may display a tree, a quick-view,
 * one file's information, or file listing. When we speak of a "panel" we
 * always refer to the **file listing** mode.)
 *
 * @classmod ui.Panel
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"
#include "lib/lua/capi.h"
#include "lib/lua/ui-impl.h"    /* luaUI_*() */
#include "lib/scripting.h"      /* scripting_trigger_widget_event() */

#include "src/setup.h"          /* panels_options */
#include "src/filemanager/panel.h"
#include "src/filemanager/filemanager.h"        /* view_listing (via layout.h) */
#include "src/filemanager/panelize.h"   /* do_external_panelize() */

#include "../modules.h"
#include "fs.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/* See comment for LUA_TO_BUTTON, in ui.c */
#define LUA_TO_PANEL(L, i) (PANEL (luaUI_check_widget (L, i)))

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

static int l_get_left (lua_State * L);
static int l_get_right (lua_State * L);
static int l_get_current (lua_State * L);
static int l_get_other (lua_State * L);

static int l_panel_get_dir (lua_State * L);
static int l_panel_get_vdir (lua_State * L);
static int l_panel_set_vdir (lua_State * L);
static int l_panel_set_filter (lua_State * L);
static int l_panel_get_filter (lua_State * L);
static int l_panel_set_panelized (lua_State * L);
static int l_panel_get_panelized (lua_State * L);
static int l_panel_panelize_by_command (lua_State * L);
static int l_panel_reload (lua_State * L);
static int l_set_list_format (lua_State * L);
static int l_get_list_format (lua_State * L);
static int l_get_custom_format (lua_State * L);
static int l_set_custom_format (lua_State * L);
static int l_get_custom_mini_status (lua_State * L);
static int l_set_custom_mini_status (lua_State * L);
static int l_get_custom_mini_status_format (lua_State * L);
static int l_set_custom_mini_status_format (lua_State * L);
static int l_get_num_brief_cols (lua_State * L);
static int l_set_num_brief_cols (lua_State * L);
static int l_set_sort_field (lua_State * L);
static int l_get_sort_field (lua_State * L);
static int l_get_sort_reverse (lua_State * L);
static int l_set_sort_reverse (lua_State * L);
static int l_panel_get_current_index (lua_State * L);
static int l_panel_set_current_index (lua_State * L);
static int l_panel_get_file_by_index (lua_State * L);
static int l_panel_mark_file_by_index (lua_State * L);
static int l_panel_get_max_index (lua_State * L);
static int l_panel_remove (lua_State * L);
static int l_panel_get_metrics (lua_State * L);

/*** file scope variables ************************************************************************/

/**
 * The list type.
 *
 * How files are listed. It is one of "full", "brief", "long", or "custom".
 * For "custom", the format is specified with @{custom_format}.
 *
 * Info: "custom" is entitled "User defined" in MC's interface. In our API we
 * use the word "custom", rather than "user", because the latter's meaning
 * isn't clear when embedded in names of other properties.
 *
 * @attr list_format
 * @property rw
 */

/* *INDENT-OFF* */
static const char *const list_format_names[] =
{
    "full",
    "brief",
    "long",
    "custom",
    NULL
};
/* *INDENT-ON* */

/* *INDENT-OFF* */
static const list_format_t list_format_values[] =
{
    list_full,
    list_brief,
    list_long,
    list_user
};
/* *INDENT-ON* */

/* *INDENT-OFF* */
static const struct luaL_Reg ui_panel_static_lib[] =
{
    { "get_left", l_get_left },
    { "get_right", l_get_right },
    { "get_current", l_get_current },
    { "get_other", l_get_other },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* *INDENT-OFF* */
static const struct luaL_Reg ui_panel_lib[] =
{
    { "get_dir", l_panel_get_dir },
    { "set_dir", l_panel_set_vdir },
    { "get_vdir", l_panel_get_vdir },
    { "set_vdir", l_panel_set_vdir },
    { "set_filter", l_panel_set_filter },
    { "get_filter", l_panel_get_filter },
    { "set_panelized", l_panel_set_panelized },
    { "get_panelized", l_panel_get_panelized },
    { "panelize_by_command", l_panel_panelize_by_command },
    { "reload", l_panel_reload },
    { "set_list_format", l_set_list_format },
    { "get_list_format", l_get_list_format },
    { "get_custom_format", l_get_custom_format },
    { "set_custom_format", l_set_custom_format },
    { "get_custom_mini_status", l_get_custom_mini_status },
    { "set_custom_mini_status", l_set_custom_mini_status },
    { "get_custom_mini_status_format", l_get_custom_mini_status_format },
    { "set_custom_mini_status_format", l_set_custom_mini_status_format },
    { "get_num_brief_cols", l_get_num_brief_cols },
    { "set_num_brief_cols", l_set_num_brief_cols },
    { "set_sort_field", l_set_sort_field },
    { "get_sort_field", l_get_sort_field },
    { "get_sort_reverse", l_get_sort_reverse },
    { "set_sort_reverse", l_set_sort_reverse },
    { "_get_current_index", l_panel_get_current_index },
    { "_set_current_index", l_panel_set_current_index },
    { "_get_file_by_index", l_panel_get_file_by_index },
    { "_mark_file_by_index", l_panel_mark_file_by_index },
    { "_get_max_index", l_panel_get_max_index },
    { "_remove", l_panel_remove },
    { "_get_metrics", l_panel_get_metrics },
    { NULL, NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* Our own flavor of filemanager.c:update_dirty_panels(). */
static void
redraw_dirty_panel (WPanel * panel)
{
    if (panel->dirty)
        widget_draw (WIDGET (panel));
}

/* --------------------------------------------------------------------------------------------- */

/**
 * General methods.
 * @section panel-general
 */

/**
 * The panel's directory.
 *
 *    -- Insert the panel's directory name into the command line.
 *    ui.Panel.bind("f16", function(pnl)
 *      local ipt = ui.current_widget("Input")
 *      if ipt then
 *        ipt:insert(pnl.dir)
 *      end
 *    end)
 *
 *    -- This is a better version of the above, which works for
 *    -- any input line.
 *    ui.Input.bind("f16", function(ipt)
 *      -- When using /usr/bin/mcedit there are no panels, hence
 *      -- the "and" check below.
 *      ipt:insert(ui.Panel.current and ui.Panel.current.dir or "")
 *    end)
 *
 *    -- An even better version!
 *    ui.Input.bind("f16", function(ipt)
 *      ipt:insert(fs.current_dir())
 *    end)
 *
 * Note: To change the panel's directory you can do either
 * `pnl.dir = '/whatever'` or `pnl:set_dir('/whatever')` (the former being
 * syntactic sugar for the latter). The latter lets you inspect the return
 * value to see if the operation was successful.
 *
 * See also @{vdir}.
 *
 * @attr dir
 * @property rw
 */
static int
l_panel_get_dir (lua_State * L)
{
    lua_pushstring (L, vfs_path_as_str (LUA_TO_PANEL (L, 1)->cwd_vpath));
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The panel's directory (as a @{~mod:fs.VPath|vpath}).
 *
 * @attr vdir
 * @property rw
 */
static int
l_panel_get_vdir (lua_State * L)
{
    luaFS_push_vpath (L, LUA_TO_PANEL (L, 1)->cwd_vpath);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_panel_set_vdir (lua_State * L)
{
    WPanel *panel;
    const vfs_path_t *new_dir;

    panel = LUA_TO_PANEL (L, 1);
    new_dir = luaFS_check_vpath (L, 2);

    lua_pushboolean (L, panel_do_cd (panel, new_dir, cd_exact));

    /*
     * Changing a panel's dir also calls mc_chdir(). So if we change the "other"
     * panel's dir, the current panel's dir will no longer be the current dir. As
     * a result the user will see error messages about non existing files (e.g.,
     * try pressing ENTER on a directory name).
     *
     * So we refocus the current panel to trigger re-chdir() to the current
     * panel's dir.
     */
    if (current_panel != NULL && current_panel != panel)
    {
        /* We can't just do `widget_select(current_panel)` becuse it's a no-op:
         * widget_select() (via widget_focus()) does nothing if the widget is
         * already focused. */
        widget_set_state (WIDGET (current_panel), WST_FOCUSED, TRUE);
    }

    redraw_dirty_panel (panel);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Reloads the panel.
 *
 * (An operation also known as "rescan" or "reread" in MC.)
 *
 * Currently, because of some deficiency in MC's API, this method reloads
 * _both_ panels.
 *
 * Info: You may alternatively do `pnl.dialog:command "reread"`; it reloads
 * the "current" panel only. But it might be that it's the "other" panel you
 * want reloaded.
 *
 * Info: When the panel is panelized, this method will __not__ cause it to
 * forget its files and instead load the directory's contents. It will only
 * cause it to update the meta data of the files displayed (sizes,
 * modification dates, etc.).
 *
 * @method reload
 */
static int
l_panel_reload (lua_State * L)
{
    WPanel *panel;

    panel = LUA_TO_PANEL (L, 1);

    (void) panel;

    /* This updates both panels. That's the only non low-level public function
     * available to us. */
    update_panels (UP_RELOAD, UP_KEEPSEL);
    /* We don't use panel_reload() directly: it doesn't keep the selected file,
     * and it doesn't send VFS_SETCTL_FLUSH to the VFS. */

    do_refresh ();
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Whether the listing is "panelized".
 *
 * [info]
 *
 * Under the @{git:panel.h|hood}, `panelized` is merely a flag set on a
 * panel that tells MC not to reload the listing. Setting (or clearing) this
 * flag has no other consequence. Specifically: it won't cause a reload (if
 * that's your intention (which only you know) you'll have to call @{reload}
 * yourself).
 *
 * See a comment in @{filter_by_fn} demonstrating the usefulness of this
 * property.
 *
 * [/info]
 *
 * @attr panelized
 * @property rw
 */
static int
l_panel_get_panelized (lua_State * L)
{
    lua_pushboolean (L, LUA_TO_PANEL (L, 1)->is_panelized);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_panel_set_panelized (lua_State * L)
{
    WPanel *panel;
    gboolean enable;

    panel = LUA_TO_PANEL (L, 1);
    enable = lua_toboolean (L, 2);

    panel->is_panelized = enable;

    widget_draw (WIDGET (panel));       /* So that the "Panelize" indicator appears. */

    scripting_trigger_widget_event ("Panel::panelize", WIDGET (panel));

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Populates the panel with the output of a shell command.
 *
 * This is known as "external panelize" in MC's lingo.
 *
 *    -- Make the panel show all MP3 files in the
 *    -- current folder and all subfolders.
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *      pnl:panelize_by_command('find . -iname "*.mp3" -print')
 *    end)
 *
 * Another example:
 *
 *    -- Make the panel show only the files having the
 *    -- same extension as the current file.
 *    -- Files in subfolders too are shown.
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *
 *      local original_current = pnl.current
 *      local extension = pnl.current:match '.*(%..*)' or ''
 *
 *      pnl:panelize_by_command(('find . -name "*"%q -print'):format(extension))
 *
 *      -- Restore the cursor to the file we've been standing on originally:
 *      pnl.current = original_current
 *
 *    end)
 *
 * @method panelize_by_command
 * @args (command)
 */
static int
l_panel_panelize_by_command (lua_State * L)
{
    WPanel *panel;
    const char *command;

    panel = LUA_TO_PANEL (L, 1);
    command = luaL_checkstring (L, 2);

    /* do_external_panelize() works on the current panel only. So
     * we have to select (aka focus) our panel in case it's not the current.
     *
     * @FIXME: Fix do_external_panelize() to work on any panel? But then
     * it'd also have to chdir() to that panel's dir because the command
     * often wants the panel's dir as the current dir (e.g.,
     * "find -iname '*.mp3'" is intended to run in the panel's dir!). */
    widget_select (WIDGET (panel));

    do_external_panelize (command);

    widget_draw (WIDGET (panel));
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The filter.
 *
 * A shell pattern determining the files to show. Set it to **nil** if
 * you want to clear the filter. Example:
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *      pnl.filter = '*.c'
 *    end)
 *
 * [note]
 *
 * MC has two _filter_ bugs:
 *
 * - When the filter is the empty string (`""`), the panel header won't give
 *   an indication that a filter is active.
 * - When the "Shell patterns" option is off, the filter string will still be
 *   interpreted as a shell pattern instead of a regex.
 *
 * Additionally, a filter doesn't affect a panelized panel.
 *
 * [/note]
 *
 * @attr filter
 * @property rw
 */
static int
l_panel_set_filter (lua_State * L)
{
    /* TODO */
    panel_set_filter_to (LUA_TO_PANEL (L, 1), g_strdup (luaL_optstring (L, 2, "*")),
                         luaL_opti (L, 2, FILE_FILTER_DEFAULT_FLAGS));
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_panel_get_filter (lua_State * L)
{
    WPanel *panel;

    panel = LUA_TO_PANEL (L, 1);
    /* TODO  */
    lua_pushstring (L, panel->filter.value);    /* NULL-safe */
    lua_pushinteger (L, panel->filter.flags);
    return 1;
}

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Marking and unmarking files
 *
 * @section
 */

/**

  This empty section comes here to circumvent an ldoc problem: we want
  "Static panel functions" to appear last. So we have to exhaust all
  other sections before arriving at it.

*/

/* --------------------------------------------------------------------------------------------- */

/**
 * The view.
 * @section panel-view
 */

/* Taken from configure_panel_listing(). It's how a panel is updated after
 * some display setting changes. */
static void
update_view (WPanel * panel)
{
    set_panel_formats (panel);
    widget_draw (WIDGET (panel));       /* configure_panel_listing() does do_refresh(), which is an overkill. */
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Whether to show a custom format for the mini status.
 *
 * Boolean.
 *
 * @attr custom_mini_status
 * @property rw
 */
static int
l_get_custom_mini_status (lua_State * L)
{
    lua_pushboolean (L, LUA_TO_PANEL (L, 1)->user_mini_status);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_set_custom_mini_status (lua_State * L)
{
    WPanel *panel;
    gboolean enable;

    panel = LUA_TO_PANEL (L, 1);
    enable = lua_toboolean (L, 2);

    /* Taken from configure_panel_listing(). */
    panel->user_mini_status = enable;

    update_view (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Custom format for the mini status.
 *
 * When @{custom_mini_status} is enabled, this property is the format to use.
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *      pnl.custom_mini_status = true
 *      pnl.custom_mini_status_format = "half type name:20 | gitcommit:10 | gitmessage"
 *    end)
 *
 * Info: MC keeps track of **four** custom mini-status formats: one per each
 * @{list_format}. This @{custom_mini_status_format} property reflects the one
 * belonging to the list_format currently in use.
 *
 * @attr custom_mini_status_format
 * @property rw
 */
static int
l_get_custom_mini_status_format (lua_State * L)
{
    WPanel *panel;

    panel = LUA_TO_PANEL (L, 1);

    lua_pushstring (L, panel->user_status_format[panel->list_format]);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_set_custom_mini_status_format (lua_State * L)
{
    WPanel *panel;
    const char *format;

    panel = LUA_TO_PANEL (L, 1);
    format = luaL_checkstring (L, 2);

    /* Taken from configure_panel_listing(). */
    g_free (panel->user_status_format[panel->list_format]);
    panel->user_status_format[panel->list_format] = g_strdup (format);

    update_view (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The list type.
 */

static int
l_set_list_format (lua_State * L)
{
    WPanel *panel;
    list_format_t list_format;

    panel = LUA_TO_PANEL (L, 1);
    list_format = luaMC_checkoption (L, 2, NULL, list_format_names, list_format_values);

    /* Taken from configure_panel_listing(). */
    panel->list_format = list_format;

    update_view (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_get_list_format (lua_State * L)
{
    luaMC_push_option (L, LUA_TO_PANEL (L, 1)->list_format, "unknown", list_format_names,
                       list_format_values);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Custom format for the listing.
 *
 * When @{list_format} is set to "custom", this property specifies the format
 * to use.
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *      pnl.list_format = "custom"
 *      pnl.custom_format = "half type name | size | perm | gitstatus | gitdate | gitauthor | gitmessage"
 *    end)
 *
 * The syntax of the format string is:
 *
 *    all              := panel_format? format
 *    panel_format     := [full|half] [1-9]
 *    format           := one_format | format , one_format
 *
 *    one_format       := align FIELD_ID [opt_width]
 *    align            := [<=>]
 *    opt_width        := : size [opt_expand]
 *    width            := [0-9]+
 *    opt_expand       := +
 *
 * (This syntax description was copied, with some minor modifications, from
 * a comment in @{git:src/filemanager/panel.c}.)
 *
 * @attr custom_format
 * @property rw
 */
static int
l_set_custom_format (lua_State * L)
{
    WPanel *panel;
    const char *format;

    panel = LUA_TO_PANEL (L, 1);
    format = luaL_checkstring (L, 2);

    /* Taken from configure_panel_listing(). */
    g_free (panel->user_format);
    panel->user_format = g_strdup (format);

    update_view (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_get_custom_format (lua_State * L)
{
    lua_pushstring (L, LUA_TO_PANEL (L, 1)->user_format);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Number of columns for the "brief" @{list_format|list type}.
 *
 * Valid values are 1 to 9 (if you provide a value outside this range it
 * will be clamped to fit the range).
 *
 * Setting this property does not automatically set the
 * @{list_format|list type} to "brief" (You'll have to do this yourself, as
 * shown in the example).
 *
 * Example:
 *
 *    -- Quickly change the number of columns with the +/- keys.
 *
 *    ui.Panel.bind('C-y plus', function(pnl)
 *      pnl.list_format = 'brief'
 *      pnl.num_brief_cols = pnl.num_brief_cols + 1
 *    end)
 *
 *    ui.Panel.bind('C-y minus', function(pnl)
 *      pnl.list_format = 'brief'
 *      pnl.num_brief_cols = pnl.num_brief_cols - 1
 *    end)
 *
 * @attr num_brief_cols
 * @property rw
 */
static int
l_set_num_brief_cols (lua_State * L)
{
    WPanel *panel;
    int cols;

    panel = LUA_TO_PANEL (L, 1);
    cols = luaL_checkint (L, 2);

    panel->brief_cols = CLAMP (cols, 1, 9);     /* GLib macro. */

    if (panel->list_format == list_brief)       /* don't waste CPU cycles otherwise. */
        update_view (panel);

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_get_num_brief_cols (lua_State * L)
{
    lua_pushinteger (L, LUA_TO_PANEL (L, 1)->brief_cols);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The field by which to sort.
 *
 * It is a string identifying a built-in field, like `"name"`, `"size"`,
 * `"mtime"`, `"extension"`, a custom field @{~fields|you yourself created},
 * or `"unsorted"` for the disk's raw order.
 *
 *    -- Toggle between two sorts.
 *    ui.Panel.bind('C-y', function(pnl)
 *      if pnl.sort_field == "name" then
 *        pnl.sort_field = "size"
 *      else
 *        pnl.sort_field = "name"
 *    end)
 *
 * @attr sort_field
 * @property rw
 */
static int
l_get_sort_field (lua_State * L)
{
    lua_pushstring (L, LUA_TO_PANEL (L, 1)->sort_field->id);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_set_sort_field (lua_State * L)
{
    WPanel *panel;
    const char *id;
    const panel_field_t *field;

    panel = LUA_TO_PANEL (L, 1);
    id = luaL_checkstring (L, 2);

    field = panel_get_field_by_id (id);

    if (field == NULL)
        luaL_error (L, _("Unknown field '%s'"), id);
    if (field->sort_routine == NULL)
        luaL_error (L, _("Field '%s' isn't sortable"), id);

    panel_set_sort_order (panel, field);

    redraw_dirty_panel (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Whether to reverse the sort.
 *
 *    ui.Panel.bind('C-y', function(pnl)
 *      pnl.sort_reverse = not pnl.sort_reverse
 *    end)
 *
 * @attr sort_reverse
 * @property rw
 */
static int
l_get_sort_reverse (lua_State * L)
{
    lua_pushboolean (L, LUA_TO_PANEL (L, 1)->sort_info.reverse);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

static int
l_set_sort_reverse (lua_State * L)
{
    WPanel *panel;
    gboolean reverse;

    panel = LUA_TO_PANEL (L, 1);
    reverse = lua_toboolean (L, 2);

    panel->sort_info.reverse = reverse;
    panel_re_sort (panel);

    redraw_dirty_panel (panel);
    return 0;
}

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Low-level methods.
 *
 * These methods aren't intended for use by end-users. These are methods
 * upon which higher-level methods are built.
 *
 * [info]
 *
 * A note to MC developers: these methods are implemented in C. Higher-level
 * methods are implemented in Lua. This lets us experiment easily in
 * designing the public API.
 *
 * [/info]
 *
 * @section panel-lowlevel
 */

/**
 * Gets the index of the current file.
 *
 * @method _get_current_index
 */
static int
l_panel_get_current_index (lua_State * L)
{
    lua_pushinteger (L, LUA_TO_PANEL (L, 1)->current + 1);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Sets the the current ("selected") file, by index.
 *
 * @method _set_current_index
 * @args (i)
 */
static int
l_panel_set_current_index (lua_State * L)
{
    WPanel *panel;
    int i;

    panel = LUA_TO_PANEL (L, 1);
    i = luaL_checkint (L, 2) - 1;

    panel->current = i;
    select_item (panel);

    redraw_dirty_panel (panel);
    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Gets meta information about a file.
 *
 * Multiple values are returned. See @{ui.Panel:current|current} for details.
 *
 * @method _get_file_by_index
 * @param i index
 * @param skip_stat Whether to return a nil instead of the @{fs.StatBuf|StatBuf} (for efficiency).
 */
static int
l_panel_get_file_by_index (lua_State * L)
{
    WPanel *panel;
    int i;
    gboolean skip_stat;

    panel = LUA_TO_PANEL (L, 1);
    i = luaL_checkint (L, 2) - 1;
    skip_stat = lua_toboolean (L, 3);

    if (i < panel->dir.len)
    {
        file_entry_t *fe;

        fe = &panel->dir.list[i];

        lua_pushstring (L, fe->fname->str);
        if (!skip_stat)
            luaFS_push_statbuf (L, &fe->st);
        else
            lua_pushnil (L);
        lua_pushboolean (L, fe->f.marked);
        lua_pushboolean (L, i == panel->current);
        lua_pushboolean (L, fe->f.stale_link);
        lua_pushboolean (L, fe->f.dir_size_computed);
        /* How many values we pushed. Make sure to update this if you change the above. */
        return 6;
    }

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Gets the number of files in the panel.
 *
 * @method _get_max_index
 */
static int
l_panel_get_max_index (lua_State * L)
{
    lua_pushinteger (L, LUA_TO_PANEL (L, 1)->dir.len);
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Changes the mark status of a file.
 *
 * Note: For efficiency, this function doesn't redraw the widget. After
 * you're done marking the files you want, call :redraw() yourself.
 *
 * @method _mark_file_by_index
 * @param i index
 * @param mark Boolean. Whether to mark or unmark the file.
 */
static int
l_panel_mark_file_by_index (lua_State * L)
{
    WPanel *panel;
    int i;
    gboolean mark;

    panel = LUA_TO_PANEL (L, 1);
    i = luaL_checkint (L, 2) - 1;
    mark = lua_toboolean (L, 3);

    do_file_mark (panel, i, mark);

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/*
 * Removes an entry from the panel. Used by l_panel_remove().
 */
static void
panel_remove_entry (WPanel * panel, int i)
{
    dir_list dir = panel->dir;

    if (i < 1 || i >= panel->dir.len)
        return;

    g_string_free (dir.list[i].fname, TRUE);

    memmove (&dir.list[i], &dir.list[i + 1], sizeof dir.list[0] * (panel->dir.len - i - 1));

    /* @todo: we should also update panel->marked and and panel->total. */

    panel->dir.len--;

    if (panel->current > i || panel->current == panel->dir.len)
        panel->current--;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Removes a file.
 *
 * Removes a file from the listing (not from disk, of course), by its index.
 *
 * This can be used to implement filtering.
 *
 * (The panel is not redrawn --for efficiency-- as it's assumed you might
 * want to remove multiple files. You have to call :redraw() yourself.)
 *
 * @method _remove
 * @param i index
 */
static int
l_panel_remove (lua_State * L)
{
    WPanel *panel;
    int i;

    panel = LUA_TO_PANEL (L, 1);
    i = luaL_checkint (L, 2);

    panel_remove_entry (panel, i - 1);

    return 0;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Returns various measurements.
 *
 * Returns several values, in this order:
 *
 * - The index of the top file displayed.
 * - The number of screen lines used for displaying files.
 * - The number of columns.
 *
 * @method _get_metrics
 */

static int
l_panel_get_metrics (lua_State * L)
{
    WPanel *panel;

    panel = LUA_TO_PANEL (L, 1);

    lua_pushinteger (L, panel->top + 1);
    lua_pushinteger (L, panel_lines (panel));
    lua_pushinteger (L, panel->list_cols);

    return 3;
}

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Static panel functions.
 *
 * Any of the properties below may return **nil**. E.g., if the left pane is
 * showing a directory tree, @{left} will return **nil**; when running as
 * "mcedit", @{current} will return **nil**. So don't assume the panels exist.
 *
 * @section panel-static
 */
static int
push_panel (lua_State * L, int panel_idx)
{
    /* *INDENT-OFF* */
    luaUI_push_widget (L, get_panel_type (panel_idx) == view_listing
                          ? get_panel_widget (panel_idx) : NULL,
                       TRUE);
    /* *INDENT-ON* */
    return 1;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The left panel.
 *
 * @attr ui.Panel.left
 * @property r
 */
static int
l_get_left (lua_State * L)
{
    return push_panel (L, 0);
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The right panel.
 *
 * @attr ui.Panel.right
 * @property r
 */
static int
l_get_right (lua_State * L)
{
    return push_panel (L, 1);
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The "current" panel.
 *
 * This is the active panel, the one that has the focus.
 *
 * Example:
 *
 *    -- Insert the panel's dir into the edited text. (tip:
 *    -- replace "Editbox" with "Input" to make it work with
 *    -- any input box.)
 *    ui.Editbox.bind('C-y', function(edt)
 *      -- "<none>" is emitted when using mcedit.
 *      edt:insert(ui.Panel.current and ui.Panel.current.dir or "<none>")
 *    end)
 *
 * [tip]
 *
 * You can make a panel the current one by calling its @{~mod:ui*widget:focus|focus}
 * method. Example:
 *
 *    -- Make the '\' key switch panels. Like the TAB key.
 *    ui.Panel.bind_if_commandline_empty('\\', function()
 *      if ui.Panel.other then
 *        ui.Panel.other:focus()
 *      end
 *    end)
 *
 * [/tip]
 *
 * @attr ui.Panel.current
 * @property r
 */
static int
l_get_current (lua_State * L)
{
    return push_panel (L, get_current_index ());
}

/* --------------------------------------------------------------------------------------------- */

/**
 * The "other" panel.
 *
 * That's the panel which is not the @{current} one.
 *
 *    ui.Panel.bind('f5', function(pnl)
 *      alert(T"You wanna copy something from here to %s":format(
 *        ui.Panel.other and ui.Panel.other.dir or T"<nowhere>"))
 *    end)
 *
 * @attr ui.Panel.other
 * @property r
 */
static int
l_get_other (lua_State * L)
{
    return push_panel (L, get_other_index ());
}

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Events
 *
 * @section
 */

/**
 * Triggered after a panel has been painted.
 *
 * You may use this event to add more information to a panel's display.
 *
 *    -- Prints the directory at the panel's bottom.
 *
 *    ui.Panel.bind('<<draw>>', function(pnl)
 *      local c = pnl:get_canvas()
 *      c:set_style(tty.style('yellow, red'))
 *      c:goto_xy(2, pnl.rows - 1)
 *      c:draw_string(pnl.dir)
 *    end)
 *
 * @moniker draw
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered after a directory has been read into the panel.
 *
 * This happens, for example, when you navigate to a new directory, when you
 * return from running a shell command, or when you reload (`C-r`) the panel.
 *
 * You may use this event to clear caches, as
 * @{~fields!clearing-cache|demonstrated} in the user guide. You may also
 * use it, together with @{activate|<<activate>>}, to inform the
 * environment of the current directory.
 *
 *    ui.Panel.bind("<<load>>", function(pnl)
 *      ....
 *    end)
 *
 * [note]
 *
 * Sometimes code you wish to run in this event may trigger `<<load>>`
 * again. For example, setting @{sort_field} to "unsorted" causes the
 * directory to be re-read. In such cases you'll cause an infinite
 * recursion, which may bring about a program crash.
 *
 * You can solve the problem by wrapping the offending code in @{ui.queue}
 * or wrapping the whole event handler (or just the offending code) in
 * @{utils.magic.once}:
 *
 *    ui.Panel.bind("<<load>>", utils.magic.once(function(pnl)
 *      if pnl.dir:find 'audio' then
 *        pnl.sort_field = 'unsorted'
 *      else
 *        pnl.sort_field = 'name'
 *      end
 *    end))
 *
 * [/note]
 *
 * @moniker load
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered when the user reloads (`C-r`) the panel.
 *
 * You may use this event to clear *expensive* caches, as
 * @{~fields!clearing-cache|demonstrated} in the user guide.
 *
 *    ui.Panel.bind("<<flush>>", function(pnl)
 *      ....
 *    end)
 *
 * Info-short: Filesystems have their own @{luafs.flush|flush event}.
 *
 * @moniker flush
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered when a panel becomes the @{ui.Panel.current|current} one.
 * (E.g., as a result of tabbing to it.)
 *
 *    ui.Panel.bind("<<activate>>", function(pnl)
 *      ....
 *    end)
 *
 * Example: The @{git:set-gterm-cwd.lua} script uses this event, together
 * with @{load|<<load>>}, to inform GNOME Terminal of the current
 * directory.
 *
 * @moniker activate
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered when a file is selected in the panel.
 *
 * Note: __Terminology:__ When we say that a file is "selected" we mean
 * that it has become the @{ui.Panel:current|current} file. Don't confuse
 * the _selected_ file with the @{marked} files: the selected file isn't
 * necessarily marked.
 *
 *    -- Read aloud the current filename, after the user
 *    -- rests on it for a second.
 *
 *    local say = timer.debounce(function(text)
 *      -- Note: we run espeak in the background (&) or else
 *      -- we'll be blocked till it finishes voicing the text.
 *      os.execute(('espeak %q &'):format(text))
 *    end, 1000)
 *
 *    ui.Panel.bind("<<select-file>>", function(pnl)
 *      say(pnl.current)
 *    end)
 *
 * @moniker select-file
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered after a panel has been panelized.
 *
 *    ui.Panel.bind("<<panelize>>", function(pnl)
 *      ....
 *    end)
 *
 * There's no `<<unpanelize>>` event (you should probably use @{load|<<load>>}
 * for that).
 *
 * This event is also triggered after you set the @{panelized} property,
 * to any value.
 *
 * @moniker panelize
 * @event
 */

/* --------------------------------------------------------------------------------------------- */

/**
 * Triggered before the directory is changed.
 *
 * In other words, it is triggered before the user navigates to another
 * directory (e.g., by pressing ENTER on a directory).
 *
 * Compare this with @{load|<<load>>}, which is triggered **after** the
 * directory has changed.
 *
 * Example:
 *
 *    -- This simple code makes the selection (the marked files)
 *    -- persistent.
 *
 *    local selections = {}
 *
 *    ui.Panel.bind("<<before-chdir>>", function(pnl)
 *      selections[pnl.dir] = pnl.marked
 *    end)
 *
 *    ui.Panel.bind("<<load>>", function(pnl)
 *       pnl.marked = selections[pnl.dir]
 *    end)
 *
 * @moniker before-chdir
 * @event
 */

/**
 * @section end
 */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
luaopen_ui_panel (lua_State * L)
{
    create_widget_metatable (L, "Panel", ui_panel_lib, ui_panel_static_lib, "Widget");
    return 0;                   /* Nothing to return! */
}

/* --------------------------------------------------------------------------------------------- */