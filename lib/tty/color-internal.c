/*
   Internal stuff of color setup

   Copyright (C) 1994-2026
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru>, 2009
   Slava Zanko <slavazanko@gmail.com>, 2009, 2013
   Egmont Koblinger <egmont@gmail.com>, 2010

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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/** \file color-internal.c
 *  \brief Source: Internal stuff of color setup
 */

#include <config.h>

#include <string.h>  // strcmp

#include "color.h"  // colors and attributes
#include "color-internal.h"

/*** global variables ****************************************************************************/

#define COLOR_INTENSITY 8

gboolean mc_tty_color_disable;

mc_tty_color_table_t const color_table[] = {
    { "black", COLOR_BLACK },
    { "gray", COLOR_BLACK + COLOR_INTENSITY },
    { "red", COLOR_RED },
    { "brightred", COLOR_RED + COLOR_INTENSITY },
    { "green", COLOR_GREEN },
    { "brightgreen", COLOR_GREEN + COLOR_INTENSITY },
    { "brown", COLOR_YELLOW },
    { "yellow", COLOR_YELLOW + COLOR_INTENSITY },
    { "blue", COLOR_BLUE },
    { "brightblue", COLOR_BLUE + COLOR_INTENSITY },
    { "magenta", COLOR_MAGENTA },
    { "brightmagenta", COLOR_MAGENTA + COLOR_INTENSITY },
    { "cyan", COLOR_CYAN },
    { "brightcyan", COLOR_CYAN + COLOR_INTENSITY },
    { "lightgray", COLOR_WHITE },
    { "white", COLOR_WHITE + COLOR_INTENSITY },
    { "default", -1 },  // default color of the terminal
    // End of list
    { NULL, 0 },
};

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

static mc_tty_color_table_t const attributes_table[] = {
    { "bold", A_BOLD },
#ifdef A_ITALIC  // available since ncurses-5.9-20130831 / slang-pre2.3.0-107
    { "italic", A_ITALIC },
#endif
    { "underline", A_UNDERLINE },
    { "reverse", A_REVERSE },
    { "blink", A_BLINK },
    // End of list
    { NULL, 0 },
};

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
tty_attr_get_bits (const char *attrs)
{
    int attr_bits = 0;

    if (attrs != NULL)
    {
        gchar **attr_list;
        int i;

        attr_list = g_strsplit (attrs, "+", -1);

        for (i = 0; attr_list[i] != NULL; i++)
        {
            int j;

            for (j = 0; attributes_table[j].name != NULL; j++)
            {
                if (strcmp (attr_list[i], attributes_table[j].name) == 0)
                {
                    attr_bits |= attributes_table[j].value;
                    break;
                }
            }
        }
        g_strfreev (attr_list);
    }
    return attr_bits;
}

/* --------------------------------------------------------------------------------------------- */

int
convert_256color_to_truecolor (int color)
{
    int r, g, b;

    // Invalid color
    if (color > 255)
        return 0;

    if (color >= 232)  // Gray scale
        r = g = b = (color - 231) * 10 + 8;
    else if (color >= 16)  // 6x6x6 color cube
    {
        color -= 16;

        r = (color / (6 * 6) % 6);
        r = r > 0 ? r * 40 + 55 : 0;

        g = (color / 6 % 6);
        g = g > 0 ? g * 40 + 55 : 0;

        b = (color % 6);
        b = b > 0 ? b * 40 + 55 : 0;
    }
    else  // We don't convert basic 16 colors as they are terminal-dependent and user-configurable
        return color;

    color = FLAG_TRUECOLOR | (r << 16) | (g << 8) | b;

    return color;
}

/* --------------------------------------------------------------------------------------------- */

int
tty_maybe_map_color (int color)
{
    if (color >= TTY_COLOR_MAP_OFFSET)
        return tty_color_role_to_pair[color - TTY_COLOR_MAP_OFFSET];
    return color;
}

/* --------------------------------------------------------------------------------------------- */
