#ifndef MC__LUA_PLUMBING_H
#define MC__LUA_PLUMBING_H

/*
 * If a function needs a lua_State argument then it means it doesn't belong here.
 */

/*** typedefs(not structures) and defined constants **********************************************/

#define MCEVENT_GROUP_LUA "Lua" /* used for mc_event_add(), mc_event_raise(). */

/* Names of environment variables with which user can override directory paths. */
#define MC_LUA_SYSTEM_DIR__ENVAR "MC_LUA_SYSTEM_DIR"
#define MC_LUA_USER_DIR__ENVAR "MC_LUA_USER_DIR"

/*** enums ***************************************************************************************/

typedef enum
{
    MC_LUA_SCRIPT_RESULT_CONTINUE,
    MC_LUA_SCRIPT_RESULT_FINISH,
    MC_LUA_SCRIPT_RESULT_ERROR
} mc_lua_script_result_t;

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* -------------------------- Meta information ---------------------------- */
const char *mc_lua_engine_name (void);
const char *mc_lua_system_dir (void);
const char *mc_lua_user_dir (void);

/* ----------------------------- Start/stop ------------------------------- */
void mc_lua_init (void);
void mc_lua_load (void);
void mc_lua_shutdown (void);

/* ------------------------------- Runtime -------------------------------- */

gboolean mc_lua_eat_key (int keycode);
void mc_lua_trigger_event (const char *event_name);
gboolean mc_lua_ui_is_ready (void);

/* --------------------------- mcscript-related --------------------------- */
mc_lua_script_result_t mc_lua_run_script (const char *filename);
void mc_lua_create_argv (const char *script_path, int argc, char **argv, int offs);

/*** inline functions ****************************************************************************/

#endif /* MC__LUA_PLUMBING_H */
