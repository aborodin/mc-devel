/*
   Widgets for the Midnight Commander

   Copyright (C) 2020-2021
   Free Software Foundation, Inc.

   Authors:
   Andrew Borodin <aborodin@vmail.ru>, 2020-2021

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

/** \file program.c
 *  \brief Source: WProgram widget (top widget for Midnight Commander)
 */

#include <config.h>

#include <stdlib.h>

#include "lib/global.h"
#include "lib/widget.h"

#include "lib/tty/tty.h"
#include "lib/tty/key.h"

#include "lib/event.h"          /* MCEVENT_GROUP_CORE */

/*** global variables ****************************************************************************/

const global_keymap_t *main_map = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * Search first fullscreen item
 */
static GList *
program_find_top_fullscreen (const WProgram * p)
{
    const Widget *pw = CONST_WUDGET (p);
    const WRect *pr = { pw->y, pw->x, pw->lines, pw->cols };
    const WGroup *g = CONST_GROUP (p);
    GList *w;
    GList *ret = NULL;

    /* top dialog or screen */
    w = g->current;
    /* it must be really top */
    g_assert (g_list_next (w) != NULL);

    /* search first fullscreen item */
    for (; w != NULL; w = g_list_previous (w))
    {
        Widget *ww = WIDGET (w->data);
        const WRect *wr = { ww->y, ww->x, ww->lines, ww->cols };

        if (rects_are_equals (pr, wr))
        {
            ret = w;
            break;
        }
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

static
program_screen_list (WProgram * p)
{
    /* TODO */
}

/* --------------------------------------------------------------------------------------------- */

static
program_scren_next (WProgram * p)
{
    /* TODO */
}

/* --------------------------------------------------------------------------------------------- */

static
program_screen_prev (WProgram * p)
{
    /* TODO */
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
program_execute_cmd (WProgram * p, long command)
{
    WGroup *g = GROUP (p);
    cb_ret_t ret = MSG_HANDLED;

    if (send_message (p, NULL, MSG_ACTION, command, NULL) == MSG_HANDLED)
        return MSG_HANDLED;

    switch (command)
    {
    case CK_Refresh:
        program_do_refresh (p);
        break;

    case CK_ScreenList:
        program_screen_list (p);
        break;
    case CK_ScreenNext:
        program_scren_next (p);
        break;
    case CK_ScreenPrev:
        program_screen_prev (p);
        break;

    default:
        ret = MSG_NOT_HANDLED;
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
program_handle_key (WProgram * p, int d_key)
{
    long command;

    command = widget_lookup_key (WIDGET (p), d_key);
    if (command != CK_IgnoreKey)
        return program_execute_cmd (p, command);

    return MSG_NOT_HANDLED;
}

/* --------------------------------------------------------------------------------------------- */

static void
program_key_event (WProgram * p, int key)
{
    Widget *w = WIDGET (p);
    WGroup *g = GROUP (p);
    cb_ret_t handled;

    if (g->widgets == NULL)
        return;

    if (g->current == NULL)
        g->current = g->widgets;

    /* first can program handle the key itself */
    handled = send_message (p, NULL, MSG_KEY, key, NULL);

    if (handled == MSG_NOT_HANDLED)
        handled = group_default_callback (w, NULL, MSG_KEY, key, NULL);

    if (handled == MSG_NOT_HANDLED)
        handled = program_handle_key (p, key);

    (void) handled;
}

/* --------------------------------------------------------------------------------------------- */

static void
program_process_event (WProgram * p, int key, Gpm_Event * event)
{
    switch (key)
    {
    case EV_NONE:
        break;

    case EV_MOUSE:
        {
            Widget *w = WIDGET (h);

            GROUP (p)->mouse_status = w->mouse_handler (w, event);
            break;
        }

    default:
        program_key_event (p, key);
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
program_draw (WProgram * p)
{
    GList *w;

    w = program_find_top_fullscreen (p);

    /* when small dialog (i.e. error message) is created first,
       there is no screen in the program */
    if (w == NULL)
    {
        tty_clear_screen ();
        w = p->current;
    }

    /* back to top dialog */
    for (; w != NULL; w = g_list_next (w))
        widget_draw (WIDGET (w->data));
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
program_default_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WProgram *p = PROGRAM (w);

    switch (msg)
    {
    case MSG_DRAW:
        program_draw (p);
        tty_refresh ();
        return MSG_HANDLED;

    case MSG_RESIZE:
        {
            /* fullscreen */
            WRect r = { 0, 0, LINES, COLS };

            tty_flush_winch ();
            tty_clear_screen ();
            tty_change_screen_size ();

#ifdef HAVE_SLANG
            tty_keypad (TRUE);
            tty_nodelay (FALSE);
#endif

            /* TODO: set winch_pending */

            return group_default_callback (w, sender, MSG_RESIZE, 0, &r);
        }

    default:
        return group_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WProgram *
program_new (widget_cb_fn callback, widget_mouse_cb_fn mouse_callback)
{
    WProgram *p;
    WGroup *g;
    Widget *w;
    int y, x, lines, cols;

    p = g_new (WProgram, 1);
    g = GROUP (p);
    w = WIDGET (p);

    /* fullscreen */
    y = 0;
    x = 0;
    lines = LINES;
    cols = COLS;
    /* own itself */
    w->owner = g;
    w->state |= WST_FOCUSED;

    group_init (g, y, x, lines, cols, callback != NULL ? callback : program_default_callback,
                mouse_callback);

    w->keymap = main_map;
    p->event_group = g_strdup_printf ("%s_%p", MCEVENT_GROUP_CORE, (void *) p);

    return p;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Main event loop
 */
/* TODO */
void
program_run (WProgram * p)
{
    Widget *w = WIDGET (p);
    Gpm_Event event;

    event.x = -1;

    while (widget_get_state (w, WST_ACTIVE))
    {
        int key;

        if (tty_got_winch ())
            send_message (w, w, MSG_RESIZE, 0, NULL);

        if (is_idle ())
        {
            if (idle_hook != NULL)
                execute_hooks (idle_hook);

            while (widget_get_state (w, WST_IDLE) && is_idle ())
                send_message (w, NULL, MSG_IDLE, 0, NULL);

            /* Allow terminating the dialog from the idle handler */
            if (!widget_get_state (w, WST_ACTIVE))
                break;
        }

        widget_update_cursor (w);

        /* Clear interrupt flag */
        tty_got_interrupt ();
        key = tty_get_event (&event, GROUP (p)->mouse_status == MOU_REPEAT, TRUE);

        program_process_event (p, key, &event);

        if (widget_get_state (w, WST_CLOSED))
            send_message (p, NULL, MSG_VALIDATE, 0, NULL);
    }
}

/* --------------------------------------------------------------------------------------------- */

void
program_refresh (WProgram * p)
{
#ifdef ENABLE_BACKGROUND
    if (mc_global.we_are_background)
        return;
#endif /* ENABLE_BACKGROUND */

    if (tty_got_winch ())
    {
        send_message (p, p, MSG_RESIZE, 0, NULL);
        widget_draw (WIDGET (p);
    }
    else
        tty_refresh ();
}

/* --------------------------------------------------------------------------------------------- */
