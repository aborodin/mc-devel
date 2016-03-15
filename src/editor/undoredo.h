/** \file
 *  \brief Header: undo/redo engine for WEdit
 */

#ifndef MC__EDIT_UNDOREDO_H
#define MC__EDIT_UNDOREDO_H

/*** typedefs(not structures) and defined constants **********************************************/

/* Initial size of the undo stack, in bytes */
#define START_STACK_SIZE 32

/* Some codes that may be pushed onto or returned from the undo stack */
#define CURS_LEFT       601
#define CURS_RIGHT      602
#define DELCHAR         603
#define BACKSPACE       604
#define STACK_BOTTOM    605
#define CURS_LEFT_LOTS  606
#define CURS_RIGHT_LOTS 607
#define COLUMN_ON       608
#define COLUMN_OFF      609
#define DELCHAR_BR      610
#define BACKSPACE_BR    611
#define MARK_1          1000
#define MARK_2          500000000
#define MARK_CURS       1000000000
#define KEY_PRESS       1500000000

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct
{
    long *stack;
    unsigned long size;
    unsigned long size_mask;
    unsigned long pointer;
    unsigned long bottom;
    gboolean disable;           /* If TRUE, don't save events in the undo stack */
} edit_undo_stack_t;

typedef struct
{
    long *stack;
    unsigned long size;
    unsigned long size_mask;
    unsigned long pointer;
    unsigned long bottom;
    gboolean reset;             /* If TRUE, need clear redo stack */
} edit_redo_stack_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void edit_push_undo_action (WEdit * edit, long c);
long edit_pop_undo_action (WEdit * edit);
long edit_get_prev_undo_action (WEdit * edit);

void edit_push_redo_action (WEdit * edit, long c);
long edit_pop_redo_action (WEdit * edit);

/*** inline functions ****************************************************************************/

/* --------------------------------------------------------------------------------------------- */

#endif /* MC__EDIT_UNDOREDO_H */
