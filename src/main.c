/*
   Main program for the Midnight Commander

   Copyright (C) 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
   2003, 2004, 2005, 2006, 2007, 2009, 2011
   The Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1994, 1995, 1996, 1997
   Janne Kukonlehto, 1994, 1995
   Norbert Warmuth, 1997

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
 *  \brief Source: this is a main module
 */

#include <config.h>

#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pwd.h>                /* for username in xterm title */
#include <signal.h>

#include "lib/global.h"

#include "lib/event.h"
#include "lib/tty/tty.h"
#include "lib/tty/key.h"        /* For init_key() */
#include "lib/skin.h"
#include "lib/filehighlight.h"
#include "lib/fileloc.h"
#include "lib/strutil.h"
#include "lib/util.h"
#include "lib/vfs/vfs.h"        /* vfs_init(), vfs_shut() */
#include "lib/widget.h"

#include "events_init.h"
#include "args.h"
#include "setup.h"              /* load_setup() */

#ifdef HAVE_CHARSET
#include "lib/charsets.h"
#include "selcodepage.h"
#endif /* HAVE_CHARSET */

#include "src/filemanager/midnight.h"   /* do_nc () */

#include "main.h"

/*** global variables ****************************************************************************/

/* Set when main loop should be terminated */
#ifdef HAVE_CHARSET
/* Numbers of (file I/O) and (input/display) codepages. -1 if not selected */
int default_source_codepage = -1;
char *autodetect_codeset = NULL;
gboolean is_autodetect_codeset_enabled = FALSE;
#endif /* !HAVE_CHARSET */

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
check_codeset (void)
{
    const char *current_system_codepage = NULL;

    current_system_codepage = str_detect_termencoding ();

#ifdef HAVE_CHARSET
    {
        const char *_display_codepage;

        _display_codepage = get_codepage_id (mc_global.display_codepage);

        if (strcmp (_display_codepage, current_system_codepage) != 0)
        {
            mc_global.display_codepage = get_codepage_index (current_system_codepage);
            if (mc_global.display_codepage == -1)
                mc_global.display_codepage = 0;

            mc_config_set_string (mc_main_config, "Misc", "display_codepage", cp_display);
        }
    }
#endif

    mc_global.utf8_display = str_isutf8 (current_system_codepage);
}

/* --------------------------------------------------------------------------------------------- */

/** POSIX version.  The only version we support.  */
static void
OS_Setup (void)
{
}

/* --------------------------------------------------------------------------------------------- */

static void
init_sigchld (void)
{
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

int
main (int argc, char *argv[])
{
    GError *error = NULL;
    gboolean isInitialized;

    /* We had LC_CTYPE before, LC_ALL includs LC_TYPE as well */
    (void) setlocale (LC_ALL, "");
    (void) bindtextdomain ("mc", LOCALEDIR);
    (void) textdomain ("mc");

    if (!events_init (&error))
    {
        fprintf (stderr, _("Failed to run:\n%s\n"), error->message);
        g_error_free (error);
        (void) mc_event_deinit (NULL);
        exit (EXIT_FAILURE);
    }

    /* Set up temporary directory */
    (void) mc_tmpdir ();

    OS_Setup ();

    str_init_strings (NULL);

    /* Initialize and create home directories */
    /* do it after the screen library initialization to show the error message */
    mc_config_init_config_paths (&error);

    if (error == NULL && mc_config_deprecated_dir_present ())
        mc_config_migrate_from_old_place (&error);

    vfs_init ();
    vfs_setup_work_dir ();

    if (!mc_args_handle (argc, argv, "mc"))
        exit (EXIT_FAILURE);

    /* check terminal type
     * $TEMR must be set and not empty
     * mc_global.tty.xterm_flag is used in init_key() and tty_init()
     * Do this after mc_args_handle() where mc_args__force_xterm is set up.
     */
    mc_global.tty.xterm_flag = tty_check_term (mc_args__force_xterm);

    /* NOTE: This has to be called before tty_init or whatever routine
       calls any define_sequence */
    init_key ();

    /* Install the SIGCHLD handler; must be done before init_subshell() */
    init_sigchld ();

    /* We need this, since ncurses endwin () doesn't restore the signals */
    save_stop_handler ();

    /* Must be done before init_subshell, to set up the terminal size: */
    /* FIXME: Should be removed and LINES and COLS computed on subshell */
    tty_init (!mc_args__nomouse, mc_global.tty.xterm_flag);

    load_setup ();

    /* start check mc_global.display_codepage and mc_global.source_codepage */
    check_codeset ();

    /* Removing this from the X code let's us type C-c */
    load_key_defs ();

    load_keymap_defs (!mc_args__nokeymap);

    tty_init_colors (mc_global.tty.disable_colors, mc_args__force_colors);

    {
        GError *error2 = NULL;
        isInitialized = mc_skin_init (&error2);
        dlg_set_default_colors ();

        if (!isInitialized)
        {
            message (D_ERROR, _("Warning"), "%s", error2->message);
            g_error_free (error2);
            error2 = NULL;
        }
    }

    if (error != NULL)
    {
        message (D_ERROR, _("Warning"), "%s", error->message);
        g_error_free (error);
        error = NULL;
    }

    /* Also done after init_subshell, to save any shell init file messages */
    if (mc_global.tty.alternate_plus_minus)
        application_keypad_mode ();

    /* Program main loop */
    if (!mc_global.widget.midnight_shutdown)
        do_nc ();

    free_keymap_defs ();

    /* Virtual File System shutdown */
    vfs_shut ();

    mc_skin_deinit ();
    tty_colors_done ();

    tty_shutdown ();

    done_setup ();

    if (mc_global.tty.alternate_plus_minus)
        numeric_keypad_mode ();

    (void) signal (SIGCHLD, SIG_DFL);   /* Disable the SIGCHLD handler */

    done_key ();

    str_uninit_strings ();

    (void) mc_event_deinit (&error);

    mc_config_deinit_config_paths ();

    if (error != NULL)
    {
        fprintf (stderr, _("\nFailed while close:\n%s\n"), error->message);
        g_error_free (error);
        exit (EXIT_FAILURE);
    }

    (void) putchar ('\n');      /* Hack to make shell's prompt start at left of screen */

    return 0;
}

/* --------------------------------------------------------------------------------------------- */
