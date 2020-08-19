
/** \file screen.h
 *  \brief Header: WScreen widget
 */

#ifndef MC__WIDGET_SCREEN_H
#define MC__WIDGET_SCREEN_H

/*** typedefs(not structures) and defined constants **********************************************/

#define SCREEN(x) ((WScreen *)(x))
#define CONST_SCREEN(x) ((const WScreen *)(x))

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    WGroup base;                /* base */

    char *event_group;          /**< Name of event group for program */

    char *(*get_title) (const WSCreen * s, size_t len); /**< String for menu of screen list */
} WScreen;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

WScreen *screen_new (widget_cb_fn callback, widget_mouse_cb_fn mouse_callback);
/* Default callback for screens */
cb_ret_t screen_default_callback (Widget * w, Widget * sender, widget_msg_t msg, int parm,
                                  void *data);

char *screen_get_title (const WScreen * h, size_t len);

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_PROGRAM_H */
