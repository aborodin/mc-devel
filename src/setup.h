/** \file setup.h
 *  \brief Header: setup loading/saving
 */

#ifndef MC__SETUP_H
#define MC__SETUP_H

#include <config.h>

#include "lib/global.h"         /* GError */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/* global paremeters */
extern char *profile_name;
extern char *global_profile_name;
extern int drop_menus;
extern int verbose;
extern int auto_save_setup;

/*** declarations of public functions ************************************************************/

char *setup_init (void);
void load_setup (void);
gboolean save_setup (gboolean save_options);
void done_setup (void);
void save_config (void);
void setup_save_config_show_error (const char *filename, GError ** error);

void load_key_defs (void);

void load_keymap_defs (gboolean load_from_file);
void free_keymap_defs (void);

/*** inline functions ****************************************************************************/

#endif /* MC__SETUP_H */
