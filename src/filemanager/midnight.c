/*
   Main dialog (file panels) of the Midnight Commander

   Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
   2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011
   The Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1994, 1995, 1996, 1997
   Janne Kukonlehto, 1994, 1995
   Norbert Warmuth, 1997
   Andrew Borodin <aborodin@vmail.ru>, 2009, 2010

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

/** \file main.c
 *  \brief Source: main dialog (file panels) of the Midnight Commander
 */

#include <config.h>

#include <locale.h>
#include <stdlib.h>

#include "lib/global.h"

#include "lib/tty/tty.h"
#include "lib/skin.h"
#include "lib/widget.h"

#include "src/setup.h"          /* variables */

#include "midnight.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

static void
setup_mc (void)
{
#ifdef HAVE_SLANG
#ifdef HAVE_CHARSET
    tty_display_8bit (TRUE);
#else
    tty_display_8bit (mc_global.full_eight_bits != 0);
#endif /* HAVE_CHARSET */

#else /* HAVE_SLANG */

#ifdef HAVE_CHARSET
    tty_display_8bit (TRUE);
#else
    tty_display_8bit (mc_global.eight_bit_clean != 0);
#endif /* HAVE_CHARSET */
#endif /* HAVE_SLANG */

    if ((tty_baudrate () < 9600) || mc_global.tty.slow_terminal)
        verbose = 0;
}

/* --------------------------------------------------------------------------------------------- */

static void
done_mc (void)
{
    save_setup (auto_save_setup);
}

/* --------------------------------------------------------------------------------------------- */

static void
create_panels_and_run_mc (void)
{
    /* Run the Midnight Commander if no file was specified in the command line */
    run_dlg (midnight_dlg);
}

/* --------------------------------------------------------------------------------------------- */

static cb_ret_t
midnight_callback (Dlg_head * h, Widget * sender, dlg_msg_t msg, int parm, void *data)
{
    return default_dlg_callback (h, sender, msg, parm, data);
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/** Run the main dialog that occupies the whole screen */
void
do_nc (void)
{
    dlg_colors_t midnight_colors;

    midnight_colors[DLG_COLOR_NORMAL] = mc_skin_color_get ("dialog", "_default_");
    midnight_colors[DLG_COLOR_FOCUS] = mc_skin_color_get ("dialog", "focus");
    midnight_colors[DLG_COLOR_HOT_NORMAL] = mc_skin_color_get ("dialog", "hotnormal");
    midnight_colors[DLG_COLOR_HOT_FOCUS] = mc_skin_color_get ("dialog", "hotfocus");
    midnight_colors[DLG_COLOR_TITLE] = mc_skin_color_get ("dialog", "title");

    midnight_dlg = create_dlg (FALSE, 0, 0, LINES, COLS, midnight_colors, midnight_callback,
                               "[main]", NULL, DLG_WANT_IDLE);

    setup_mc ();
    create_panels_and_run_mc ();

    mc_global.widget.midnight_shutdown = TRUE;
    dialog_switch_shutdown ();
    done_mc ();
    destroy_dlg (midnight_dlg);
}

/* --------------------------------------------------------------------------------------------- */
