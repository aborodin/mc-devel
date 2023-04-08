#ifndef MC__LUA_FIELDS_H
#define MC__LUA_FIELDS_H

/*
 * Since we're referring to WPanel here, you'll have to #include this file after src/filemanager/panel.h.
 */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void mc_lua_set_current_field (WPanel * panel, const char *field_id);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_FIELDS_H */
