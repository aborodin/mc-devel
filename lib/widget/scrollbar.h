#ifndef MC__WIDGET_SCROLLBAR_H
#define MC__WIDGET_SCROLLBAR_H

/*** typedefs(not structures) and defined constants **********************************************/

#define SCROLLBAR(x) ((WScrollBar *)(x))

/*** enums ***************************************************************************************/

/* WScrollBar part codes */
typedef enum
{
    SB_AREA_UP = 0,     /**< top arrow of vertical scroll bar */
    SB_AREA_DOWN,       /**< bottom arrow of vertical scroll bar */
    SB_AREA_PAGE_UP,    /**< track area upper vertical thumb */
    SB_AREA_PAGE_DOWN,  /**< track area below vertical thumb */
    SB_AREA_LEFT,       /**< left arrow of horizontal scroll bar */
    SB_AREA_RIGHT,      /**< right arrow of horizontal scroll bar */
    SB_AREA_PAGE_LEFT,  /**< track area to the left of horizontal thumb */
    SB_AREA_PAGE_RIGHT, /**< track area to the right of horizontal thumb */
    SB_AREA_THUMB       /**< thumb position on scroll bar */
} scrollbar_area_t;

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    Widget widget;

    int maximum;
    int minimum;
    int value;
    int step;           /**< arrow-clicket step */
    int page;           /**< track-clicked step */
} WScrollBar;

/*** global variables defined in .c file *********************************************************/

extern const global_keymap_t *scrollbar_map;

/*** declarations of public functions ************************************************************/

WScrollBar *scrollbar_new (int y, int x, int lines);
void scrollbar_set_range (WScrollBar * sb, int minimum, int maximum);
void scrollbar_set_value (WScrollBar * sb, int value);

/* --------------------------------------------------------------------------------------------- */
/*** inline functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static inline void
scrollbar_set_step (WScrollBar * sb, int step)
{
    sb->step = step;
}

/* --------------------------------------------------------------------------------------------- */

static inline void
scrollbar_set_page (WScrollBar * sb, int page)
{
    sb->page = page;
}

/* --------------------------------------------------------------------------------------------- */

#endif /* MC__WIDGET_SCROLLBAR_H */
