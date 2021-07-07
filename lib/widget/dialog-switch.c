/*
   Support of multiply editors and viewers.

   Original idea and code: Oleg "Olegarch" Konovalov <olegarch@linuxinside.com>

   Copyright (C) 2009-2021
   Free Software Foundation, Inc.

   Written by:
   Daniel Borca <dborca@yahoo.com>, 2007
   Andrew Borodin <aborodin@vmail.ru>, 2010, 2013

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

/** \file dialog-switch.c
 *  \brief Source: support of multiply editors and viewers.
 */

#include <config.h>

#include "lib/global.h"
#include "lib/tty/tty.h"        /* LINES, COLS */
#include "lib/tty/color.h"      /* tty_set_normal_attrs() */
#include "lib/widget.h"
#include "lib/event.h"

/*** global variables ****************************************************************************/

WProgram *midnight = NULL;
WScreen *filemanager = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static unsigned char
get_hotkey (int n)
{
    return (n <= 9) ? '0' + n : 'a' + n - 10;
}

/* --------------------------------------------------------------------------------------------- */

static void
dialog_switch_suspend (void *data, void *user_data)
{
    (void) user_data;

    if (data != mc_current->data)
        widget_set_state (WIDGET (data), WST_SUSPENDED, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static void
dialog_switch_goto (GList * dlg)
{
    if (mc_current != dlg)
    {
        WDialog *old = DIALOG (mc_current->data);

        mc_current = dlg;

        if (old == filemanager)
        {
            /* switch from panels to another dialog (editor, viewer, etc) */
            dialog_switch_pending = TRUE;
            dialog_switch_process_pending ();
        }
        else
        {
            /* switch from editor, viewer, etc to another dialog */
            widget_set_state (WIDGET (old), WST_SUSPENDED, TRUE);

            if (DIALOG (dlg->data) != filemanager)
                /* switch to another editor, viewer, etc */
                /* return to panels before run the required dialog */
                dialog_switch_pending = TRUE;
            else
            {
                /* switch to panels */
                widget_set_state (WIDGET (filemanager), WST_ACTIVE, TRUE);
                widget_draw (midnight);
            }
        }
    }
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_add (WDialog * h)
{
    GList *dlg;

    dlg = g_list_find (mc_dialogs, h);

    if (dlg != NULL)
        mc_current = dlg;
    else
    {
        mc_dialogs = g_list_prepend (mc_dialogs, h);
        mc_current = mc_dialogs;
    }

    /* suspend forced all other screens */
    g_list_foreach (mc_dialogs, dialog_switch_suspend, NULL);
}

/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_remove (WDialog * h)
{
    GList *this;

    if (DIALOG (mc_current->data) == h)
        this = mc_current;
    else
        this = g_list_find (mc_dialogs, h);

    mc_dialogs = g_list_delete_link (mc_dialogs, this);

    /* adjust current dialog */
    if (top_dlg != NULL)
        mc_current = g_list_find (mc_dialogs, DIALOG (top_dlg->data));
    else
        mc_current = mc_dialogs;

    /* resume forced the current screen */
    if (mc_current != NULL)
        widget_set_state (WIDGET (mc_current->data), WST_ACTIVE, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

size_t
dialog_switch_num (void)
{
    return g_list_length (mc_dialogs);
}

/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_next (void)
{
    GList *next;

    if (mc_global.midnight_shutdown || mc_current == NULL)
        return;

    next = g_list_next (mc_current);
    if (next == NULL)
        next = mc_dialogs;

    dialog_switch_goto (next);
}

/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_prev (void)
{
    GList *prev;

    if (mc_global.midnight_shutdown || mc_current == NULL)
        return;

    prev = g_list_previous (mc_current);
    if (prev == NULL)
        prev = g_list_last (mc_dialogs);

    dialog_switch_goto (prev);
}

/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_list (void)
{
    const size_t dlg_num = g_list_length (mc_dialogs);
    int lines, cols;
    Listbox *listbox;
    GList *h, *selected;
    int i = 0;

    if (mc_global.midnight_shutdown || mc_current == NULL)
        return;

    lines = MIN ((size_t) (LINES * 2 / 3), dlg_num);
    cols = COLS * 2 / 3;

    listbox = create_listbox_window (lines, cols, _("Screens"), "[Screen selector]");

    for (h = mc_dialogs; h != NULL; h = g_list_next (h))
    {
        WDialog *dlg = DIALOG (h->data);
        char *title;

        if (dlg->get_title != NULL)
            title = dlg->get_title (dlg, WIDGET (listbox->list)->cols - 2);
        else
            title = g_strdup ("");

        listbox_add_item (listbox->list, LISTBOX_APPEND_BEFORE, get_hotkey (i++), title, h, FALSE);

        g_free (title);
    }

    selected = run_listbox_with_data (listbox, mc_current);
    if (selected != NULL)
        dialog_switch_goto (selected);
}

/* --------------------------------------------------------------------------------------------- */

int
dialog_switch_process_pending (void)
{
    int ret = 0;

    while (dialog_switch_pending)
    {
        WDialog *h = DIALOG (mc_current->data);
        Widget *wh = WIDGET (h);

        dialog_switch_pending = FALSE;
        widget_set_state (wh, WST_SUSPENDED, TRUE);
        ret = dlg_run (h);
        if (widget_get_state (wh, WST_CLOSED))
        {
            widget_destroy (wh);

            /* return to panels */
            if (mc_global.mc_run_mode == MC_RUN_FULL)
            {
                mc_current = g_list_find (mc_dialogs, filemanager);
                mc_event_raise (MCEVENT_GROUP_FILEMANAGER, "update_panels", NULL);
            }
        }
    }

    widget_draw (midnight);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

void
dialog_switch_shutdown (void)
{
    while (mc_dialogs != NULL)
    {
        WDialog *dlg = DIALOG (mc_dialogs->data);

        dlg_run (dlg);
        widget_destroy (WIDGET (dlg));
    }
}

/* --------------------------------------------------------------------------------------------- */
