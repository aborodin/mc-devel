/*
   Widgets for the Midnight Commander

   Copyright (C) 2021
   Free Software Foundation, Inc.

   Authors:
   Andrew Borodin <aborodin@vmail.ru>, 2021

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

/** \file grepping-listbox.c
 *  \brief Source: WGreppingListbox widget
 */

#include <config.h>

#include "lib/global.h"

#include "lib/tty/tty.h"
#include "lib/strutil.h"
#include "lib/util.h"           /* Q_() */
#include "lib/widget.h"

/*** global variables ****************************************************************************/

const global_keymap_t *grepping_listbox_map = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
grepping_listbox_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WGreppingListbox *l = GREPPING_LISTBOX (w);

    switch (msg)
    {
    default:
        return group_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WGreppingListbox *
grepping_listbox_new (int y, int x, int height, int width, gboolean deletable, const char *histname)
{
    WGreppingListbox *l;
    WGroup *g;
    Widget *w;

    if (height < 3)
        height = 3;

    l = g_new (WGreppingListbox, 1);
    g = GROUP (l);
    w = WIDGET (l);
    group_init (g, y, x, height, width, grepping_listbox_callback, NULL);
    w->options |= WOP_SELECTABLE | WOP_WANT_HOTKEY;
    w->keymap = grepping_listbox_map;

    l->mode = GREPPING_LISTBOX_MODE_DEFAULT;

    /* widgets */
    l->listbox = listbox_new (0, 0, height, width, deletable, NULL);
    group_add_widget_autopos (g, l->listbox, WPOS_KEEP_ALL, NULL);

    l->label = label_new (height - 2, 0, NULL);
    widget_set_state (WIDGET (l->label), WST_VISIBLE, FALSE);
    group_add_widget_autopos (g, l->label, WPOS_KEEP_HORZ | WPOS_KEEP_BOTTOM, NULL);

    l->input = input_new (height - 1, 0, input_colors, width, NULL, histname, INPUT_COMPLETE_NONE);
    widget_set_state (WIDGET (l->input), WST_VISIBLE, FALSE);
    group_add_widget_autopos (g, l->input, WPOS_KEEP_HORZ | WPOS_KEEP_BOTTOM, NULL);

    return l;
}

/* --------------------------------------------------------------------------------------------- */
