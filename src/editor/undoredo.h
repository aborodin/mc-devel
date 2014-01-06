/** \file
 *  \brief Header: undo and redo stacks for WEdit
 */

#ifndef MC__EDIT_UNDOREDO_H
#define MC__EDIT_UNDOREDO_H

/*** typedefs(not structures) and defined constants **********************************************/

/* Some codes that may be pushed onto or returned from the undo/redo stacks */
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

typedef struct edit_undoredo_stack_struct
{
    long *stack;
    unsigned long size;
    unsigned long size_mask;
    unsigned long bottom;
    unsigned long pointer;
    struct WEdit *edit;
} edit_undoredo_stack_t;

typedef struct edit_undo_stack_struct
{
    edit_undoredo_stack_t base;
    gboolean disable;           /* If TRUE, don't save events in the undo stack */
} edit_undo_stack_t;

typedef struct edit_redo_stack_struct
{
    edit_undoredo_stack_t base;
    gboolean reset;             /* If TRUE, clear */
} edit_redo_stack_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void edit_undo_stack_init (edit_undo_stack_t * undo, struct WEdit *edit);
void edit_undo_stack_free (edit_undo_stack_t * undo);
void edit_push_undo_action (edit_undo_stack_t * undo, long c);
long edit_pop_undo_action (edit_undo_stack_t * undo);
long edit_get_prev_undo_action (const edit_undo_stack_t * undo);

void edit_redo_stack_init (edit_redo_stack_t * redo, struct WEdit *edit);
void edit_redo_stack_free (edit_redo_stack_t * redo);
void edit_push_redo_action (edit_redo_stack_t * redo, long c);
long edit_pop_redo_action (edit_redo_stack_t * redo);
long edit_get_prev_redo_action (const edit_redo_stack_t * redo);

/*** inline functions ****************************************************************************/

#endif /* MC__EDIT_UNDOREDO_H */
