/*
   Color setup.
   Interface functions.

   Copyright (C) 1994-2026
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru>, 2009
   Slava Zanko <slavazanko@gmail.com>, 2009
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

/** \file color.c
 *  \brief Source: color setup
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  // size_t

#include "lib/global.h"
#include "lib/util.h"  // MC_PTR_FREE

#include "tty.h"
#include "color.h"

#include "color-internal.h"

/*** global variables ****************************************************************************/

int *tty_color_role_to_pair = NULL;

/* Set if we are actually using colors */
gboolean use_colors = FALSE;

gboolean need_convert_256color = FALSE;

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

#ifdef HAVE_TESTS
extern GHashTable *mc_tty_color__hashtable;
#endif

MC_TESTABLE GHashTable *mc_tty_color__hashtable = NULL;

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static gboolean
tty_color_free_temp_cb (gpointer key, gpointer value, gpointer user_data)
{
    (void) key;
    (void) user_data;

    return ((tty_color_lib_pair_t *) value)->is_temp;
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
tty_color_get_next_cpn_cb (gpointer key, gpointer value, gpointer user_data)
{
    tty_color_lib_pair_t *mc_color_pair = (tty_color_lib_pair_t *) value;
    size_t cp = GPOINTER_TO_SIZE (user_data);

    (void) key;

    return (cp == mc_color_pair->pair_index);
}

/* --------------------------------------------------------------------------------------------- */

static size_t
tty_color_get_next__color_pair_number (void)
{
    size_t cp_count, cp;

    cp_count = g_hash_table_size (mc_tty_color__hashtable);
    for (cp = 0; cp < cp_count; cp++)
        if (g_hash_table_find (mc_tty_color__hashtable, tty_color_get_next_cpn_cb,
                               GSIZE_TO_POINTER (cp))
            == NULL)
            break;

    return cp;
}

/* --------------------------------------------------------------------------------------------- */

static inline int
parse_hex_digit (char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    c |= 0x20;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

/* --------------------------------------------------------------------------------------------- */

static int
parse_256_or_true_color_name (const char *color_name)
{
    int i;
    char dummy;

    if (sscanf (color_name, "color%d%c", &i, &dummy) == 1 && i >= 0 && i < 256)
    {
        return i;
    }
    if (sscanf (color_name, "gray%d%c", &i, &dummy) == 1 && i >= 0 && i < 24)
    {
        return 232 + i;
    }
    if (strncmp (color_name, "rgb", 3) == 0             //
        && color_name[3] >= '0' && color_name[3] < '6'  //
        && color_name[4] >= '0' && color_name[4] < '6'  //
        && color_name[5] >= '0' && color_name[5] < '6'  //
        && color_name[6] == '\0')
    {
        return 16 + 36 * (color_name[3] - '0') + 6 * (color_name[4] - '0') + (color_name[5] - '0');
    }
    if (color_name[0] == '#')
    {
        int len;

        color_name++;
        len = (int) strlen (color_name);
        if (len == 3 || len == 6)
        {
            int h[6];

            for (i = 0; i < len; i++)
            {
                h[i] = parse_hex_digit (color_name[i]);
                if (h[i] == -1)
                    return -1;
            }

            if (i == 3)
                i = (h[0] << 20) | (h[0] << 16) | (h[1] << 12) | (h[1] << 8) | (h[2] << 4) | h[2];
            else
                i = (h[0] << 20) | (h[1] << 16) | (h[2] << 12) | (h[3] << 8) | (h[4] << 4) | h[5];
            return FLAG_TRUECOLOR | i;
        }
    }

    return -1;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
tty_init_colors (gboolean disable, gboolean force, int color_map_size)
{
    tty_color_role_to_pair = g_new (int, color_map_size);
    tty_color_init_lib (disable, force);
    mc_tty_color__hashtable = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
}

/* --------------------------------------------------------------------------------------------- */

void
tty_colors_done (void)
{
    tty_color_deinit_lib ();
    g_hash_table_destroy (mc_tty_color__hashtable);
    MC_PTR_FREE (tty_color_role_to_pair);
}

/* --------------------------------------------------------------------------------------------- */

gboolean
tty_use_colors (void)
{
    return use_colors;
}

/* --------------------------------------------------------------------------------------------- */

int
tty_try_alloc_color_pair (const tty_color_pair_t *color, gboolean is_temp)
{
    gchar *color_pair;
    tty_color_lib_pair_t *mc_color_pair;
    int ifg, ibg, attr;

    ifg = tty_color_get_index_by_name (color->fg);
    ibg = tty_color_get_index_by_name (color->bg);
    attr = tty_attr_get_bits (color->attrs);

    color_pair = g_strdup_printf ("%d.%d.%d", ifg, ibg, attr);
    if (color_pair == NULL)
        return 0;

    mc_color_pair = (tty_color_lib_pair_t *) g_hash_table_lookup (mc_tty_color__hashtable,
                                                                  (gpointer) color_pair);

    if (mc_color_pair != NULL)
    {
        g_free (color_pair);
        return mc_color_pair->pair_index;
    }

    if (need_convert_256color)
    {
        if ((ifg & FLAG_TRUECOLOR) == 0)
            ifg = convert_256color_to_truecolor (ifg);

        if ((ibg & FLAG_TRUECOLOR) == 0)
            ibg = convert_256color_to_truecolor (ibg);
    }

    mc_color_pair = g_try_new0 (tty_color_lib_pair_t, 1);
    if (mc_color_pair == NULL)
    {
        g_free (color_pair);
        return 0;
    }

    mc_color_pair->is_temp = is_temp;
    mc_color_pair->fg = ifg;
    mc_color_pair->bg = ibg;
    mc_color_pair->attr = attr;
    mc_color_pair->pair_index = tty_color_get_next__color_pair_number ();

    tty_color_try_alloc_lib_pair (mc_color_pair);

    g_hash_table_insert (mc_tty_color__hashtable, (gpointer) color_pair, (gpointer) mc_color_pair);

    return mc_color_pair->pair_index;
}

/* --------------------------------------------------------------------------------------------- */

void
tty_color_free_temp (void)
{
    g_hash_table_foreach_remove (mc_tty_color__hashtable, tty_color_free_temp_cb, NULL);
}

/* --------------------------------------------------------------------------------------------- */

void
tty_color_free_all (void)
{
    g_hash_table_remove_all (mc_tty_color__hashtable);
}

/* --------------------------------------------------------------------------------------------- */

int
tty_color_get_index_by_name (const char *color_name)
{
    if (color_name != NULL)
    {
        size_t i;

        for (i = 0; color_table[i].name != NULL; i++)
            if (strcmp (color_name, color_table[i].name) == 0)
                return color_table[i].value;
        return parse_256_or_true_color_name (color_name);
    }
    return -1;
}

/* --------------------------------------------------------------------------------------------- */
