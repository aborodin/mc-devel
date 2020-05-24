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

#include "lib/widget.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

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
