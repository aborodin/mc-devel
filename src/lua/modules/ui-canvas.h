#ifndef MC__LUA_UI_CANVAS_H
#define MC__LUA_UI_CANVAS_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void luaUI_new_canvas (lua_State * L);
void luaUI_set_canvas_dimensions (lua_State * L, int index, const WRect * r);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_UI_CANVAS_H */
