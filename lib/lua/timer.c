/*
   Functions serve the mechanism that lets us schedule Lua functions to run in the future.

   Copyright (C) 2015-2023
   Free Software Foundation, Inc.

   Written by:
   Moffie <mooffie@gmail.com> 2015

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

/*
 * The functions here serve the mechanism that lets us schedule Lua
 * functions to run in the future.
 *
 * That mechanism is written in Lua: it's our Lua 'timer' module, which
 * exposes, notably, the functions set_timeout() and set_interval().
 *
 * The C side, this file, contains a few devices:
 *
 * It defines a few primitive functions:
 *
 * - mc_lua_set_next_timeout() and mc_lua_timer_now(), which are used by
 *   the Lua side.
 *
 * - mc_lua_execute_ready_timeouts() and mc_lua_has_pending_timeouts(),
 *   which are used by the C side.
 */

#include <config.h>

#include "lib/global.h"

#include "capi.h"
#include "capi-safecall.h"

#include "timer.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/**
 * Holds the timestamp of the next Lua function we're to run. Zero if
 * there's none.
 *
 * Note that this is *all* the data we, the C side, need. We do *not*
 * store here the complete datastructure containing information about
 * *all* the scheduled functions. *That* is what the Lua side does. We
 * only need to know when to invoke the Lua side.
 */
static gint64 next_timeout;

/**
 * This flag is documented at Lua's timer.unlock().
 */
static gboolean lock;

/*** forward declarations (file scope functions) *************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 * The Lua side uses this function (exposed to Lua in the modules/timr.c)
 * to tell us when it's time to invoke it.
 */
void
mc_lua_set_next_timeout (gint64 tm)
{
    next_timeout = tm;
}

/* --------------------------------------------------------------------------------------------- */

void
mc_lua_timer_unlock (void)
{
    lock = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Returns the current timestamp.
 *
 * You can think of it as the equivalent of C's time(), with the following
 * differences:
 *
 * - The resolution is 1 microsecond.
 *
 * - The first call to mc_lua_timer_now() is the 'epoch'. That is, the first
 *   call returns zero.
 *
 * - mc_lua_timer_now() is monotonous: if the admin sets the clock to earlier
 *   time, mc_lua_timer_now() will detect this and at the least return a
 *   reading equal to the previous reading.
 */
gint64
mc_lua_timer_now (void)
{
    static gint64 last_reading = 0;
    static gint64 current_pit;

    gint64 now;

    now = g_get_monotonic_time ();

    if (last_reading == 0)
    {
        /* First call. */
        last_reading = now;
    }
    else if (now < last_reading)
    {
        /* The clock was set backwards in time. Ensure monotony. */
        last_reading = now;
    }

    current_pit += now - last_reading;

    last_reading = now;

    return current_pit;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Tells us whether there are functions scheduled to run, and calculates
 * the time till the first scheduled function.
 *
 * MC's event loop uses this function to know the maximum time it's
 * got to wait for user input.
 */
gboolean
mc_lua_has_pending_timeouts (gint64 * time_out)
{
    if (next_timeout == 0)
        return FALSE;

    if (time_out != NULL)
    {
        /* Calculate time till next timeout. */
        gint64 now;

        now = mc_lua_timer_now ();

        if (next_timeout <= now)
            *time_out = 0;
        else
            *time_out = next_timeout - now;
    }

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */

/**
 * Runs the scheduled functions that are now ready to run.
 *
 * MC's event loop calls this function.
 */
void
mc_lua_execute_ready_timeouts (void)
{
    if (lock)
        return;

    if (next_timeout != 0 && next_timeout <= mc_lua_timer_now ())       /* There's a timeout ready */
    {
        lock = TRUE;

        if (luaMC_get_system_callback (Lg, "timer::execute_ready_timeouts"))
            luaMC_safe_call (Lg, 0, 0);

        /* We might want to call mc_refresh() here so the user doesn't
         * have to call 'tty.refresh()' himself (see explanation in
         * modules/tty.c). But perhaps such call is time consuming. */

        lock = FALSE;
    }
}

/* --------------------------------------------------------------------------------------------- */
