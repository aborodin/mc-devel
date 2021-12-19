/*
   Skins engine.
   Work with scroll bar drawing chars.

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

#include <config.h>
#include <stdlib.h>

#include "internal.h"
#include "lib/tty/tty.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

static int
mc_skin_sb_load (mc_skin_t * mc_skin, const char *name, int def_char)
{
    int ret = def_char;
    char *sb_val;

    sb_val = mc_config_get_string_raw (mc_skin->config, "widget-scrollbar", name, NULL);
    if (sb_val != NULL)
    {
        ret = mc_tty_normalize_skin_char (sb_val);
        g_free (sb_val);
    }

    return ret;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
mc_skin_scrollbar_parse_ini_file (mc_skin_t * mc_skin)
{
    if (mc_global.tty.ugly_line_drawing)
        mc_skin_hardcoded_ugly_scrollbar (mc_skin);

    mc_tty_scrollbar[MC_TTY_SB_UP] = mc_skin_sb_load (mc_skin, "up-char", ACS_UARROW);
    mc_tty_scrollbar[MC_TTY_SB_DOWN] = mc_skin_sb_load (mc_skin, "down-char", ACS_DARROW);
    mc_tty_scrollbar[MC_TTY_SB_THUMB] = mc_skin_sb_load (mc_skin, "thumb-vert-char", ACS_BLOCK);
    mc_tty_scrollbar[MC_TTY_SB_TRACK] = mc_skin_sb_load (mc_skin, "track-vert-char", ACS_BOARD);
}

/* --------------------------------------------------------------------------------------------- */
