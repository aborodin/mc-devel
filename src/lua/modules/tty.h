#ifndef MC__LUA_TTY_H
#define MC__LUA_TTY_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

long luaTTY_check_keycode (lua_State * L, int name_index, gboolean push_name_short);
void luaTTY_assert_ui_is_ready (lua_State * L);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_TTY_H */
