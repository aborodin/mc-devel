#ifndef MC__ARGS_H
#define MC__ARGS_H

#include "lib/global.h"         /* gboolean */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

extern gboolean mc_args__force_xterm;
extern gboolean mc_args__nomouse;
extern gboolean mc_args__force_colors;
extern gboolean mc_args__nokeymap;
extern gboolean mc_args__version;
extern char *mc_args__keymap_file;

/*** declarations of public functions ************************************************************/

gboolean mc_args_handle (int argc, char **argv, const char *translation_domain);

/*** inline functions ****************************************************************************/
#endif /* MC__ARGS_H */
