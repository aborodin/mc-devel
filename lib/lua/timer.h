#ifndef MC__LUA_TIMER_H
#define MC__LUA_TIMER_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

gint64 mc_lua_timer_now (void);
void mc_lua_set_next_timeout (gint64 tm);
void mc_lua_execute_ready_timeouts (void);
gboolean mc_lua_has_pending_timeouts (gint64 * time_out);
void mc_lua_timer_unlock (void);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_TIMER_H */
