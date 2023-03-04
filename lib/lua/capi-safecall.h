#ifndef MC__LUA_CAPI_SAFECALL_H
#define MC__LUA_CAPI_SAFECALL_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

gboolean luaMC_safe_call (lua_State * L, int nargs, int nresults);
int luaMC_safe_dofile (lua_State * L, const char *dirname, const char *basename);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_CAPI_SAFECALL_H */
