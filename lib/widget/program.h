
/** \file program.h
 *  \brief Header: WProgram widget
 */

#ifndef MC__WIDGET_PROGRAM_H
#define MC__WIDGET_PROGRAM_H

/*** typedefs(not structures) and defined constants **********************************************/

#define PROGRAM(x) ((WProgram *)(x))
#define CONST_PROGRAM(x) ((const WProgram *)(x))

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    WGroup base;                /* base */

    char *event_group;          /* Name of event group for program */
} WProgram;

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *main_map;

/*** declarations of public functions ************************************************************/

WProgram *program_new (widget_cb_fn callback, widget_mouse_cb_fn mouse_callback);

void program_run (WProgram * p);
void program_refresh (WProgram * p);

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_PROGRAM_H */
