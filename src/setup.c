/*
   Setup loading/saving.

   Copyright (C) 1994, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
   2006, 2007, 2009, 2010, 2011
   The Free Software Foundation, Inc.

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

/** \file setup.c
 *  \brief Source: setup loading/saving
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib/global.h"

#include "lib/tty/tty.h"
#include "lib/tty/key.h"
#include "lib/mcconfig.h"
#include "lib/fileloc.h"
#include "lib/timefmt.h"
#include "lib/strutil.h"
#include "lib/util.h"
#include "lib/widget.h"

#include "lib/vfs/vfs.h"

#ifdef HAVE_CHARSET
#include "lib/charsets.h"
#endif

#include "args.h"
#include "keybind-defaults.h"   /* keybind_lookup_action */
#include "main.h"

#ifdef HAVE_CHARSET
#include "selcodepage.h"
#endif

#include "setup.h"

/*** global variables ****************************************************************************/

char *profile_name;             /* ${XDG_CONFIG_HOME}/mc/ini */
char *global_profile_name;      /* mc.lib */

/* This flag indicates if the pull down menus by default drop down */
int drop_menus = 0;

int setup_copymove_persistent_attr = 1;

/* Ugly hack to allow panel_save_setup to work as a place holder for */
/* default panel values */
int saving_setup;

/* It true saves the setup when quitting */
int auto_save_setup = 1;

int verbose = 1;

/*** file scope macro definitions ****************************************************************/

/* In order to use everywhere the same setup for the locale we use defines */
#define FMTYEAR _("%b %e  %Y")
#define FMTTIME _("%b %e %H:%M")

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* *INDENT-OFF* */
static const struct
{
    const char *opt_name;
    int *opt_addr;
} int_options [] = {
    { "verbose", &verbose },
    { "auto_save_setup", &auto_save_setup },
    { "mouse_repeat_rate", &mou_auto_repeat },
    { "double_click_speed", &double_click_speed },
#ifndef HAVE_CHARSET
    { "eight_bit_clean", &mc_global.eight_bit_clean },
    { "full_eight_bits", &mc_global.full_eight_bits },
#endif /* !HAVE_CHARSET */
    { "use_8th_bit_as_meta", &use_8th_bit_as_meta },
    { "mouse_close_dialog", &mouse_close_dialog},
    { "fast_refresh", &fast_refresh },
    { "old_esc_mode", &old_esc_mode },
    { "old_esc_mode_timeout", &old_esc_mode_timeout },
    { "show_all_if_ambiguous", &mc_global.widget.show_all_if_ambiguous },
    { "alternate_plus_minus", &mc_global.tty.alternate_plus_minus },
    { "num_history_items_recorded", &num_history_items_recorded },
    { NULL, NULL }
};
/* *INDENT-ON* */

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
  Get name of config file.

 \param subdir
 if not NULL, then config also search into specified subdir.

 \param config_file_name
 If specified filename is relative, then will search in standart patches.

 \return
 Newly allocated path to config name or NULL if file not found.

 If config_file_name is a relative path, then search config in stantart paths.
*/
static char *
load_setup_get_full_config_name (const char *subdir, const char *config_file_name)
{
    /*
       TODO: IMHO, in future this function must be placed into mc_config module.
     */
    char *lc_basename, *ret;

    if (config_file_name == NULL)
        return NULL;

    if (g_path_is_absolute (config_file_name))
        return g_strdup (config_file_name);


    lc_basename = g_path_get_basename (config_file_name);
    if (lc_basename == NULL)
        return NULL;

    if (subdir != NULL)
        ret = g_build_filename (mc_config_get_path (), subdir, lc_basename, NULL);
    else
        ret = g_build_filename (mc_config_get_path (), lc_basename, NULL);

    if (exist_file (ret))
    {
        g_free (lc_basename);
        return ret;
    }
    g_free (ret);

    if (subdir != NULL)
        ret = g_build_filename (mc_global.sysconfig_dir, subdir, lc_basename, NULL);
    else
        ret = g_build_filename (mc_global.sysconfig_dir, lc_basename, NULL);

    if (exist_file (ret))
    {
        g_free (lc_basename);
        return ret;
    }
    g_free (ret);

    if (subdir != NULL)
        ret = g_build_filename (mc_global.share_data_dir, subdir, lc_basename, NULL);
    else
        ret = g_build_filename (mc_global.share_data_dir, lc_basename, NULL);

    g_free (lc_basename);

    if (exist_file (ret))
        return ret;

    g_free (ret);
    return NULL;

}

/* --------------------------------------------------------------------------------------------- */
/**
  Create new mc_config object from specified ini-file or
  append data to existing mc_config object from ini-file
*/

static void
load_setup_init_config_from_file (mc_config_t ** config, const char *fname)
{
    /*
       TODO: IMHO, in future this function must be placed into mc_config module.
     */
    if (exist_file (fname))
    {
        if (*config != NULL)
            mc_config_read_file (*config, fname, TRUE);
        else
            *config = mc_config_init (fname);
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
load_keys_from_section (const char *terminal, mc_config_t * cfg)
{
    char *section_name;
    gchar **profile_keys, **keys;
    gchar **values, **curr_values;
    char *valcopy, *value;
    long key_code;
    gsize len, values_len;

    if (terminal == NULL)
        return;

    section_name = g_strconcat ("terminal:", terminal, (char *) NULL);
    profile_keys = keys = mc_config_get_keys (cfg, section_name, &len);

    while (*profile_keys != NULL)
    {
        /* copy=other causes all keys from [terminal:other] to be loaded. */
        if (g_ascii_strcasecmp (*profile_keys, "copy") == 0)
        {
            valcopy = mc_config_get_string (cfg, section_name, *profile_keys, "");
            load_keys_from_section (valcopy, cfg);
            g_free (valcopy);
            profile_keys++;
            continue;
        }

        curr_values = values =
            mc_config_get_string_list (cfg, section_name, *profile_keys, &values_len);

        key_code = lookup_key (*profile_keys, NULL);

        if (key_code != 0)
        {
            if (curr_values != NULL)
            {
                while (*curr_values != NULL)
                {
                    valcopy = convert_controls (*curr_values);
                    define_sequence (key_code, valcopy, MCKEY_NOACTION);
                    g_free (valcopy);
                    curr_values++;
                }
            }
            else
            {
                value = mc_config_get_string (cfg, section_name, *profile_keys, "");
                valcopy = convert_controls (value);
                define_sequence (key_code, valcopy, MCKEY_NOACTION);
                g_free (valcopy);
                g_free (value);
            }
        }

        profile_keys++;
        g_strfreev (values);
    }
    g_strfreev (keys);
    g_free (section_name);
}

/* --------------------------------------------------------------------------------------------- */

static void
load_keymap_from_section (const char *section_name, GArray * keymap, mc_config_t * cfg)
{
    gchar **profile_keys, **keys;
    gsize len;

    if (section_name == NULL)
        return;

    profile_keys = keys = mc_config_get_keys (cfg, section_name, &len);

    while (*profile_keys != NULL)
    {
        gchar **values, **curr_values;

        curr_values = values = mc_config_get_string_list (cfg, section_name, *profile_keys, &len);

        if (curr_values != NULL)
        {
            int action;

            action = keybind_lookup_action (*profile_keys);
            if (action > 0)
                while (*curr_values != NULL)
                {
                    keybind_cmd_bind (keymap, *curr_values, action);
                    curr_values++;
                }

            g_strfreev (values);
        }

        profile_keys++;
    }

    g_strfreev (keys);
}

/* --------------------------------------------------------------------------------------------- */

static mc_config_t *
load_setup_get_keymap_profile_config (gboolean load_from_file)
{
    /*
       TODO: IMHO, in future this function must be placed into mc_config module.
     */
    mc_config_t *keymap_config;
    char *fname, *fname2;

    /* 0) Create default keymap */
    keymap_config = create_default_keymap ();
    if (!load_from_file)
        return keymap_config;

    /* 1) /usr/share/mc (mc_global.share_data_dir) */
    fname = g_build_filename (mc_global.share_data_dir, GLOBAL_KEYMAP_FILE, NULL);
    load_setup_init_config_from_file (&keymap_config, fname);
    g_free (fname);

    /* 2) /etc/mc (mc_global.sysconfig_dir) */
    fname = g_build_filename (mc_global.sysconfig_dir, GLOBAL_KEYMAP_FILE, NULL);
    load_setup_init_config_from_file (&keymap_config, fname);
    g_free (fname);

    /* 3) ${XDG_CONFIG_HOME}/mc */
    fname = mc_config_get_full_path (GLOBAL_KEYMAP_FILE);
    load_setup_init_config_from_file (&keymap_config, fname);
    g_free (fname);

    /* 4) main config; [Midnight Commander] -> keymap */
    fname2 =
        mc_config_get_string (mc_main_config, CONFIG_APP_SECTION, "keymap", GLOBAL_KEYMAP_FILE);
    fname = load_setup_get_full_config_name (NULL, fname2);
    if (fname != NULL)
    {
        load_setup_init_config_from_file (&keymap_config, fname);
        g_free (fname);
    }
    g_free (fname2);

    /* 5) getenv("MC_KEYMAP") */
    fname = load_setup_get_full_config_name (NULL, g_getenv ("MC_KEYMAP"));
    if (fname != NULL)
    {
        load_setup_init_config_from_file (&keymap_config, fname);
        g_free (fname);
    }

    /* 6) --keymap=<keymap> */
    fname = load_setup_get_full_config_name (NULL, mc_args__keymap_file);
    if (fname != NULL)
    {
        load_setup_init_config_from_file (&keymap_config, fname);
        g_free (fname);
    }

    return keymap_config;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

char *
setup_init (void)
{
    char *profile;
    char *inifile;

    if (profile_name != NULL)
        return profile_name;

    profile = mc_config_get_full_path (MC_CONFIG_FILE);
    if (!exist_file (profile))
    {
        inifile = concat_dir_and_file (mc_global.sysconfig_dir, "mc.ini");
        if (exist_file (inifile))
        {
            g_free (profile);
            profile = inifile;
        }
        else
        {
            g_free (inifile);
            inifile = concat_dir_and_file (mc_global.share_data_dir, "mc.ini");
            if (exist_file (inifile))
            {
                g_free (profile);
                profile = inifile;
            }
            else
                g_free (inifile);
        }
    }

    profile_name = profile;

    return profile;
}

/* --------------------------------------------------------------------------------------------- */

void
load_setup (void)
{
    char *profile;
    size_t i;
    char *buffer;
    const char *kt;

#ifdef HAVE_CHARSET
    load_codepages_list ();
#endif /* HAVE_CHARSET */

    profile = setup_init ();

    /* mc.lib is common for all users, but has priority lower than
       ${XDG_CONFIG_HOME}/mc/ini.  FIXME: it's only used for keys and treestore now */
    global_profile_name =
        g_build_filename (mc_global.sysconfig_dir, MC_GLOBAL_CONFIG_FILE, (char *) NULL);
    if (!exist_file (global_profile_name))
    {
        g_free (global_profile_name);
        global_profile_name =
            g_build_filename (mc_global.share_data_dir, MC_GLOBAL_CONFIG_FILE, (char *) NULL);
    }

    mc_main_config = mc_config_init (profile);

    /* Load integer boolean options */
    for (i = 0; int_options[i].opt_name != NULL; i++)
        *int_options[i].opt_addr =
            mc_config_get_int (mc_main_config, CONFIG_APP_SECTION, int_options[i].opt_name,
                               *int_options[i].opt_addr);

    /* overwrite old_esc_mode_timeout */
    kt = getenv ("KEYBOARD_KEY_TIMEOUT_US");
    if ((kt != NULL) && (kt[0] != '\0'))
        old_esc_mode_timeout = atoi (kt);

    /* Load time formats */
    user_recent_timeformat =
        mc_config_get_string (mc_main_config, "Misc", "timeformat_recent", FMTTIME);
    user_old_timeformat = mc_config_get_string (mc_main_config, "Misc", "timeformat_old", FMTYEAR);

    /* The default color and the terminal dependent color */
    mc_global.tty.setup_color_string =
        mc_config_get_string (mc_main_config, "Colors", "base_color", "");
    mc_global.tty.term_color_string =
        mc_config_get_string (mc_main_config, "Colors", getenv ("TERM"), "");
    mc_global.tty.color_terminal_string =
        mc_config_get_string (mc_main_config, "Colors", "color_terminals", "");

    /* Load the directory history */
    /*    directory_history_load (); */
    /* Remove the temporal entries */

#ifdef HAVE_CHARSET
    if (codepages->len > 1)
    {
        buffer = mc_config_get_string (mc_main_config, "Misc", "display_codepage", "");
        if (buffer[0] != '\0')
        {
            mc_global.display_codepage = get_codepage_index (buffer);
            cp_display = get_codepage_id (mc_global.display_codepage);
        }
        g_free (buffer);
        buffer = mc_config_get_string (mc_main_config, "Misc", "source_codepage", "");
        if (buffer[0] != '\0')
        {
            default_source_codepage = get_codepage_index (buffer);
            mc_global.source_codepage = default_source_codepage;        /* May be source_codepage doesn't need this */
            cp_source = get_codepage_id (mc_global.source_codepage);
        }
        g_free (buffer);
    }

    autodetect_codeset = mc_config_get_string (mc_main_config, "Misc", "autodetect_codeset", "");
    if ((autodetect_codeset[0] != '\0') && (strcmp (autodetect_codeset, "off") != 0))
        is_autodetect_codeset_enabled = TRUE;

    g_free (init_translation_table (mc_global.source_codepage, mc_global.display_codepage));
    buffer = (char *) get_codepage_id (mc_global.display_codepage);
    if (buffer != NULL)
        mc_global.utf8_display = str_isutf8 (buffer);
#endif /* HAVE_CHARSET */
}

/* --------------------------------------------------------------------------------------------- */

gboolean
save_setup (gboolean save_options)
{
    gboolean ret = TRUE;

    saving_setup = 1;

    if (save_options)
    {
        char *tmp_profile;

        save_config ();

#ifdef HAVE_CHARSET
        mc_config_set_string (mc_main_config, "Misc", "display_codepage",
                              get_codepage_id (mc_global.display_codepage));
        mc_config_set_string (mc_main_config, "Misc", "source_codepage",
                              get_codepage_id (default_source_codepage));
        mc_config_set_string (mc_main_config, "Misc", "autodetect_codeset", autodetect_codeset);
#endif /* HAVE_CHARSET */

        tmp_profile = mc_config_get_full_path (MC_CONFIG_FILE);
        ret = mc_config_save_to_file (mc_main_config, tmp_profile, NULL);
        g_free (tmp_profile);
    }

    saving_setup = 0;

    return ret;
}

/* --------------------------------------------------------------------------------------------- */

void
done_setup (void)
{
    size_t i;

    g_free (profile_name);
    g_free (global_profile_name);
    g_free (mc_global.tty.color_terminal_string);
    g_free (mc_global.tty.term_color_string);
    g_free (mc_global.tty.setup_color_string);
    mc_config_deinit (mc_main_config);

    g_free (user_recent_timeformat);
    g_free (user_old_timeformat);

#ifdef HAVE_CHARSET
    g_free (autodetect_codeset);
    free_codepages_list ();
#endif
}

/* --------------------------------------------------------------------------------------------- */

void
save_config (void)
{
    size_t i;

    /* Save integer options */
    for (i = 0; int_options[i].opt_name != NULL; i++)
        mc_config_set_int (mc_main_config, CONFIG_APP_SECTION, int_options[i].opt_name,
                           *int_options[i].opt_addr);
}

/* --------------------------------------------------------------------------------------------- */

void
setup_save_config_show_error (const char *filename, GError ** error)
{
    if (error != NULL && *error != NULL)
    {
        message (D_ERROR, MSG_ERROR, _("Cannot save file %s:\n%s"), filename, (*error)->message);
        g_error_free (*error);
        *error = NULL;
    }
}

/* --------------------------------------------------------------------------------------------- */

void
load_key_defs (void)
{
    /*
     * Load keys from mc.lib before ${XDG_CONFIG_HOME}/mc/ini, so that the user
     * definitions override global settings.
     */
    mc_config_t *mc_global_config;

    mc_global_config = mc_config_init (global_profile_name);
    if (mc_global_config != NULL)
    {
        load_keys_from_section ("general", mc_global_config);
        load_keys_from_section (getenv ("TERM"), mc_global_config);
        mc_config_deinit (mc_global_config);
    }

    load_keys_from_section ("general", mc_main_config);
    load_keys_from_section (getenv ("TERM"), mc_main_config);
}

/* --------------------------------------------------------------------------------------------- */

void
load_keymap_defs (gboolean load_from_file)
{
    /*
     * Load keymap from GLOBAL_KEYMAP_FILE before ${XDG_CONFIG_HOME}/mc/mc.keymap, so that the user
     * definitions override global settings.
     */
    mc_config_t *mc_global_keymap;

    mc_global_keymap = load_setup_get_keymap_profile_config (load_from_file);

    if (mc_global_keymap != NULL)
    {
        dialog_keymap = g_array_new (TRUE, FALSE, sizeof (global_keymap_t));
        load_keymap_from_section (KEYMAP_SECTION_DIALOG, dialog_keymap, mc_global_keymap);

        input_keymap = g_array_new (TRUE, FALSE, sizeof (global_keymap_t));
        load_keymap_from_section (KEYMAP_SECTION_INPUT, input_keymap, mc_global_keymap);

        listbox_keymap = g_array_new (TRUE, FALSE, sizeof (global_keymap_t));
        load_keymap_from_section (KEYMAP_SECTION_LISTBOX, listbox_keymap, mc_global_keymap);

        tree_keymap = g_array_new (TRUE, FALSE, sizeof (global_keymap_t));
        load_keymap_from_section (KEYMAP_SECTION_TREE, tree_keymap, mc_global_keymap);

        help_keymap = g_array_new (TRUE, FALSE, sizeof (global_keymap_t));
        load_keymap_from_section (KEYMAP_SECTION_HELP, help_keymap, mc_global_keymap);

        mc_config_deinit (mc_global_keymap);
    }

    dialog_map = (global_keymap_t *) dialog_keymap->data;
    input_map = (global_keymap_t *) input_keymap->data;
    listbox_map = (global_keymap_t *) listbox_keymap->data;
    tree_map = (global_keymap_t *) tree_keymap->data;
    help_map = (global_keymap_t *) help_keymap->data;
}

/* --------------------------------------------------------------------------------------------- */

void
free_keymap_defs (void)
{
    if (dialog_keymap != NULL)
        g_array_free (dialog_keymap, TRUE);
    if (input_keymap != NULL)
        g_array_free (input_keymap, TRUE);
    if (listbox_keymap != NULL)
        g_array_free (listbox_keymap, TRUE);
    if (tree_keymap != NULL)
        g_array_free (tree_keymap, TRUE);
    if (help_keymap != NULL)
        g_array_free (help_keymap, TRUE);
}

/* --------------------------------------------------------------------------------------------- */
