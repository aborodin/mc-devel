/** \file main.h
 *  \brief Header: this is a main module header
 */

#ifndef MC__MAIN_H
#define MC__MAIN_H

#include "lib/global.h"

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

#ifdef HAVE_CHARSET
extern int default_source_codepage;
extern char *autodetect_codeset;
extern gboolean is_autodetect_codeset_enabled;
#endif /* !HAVE_CHARSET */

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif /* MC__MAIN_H */
