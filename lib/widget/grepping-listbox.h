
/** \file grepping-listbox.h
 *  \brief Header: WGreppingListbox widget
 */

#ifndef MC__WIDGET_GREPPING_LISTBOX_H
#define MC__WIDGET_GREPPING_LISTBOX_H

/*** typedefs(not structures) and defined constants **********************************************/

#define GREPPING_LISTBOX(x) ((WGreppingListbox *)(x))

/*** enums ***************************************************************************************/

typedef enum
{
    GREPPING_LISTBOX_MODE_DEFAULT = 0,  /* listbox only */
    GREPPING_LISTBOX_MODE_FILTER,       /* input line as filter */
    GREPPING_LISTBOX_MODE_SEARCH        /* input line as quick search */
} grepping_listbox_mode_t;

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct WGreppingListbox
{
    WGroup base;

    /* widgets */
    WListbox *listbox;
    WLabel *label;
    WInput *input;

    grepping_listbox_mode_t mode;

    GQueue *list;       /*< listbox->list in GREPPING_LISTBOX_MODE_DEFAULT mode */
} WGreppingListbox;

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *grepping_listbox_map;

/*** declarations of public functions ************************************************************/

WGreppingListbox *grepping_listbox_new (int y, int x, int height, int width, gboolean deletable,
                                        const char *histname);

/*** inline functions ****************************************************************************/

#endif /* MC__WIDGET_GREPPING_LISTBOX_H */
