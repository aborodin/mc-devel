#ifndef MC__SCRIPTING_H
#define MC__SCRIPTING_H

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/* forward declarations */
struct Widget;
typedef struct Widget Widget;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void scripting_trigger_event (const char *event_name);
void scripting_trigger_widget_event (const char *event_name, Widget * w);
void scripting_notify_on_widget_destruction (Widget * w);

/*** inline functions ****************************************************************************/

#endif /* MC__SCRIPTING_H */
