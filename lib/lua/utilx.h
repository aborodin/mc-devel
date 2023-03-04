#ifndef MC__LUA_UTIL_H
#define MC__LUA_UTIL_H

/*** typedefs(not structures) and defined constants **********************************************/

#define STREQ(a,b) (strcmp (a,b) == 0)

/* For programmer-facing errors. See explanation in modules/locale.c. */
#define E_(String) _(String)

/* Debugging aid. */
#if 0
#define d_message(args) (printf args)
#else
#define d_message(args)
#endif

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_UTIL_H */
