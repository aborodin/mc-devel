/*
   Global structure for some library-related variables

   Copyright (C) 2009, 2011
   The Free Software Foundation, Inc.

   Written by:
   Slava Zanko <slavazanko@gmail.com>, 2009.

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

/** \file glibcompat.c
 *  \brief Source: global structure for some library-related variables
 *
 */

#include <config.h>

#include "global.h"

#define SUBSHELL_USE FALSE

/*** global variables ****************************************************************************/

/* *INDENT-OFF* */
mc_global_t mc_global = {
    .message_visible = 1,
    .keybar_visible = 1,
    .mc_run_mode = MC_RUN_FULL,

#ifdef HAVE_CHARSET
    .source_codepage = -1,
    .display_codepage = -1,
#else
    .eight_bit_clean = 1,
    .full_eight_bits = 0,
#endif /* !HAVE_CHARSET */

    .utf8_display = 0,
    .sysconfig_dir = NULL,
    .share_data_dir = NULL,

    .is_right = FALSE,

    .skin = NULL,

    .widget =
    {
        .midnight_shutdown = FALSE,
        .confirm_history_cleanup = TRUE,
        .show_all_if_ambiguous = FALSE
    },

    .tty =
    {
        .setup_color_string = NULL,
        .term_color_string = NULL,
        .color_terminal_string = NULL,
#ifndef LINUX_CONS_SAVER_C
        .console_flag = '\0',
#endif /* !LINUX_CONS_SAVER_C */

        .use_subshell = SUBSHELL_USE,

        .winch_flag = FALSE,
        .command_line_colors = NULL,
        .xterm_flag = FALSE,
        .disable_x11 = FALSE,
        .slow_terminal = FALSE,
        .disable_colors = FALSE,
        .ugly_line_drawing = FALSE,
        .old_mouse = FALSE,
        .alternate_plus_minus = FALSE,
    },

    .vfs =
    {
        .cd_symlinks = TRUE,
        .preallocate_space = FALSE,
    }

};
/* *INDENT-ON* */

#undef SUBSHELL_USE

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */
