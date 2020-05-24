
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

WWindow *window_new (int y, int x, int lines, int cols, widget_pos_flags_t pos_flags,
                     gboolean compact, const int *colors, widget_cb_fn callback,
                     widget_mouse_cb_fn mouse_callback,  const char *help_ctx, const char *title);
void window_init (WWindow * w, int y, int x, int lines, int cols, widget_pos_flags_t pos_flags,
                  gboolean compact, const int *colors, widget_cb_fn callback,
                  widget_mouse_cb_fn mouse_callback, const char *help_ctx);
void window_destroy (WWindow * w);

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_WINDOW_H */
