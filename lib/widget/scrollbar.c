/*
   Widgets for the Midnight Commander.

   Copyright (C) 2021
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru>, 2021.

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

/** \file scrollbar.c
 *  \brief Source: WScrollBar widget
 */

#include <config.h>

#include "lib/global.h"
#include "lib/widget.h"
#include "lib/tty/tty.h"
#include "lib/tty/color.h"      /* tty_setcolor() */
#include "lib/skin.h"

#include "scrollbar.h"

/*** global variables ****************************************************************************/

const global_keymap_t *scrollbar_map = NULL;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

static int sb_size, sb_pos;

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static int
scrollbar_get_size (const WScrollBar * sb)
{
    return MAX (3, CONST_WIDGET (sb)->rect.lines);
}

/* --------------------------------------------------------------------------------------------- */

static int
scrollbar_get_pos (const WScrollBar * sb)
{
    int range;

    range = sb->maximum - sb->minimum;
    if (range == 0)
        return 1;

    return (((sb->value - sb->minimum) * (scrollbar_get_size (sb) - 3) + range / 2) / range + 1);
}

/* --------------------------------------------------------------------------------------------- */

static int
scrollbar_get_scroll_step (const WScrollBar * sb, scrollbar_area_t area)
{
    int step;

    switch (area)
    {
    case SB_AREA_UP:
    case SB_AREA_LEFT:
        step = -sb->step;
        break;

    case SB_AREA_DOWN:
    case SB_AREA_RIGHT:
        step = sb->step;
        break;

    case SB_AREA_PAGE_UP:
    case SB_AREA_PAGE_LEFT:
        step = -sb->page;
        break;

    case SB_AREA_PAGE_DOWN:
    case SB_AREA_PAGE_RIGHT:
        step = sb->page;
        break;

    default:
        step = 0;
        break;
    }

    return step;
}

/* --------------------------------------------------------------------------------------------- */

static scrollbar_area_t
scrollbar_get_mouse_pos (const WScrollBar * sb, const mouse_event_t * event)
{
    gboolean vertical;
    int mouse_pos;
    scrollbar_area_t ret;

    vertical = CONST_WIDGET (sb)->rect.cols == 1;
    mouse_pos = vertical ? event->y : event->x; /* relative to widget */

    if (mouse_pos == sb_pos)
        ret = SB_AREA_THUMB;
    else if (mouse_pos < 1)
        ret = vertical ? SB_AREA_UP : SB_AREA_LEFT;
    else if (mouse_pos < sb_pos)
        ret = vertical ? SB_AREA_PAGE_UP : SB_AREA_PAGE_LEFT;
    else if (mouse_pos < sb_size)
        ret = vertical ? SB_AREA_PAGE_DOWN : SB_AREA_PAGE_RIGHT;
    else
        ret = vertical ? SB_AREA_DOWN : SB_AREA_RIGHT;

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

static void
scrollbar_set_value_int (WScrollBar * sb, int value, long command)
{
    value = CLAMP (value, sb->minimum, sb->maximum);

    if (sb->value != value)
    {
        sb->value = value;

        widget_draw (WIDGET (sb));
        send_message (WIDGET (sb)->owner, sb, MSG_NOTIFY, command, NULL);
    }
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
scrollbar_execute_cmd (WScrollBar * sb, long command)
{
    scrollbar_area_t area = SB_AREA_THUMB;
    cb_ret_t ret = MSG_HANDLED;
    Widget *w = WIDGET (sb);

    switch (command)
    {
    case CK_Top:
        scrollbar_set_value_int (sb, sb->minimum, CK_Top);
        break;
    case CK_Bottom:
        scrollbar_set_value_int (sb, sb->maximum, CK_Bottom);
        break;
    case CK_Up:
        area = SB_AREA_UP;
        break;
    case CK_Down:
        area = SB_AREA_DOWN;
        break;
    case CK_PageUp:
        area = SB_AREA_PAGE_UP;
        break;
    case CK_PageDown:
        area = SB_AREA_PAGE_DOWN;
        break;
    default:
        ret = MSG_NOT_HANDLED;
    }

    if (ret == MSG_HANDLED && area != SB_AREA_THUMB)
        scrollbar_set_value_int (sb, sb->value + scrollbar_get_scroll_step (sb, area), command);

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
scrollbar_key (WScrollBar * sb, int key)
{
    long command;

    command = widget_lookup_key (WIDGET (sb), key);
    if (command == CK_IgnoreKey)
        return MSG_NOT_HANDLED;
    return scrollbar_execute_cmd (sb, command);
}

/* --------------------------------------------------------------------------------------------- */

static void
scrollbar_draw (const WScrollBar * sb, int pos)
{
    const Widget *w = CONST_WIDGET (sb);
    const int *colors;
    gboolean disabled;
    int s;
    int i;

    colors = widget_get_colors (w);
    disabled = widget_get_state (w, WST_DISABLED);
    tty_setcolor (disabled ? DISABLED_COLOR : colors[DLG_COLOR_NORMAL]);

    s = scrollbar_get_size (sb);

    widget_gotoyx (w, 0, 0);
    tty_print_alt_char (ACS_UARROW, TRUE);

    for (i = 1; i < s - 1; i++)
    {
        widget_gotoyx (w, i, 0);
        tty_print_alt_char (i == pos || sb->minimum == sb->maximum ? ACS_BLOCK : ACS_BOARD, TRUE);
    }

    widget_gotoyx (w, s - 1, 0);
    tty_print_alt_char (ACS_DARROW, TRUE);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
scrollbar_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm, void *data)
{
    WScrollBar *sb = SCROLLBAR (w);

    switch (msg)
    {
    case MSG_KEY:
        return scrollbar_key (sb, parm);

    case MSG_ACTION:
        return scrollbar_execute_cmd (sb, parm);

    case MSG_DRAW:
        scrollbar_draw (sb, scrollbar_get_pos (sb));
        return MSG_HANDLED;

    default:
        return widget_default_callback (w, sender, msg, parm, data);
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
scrollbar_mouse_callback (Widget * w, mouse_msg_t msg, mouse_event_t * event)
{
    WScrollBar *sb = SCROLLBAR (w);
    scrollbar_area_t where;

    switch (msg)
    {
    case MSG_MOUSE_DOWN:
    case MSG_MOUSE_CLICK:
        sb_pos = scrollbar_get_pos (sb);
        sb_size = scrollbar_get_size (sb) - 1;  /* FIXME: -1 */
        where = scrollbar_get_mouse_pos (sb, event);

        if (where != SB_AREA_THUMB)
        {
            int command;

            switch (where)
            {
            case SB_AREA_UP:
            case SB_AREA_LEFT:
                command = CK_Up;
                break;
            case SB_AREA_DOWN:
            case SB_AREA_RIGHT:
                command = CK_Down;
                break;
            case SB_AREA_PAGE_UP:
            case SB_AREA_PAGE_LEFT:
                command = CK_PageUp;
                break;
            case SB_AREA_PAGE_DOWN:
            case SB_AREA_PAGE_RIGHT:
                command = CK_PageDown;
                break;
            default:
                command = CK_IgnoreKey;
                break;
            }

            scrollbar_set_value_int (sb, sb->value + scrollbar_get_scroll_step (sb, where),
                                     command);
            event->result.repeat = msg == MSG_MOUSE_DOWN;
        }
        break;

    case MSG_MOUSE_DRAG:
        sb_pos = scrollbar_get_pos (sb);
        sb_size = scrollbar_get_size (sb) - 1;  /* FIXME: -1 */
        where = scrollbar_get_mouse_pos (sb, event);

        if (where == SB_AREA_THUMB)
        {
            int mouse_pos = w->rect.cols == 1 ? event->y : event->x;

            mouse_pos = MAX (mouse_pos, 1);
            mouse_pos = MIN (mouse_pos, sb_size - 1);

            if (mouse_pos != sb_pos)
            {
                scrollbar_draw (sb, mouse_pos);
                sb_pos = mouse_pos;
            }

            if (sb_size > 2)
            {
                int new_value;

                sb_size -= 2;
                new_value =
                    sb->minimum +
                    (((sb_pos - 1) * (sb->maximum - sb->minimum) + sb_size / 2) / sb_size);

                if (new_value > sb->value)
                    scrollbar_execute_cmd (sb, CK_PageDown);
                else if (new_value < sb->value)
                    scrollbar_execute_cmd (sb, CK_PageUp);
            }
        }
        break;

    case MSG_MOUSE_SCROLL_UP:  /* FIXME: unneeded? */
        scrollbar_execute_cmd (sb, CK_Up);
        break;

    case MSG_MOUSE_SCROLL_DOWN:        /* FIXME: unneeded? */
        scrollbar_execute_cmd (sb, CK_Down);
        break;

    default:
        break;
    }
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

WScrollBar *
scrollbar_new (int y, int x, int lines)
{
    WScrollBar *sb;
    Widget *w;
    WRect r = { y, x, 0, 1 };

    r.lines = MAX (3, lines);

    sb = g_new (WScrollBar, 1);
    w = WIDGET (sb);

    widget_init (w, &r, scrollbar_callback, scrollbar_mouse_callback);
    w->options |= WOP_SELECTABLE;       /* to handle mouse */
    w->pos_flags = WPOS_KEEP_RIGHT;
    w->keymap = scrollbar_map;

    sb->minimum = 0;
    sb->maximum = scrollbar_get_size (sb);
    sb->value = 0;
    sb->step = 1;
    sb->page = 1;

    return sb;
}

/* --------------------------------------------------------------------------------------------- */

void
scrollbar_set_range (WScrollBar * sb, int minimum, int maximum)
{
    maximum = MAX (maximum, minimum);

    if (sb->minimum != minimum || sb->maximum != maximum)
    {
        sb->minimum = minimum;
        sb->maximum = maximum;

        widget_draw (WIDGET (sb));
    }
}

/* --------------------------------------------------------------------------------------------- */

void
scrollbar_set_value (WScrollBar * sb, int value)
{
    scrollbar_set_value_int (sb, value, CK_IgnoreKey);
}

/* --------------------------------------------------------------------------------------------- */
