#ifndef MC__LUA_PRE_INIT_H
#define MC__LUA_PRE_INIT_H

/*
 * The reason we have this header file, instead of just putting the below
 * declaration in modules.h, is because this function is called by main(),
 * and if we #include modules.h in main.c, it will complain about the
 * unknown type lua_State used in that header. We can of course put forward
 * declaration to lua_State in module.h, but this doesn't seem elegant.
 */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void mc_lua_pre_init (void);    /* implemented in modules.c */

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_PRE_INIT_H */
