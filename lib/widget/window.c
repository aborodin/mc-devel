/*
   Widgets for the Midnight Commander

   Copyright (C) 2020
   The Free Software Foundation, Inc.

   Authors:
   Andrew Borodin <aborodin@vmail.ru>, 2020

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

/** \file window.c
 *  \brief Source: WWindow widget
 */

#include <config.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/global.h"
#include "lib/skin.h"
#include "lib/fileloc.h"        /* MC_HISTORY_FILE */
#include "lib/mcconfig.h"
#include "lib/event.h"          /* mc_event_raise() */
#include "lib/event-types.h"

#include "lib/widget.h"

/*** global variables ****************************************************************************/

/* Color styles */
window_colors_t window_colors;
window_colors_t listbox_colors;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

<<<<<<< HEAD
WWindow *
window_new (int y, int x, int lines, int cols, widget_pos_flags_t pos_flags, gboolean compact,
            const int *colors, widget_cb_fn callback, widget_mouse_cb_fn mouse_callback,
            const char *help_ctx, const char *title)
{
}

/* --------------------------------------------------------------------------------------------- */

void
window_init (WWindow * w, int y, int x, int lines, int cols, widget_pos_flags_t pos_flags,
             gboolean compact, const int *colors, widget_cb_fn callback,
             widget_mouse_cb_fn mouse_callback, const char *help_ctx, const char *title)
{
}

/* --------------------------------------------------------------------------------------------- */

void
window_destroy (WWindow * w)
{
}

/* --------------------------------------------------------------------------------------------- */

void
window_set_default_colors (void)
{
    window_colors[WINDOW_COLOR_NORMAL] = COLOR_NORMAL;
    window_colors[WINDOW_COLOR_FOCUS] = COLOR_FOCUS;
    window_colors[WINDOW_COLOR_HOT_NORMAL] = COLOR_HOT_NORMAL;
    window_colors[WINDOW_COLOR_HOT_FOCUS] = COLOR_HOT_FOCUS;
    window_colors[WINDOW_COLOR_TITLE] = COLOR_TITLE;

    listbox_colors[WINDOW_COLOR_NORMAL] = PMENU_ENTRY_COLOR;
    listbox_colors[WINDOW_COLOR_FOCUS] = PMENU_SELECTED_COLOR;
    listbox_colors[WINDOW_COLOR_HOT_NORMAL] = PMENU_ENTRY_COLOR;
    listbox_colors[WINDOW_COLOR_HOT_FOCUS] = PMENU_SELECTED_COLOR;
    listbox_colors[WINDOW_COLOR_TITLE] = PMENU_TITLE_COLOR;
}

/* --------------------------------------------------------------------------------------------- */

/**
  * Read histories from the ${XDG_CACHE_HOME}/mc/history file.
  */
void
window_read_history (const WWindow * w)
{
    char *profile;
    ev_history_load_save_t event_data;

    /* Is history save disabled? */
    if (num_history_items_recorded == 0)
        return;

    profile = mc_config_get_full_path (MC_HISTORY_FILE);
    event_data.cfg = mc_config_init (profile, TRUE);
    event_data.receiver = NULL;

    /* create all histories in dialog */
    mc_event_raise (w->event_group, MCEVENT_HISTORY_LOAD, &event_data);

    mc_config_deinit (event_data.cfg);
    g_free (profile);
}

/* --------------------------------------------------------------------------------------------- */

/**
  * Write history to the ${XDG_CACHE_HOME}/mc/history file.
  */
void
window_save_history (const WWindow * w)
{
    char *profile;
    int fd;

    /* Is history save disabled? */
    if (num_history_items_recorded == 0)
        return;

    profile = mc_config_get_full_path (MC_HISTORY_FILE);
    fd = open (profile, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd != -1)
        close (fd);

    /* Make sure the history is only readable by the user */
    if (chmod (profile, S_IRUSR | S_IWUSR) != -1 || errno == ENOENT)
    {
        ev_history_load_save_t event_data;

        event_data.cfg = mc_config_init (profile, FALSE);
        event_data.receiver = NULL;

        /* get all histories in dialog */
        mc_event_raise (w->event_group, MCEVENT_HISTORY_SAVE, &event_data);

        mc_config_save_file (event_data.cfg, NULL);
        mc_config_deinit (event_data.cfg);
    }

    g_free (profile);
}

/* --------------------------------------------------------------------------------------------- */
