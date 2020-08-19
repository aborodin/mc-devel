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

/** \file screen.c
 *  \brief Source: WScreen widget (a base class mceditor, mcviever, mcdiffviewer)
 */

#include <config.h>

#include <stdlib.h>

#include "lib/global.h"
#include "lib/widget.h"

/*** global variables ****************************************************************************/

#include "lib/event.h"          /* MCEVENT_GROUP_CORE */

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
screen_default_destroy (Widget * w)
{
    g_free (SCREEN (w)->event_group);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

cb_ret_t
screen_default_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    switch (msg)
    {
    case MSG_RESIZE:
        {
            Widget *o = WIDGET (w->owner);
            /* fullscreen actually, but get values from owner */
            WRect *r = { o->y, o->x, o->lines, o->cols };

            return group_default_callback (w, o, MSG_RESIZE, 0, &r);
        }

    default:
        return group_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */

WScreen *
screen_new (widget_cb_fn callback, widget_mouse_cb_fn mouse_callback)
{
    WScreen *s;
    WGroup *g;
    Widget *w;
    int y, x, lines, cols;

    s = g_new (WScreen, 1);
    g = GROUP (s);
    w = WIDGET (s);

    /* initially fullscreen */
    y = 0;
    x = 0;
    lines = LINES;
    cols = COLS;

    w->pos_flags = WPOS_KEEP_ALL;
    w->state |= WST_FOCUSED;

    group_init (g, y, x, lines, cols, callback != NULL ? callback : screen_default_callback,
                mouse_callback);

    s->event_group = g_strdup_printf ("%s_%p", MCEVENT_GROUP_CORE, (void *) s);

    s->destroy = screen_default_destroy;

    return s;
}

/* --------------------------------------------------------------------------------------------- */

char *
screen_get_title (const WScreen * s, size_t len)
{
    char *t;

    if (s == NULL)
        abort ();

    if (s->get_title != NULL)
        t = s->get_title (h, len);
    else
        t = g_strdup ("");

    return t;
}

/* --------------------------------------------------------------------------------------------- */
