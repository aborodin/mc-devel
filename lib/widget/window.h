
/** \file window.h
 *  \brief Header: WWindow widget
 */

#ifndef MC__WIDGET_WINDOW_H
#define MC__WIDGET_WINDOW_H

/*** typedefs(not structures) and defined constants **********************************************/

#define WINDOW(x) ((WWindow *)(x))
#define CONST_WINDOW(x) ((const WWindow *)(x))

/*** enums ***************************************************************************************/

/*** typedefs(not structures) ********************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    WGroup base;

    gboolean compact;           /* Suppress spaces around the frame */
    const char *help_ctx;       /* Name of the help entry */
    const int *colors;          /* Color set. Unused in viewer and editor */

    char *event_group;          /* Name of event group for this window */
    Widget *bg;                 /* WFrame or WBackground */
} WWindow;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_WINDOW_H */
