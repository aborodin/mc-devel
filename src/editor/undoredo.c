/*
   Editor undo and redo stacks.

   Copyright (C) 2014
   The Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru> 2014

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *  \brief Source: editor undo and redo stacks.
 *  \author Andrew Borodin
 *  \date 2014
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "lib/global.h"

#include "edit-impl.h"
#include "editwidget.h"
#include "undoredo.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

#define UNDOREDOSTACK(x) ((edit_undoredo_stack_t *) (x))

/* Initial size of stacks */
#define START_STACK_SIZE 32

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
edit_undoredo_stack_init (edit_undoredo_stack_t * stack, struct WEdit *edit)
{
    stack->size = START_STACK_SIZE;
    stack->size_mask = START_STACK_SIZE - 1;
    stack->stack = g_malloc0 ((stack->size + 10) * sizeof (long));
    stack->bottom = 0;
    stack->pointer = 0;
    stack->edit = edit;
}

/* --------------------------------------------------------------------------------------------- */

static void
edit_undoredo_stack_enlarge (edit_undoredo_stack_t * stack)
{
    if (stack->pointer > stack->size - 10)
    {
        if (option_max_undo < 256)
            option_max_undo = 256;

        if (stack->size < (unsigned long) option_max_undo)
        {
            long *t;

            t = g_try_realloc (stack->stack, (stack->size * 2 + 10) * sizeof (long));
            if (t != NULL)
            {
                stack->stack = t;
                stack->size *= 2;
                stack->size_mask = stack->size - 1;
            }
        }
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
edit_undoredo_stack_check_bottom (edit_undoredo_stack_t * stack)
{
    unsigned long c;

    stack->pointer = (stack->pointer + 1) & stack->size_mask;

    /* If the sp wraps round and catches the stack->bottom then erase
     * the first set of actions on the stack->stack to make space -
     * by moving stack->bottom forward one "key press" */
    c = (unsigned long) ((stack->pointer + 2) & stack->size_mask);
    if (c == stack->bottom || ((c + 1) & stack->size_mask) == stack->bottom)
    {
        do
        {
            stack->bottom = (stack->bottom + 1) & stack->size_mask;
        }
        while (stack->stack[stack->bottom] < KEY_PRESS && stack->bottom != stack->pointer);
    }

    /* If a single key produced enough pushes to wrap all the way round then
     * we would notice that the [stack->bottom] does not contain KEY_PRESS.
     * The stack is then initialised: */
    if (stack->pointer != stack->bottom && stack->stack[stack->bottom] < KEY_PRESS)
    {
        stack->bottom = 0;
        stack->pointer = 0;
    }
}

/* --------------------------------------------------------------------------------------------- */

static void
edit_push_undoredo_action (edit_undoredo_stack_t * stack, long c)
{
    unsigned long sp, spm1;

    sp = stack->pointer;
    spm1 = (sp - 1) & stack->size_mask;

    if (sp != stack->bottom && spm1 != stack->bottom
        && ((sp - 2) & stack->size_mask) != stack->bottom)
    {
        int d;

        if (stack->stack[spm1] < 0)
        {
            d = stack->stack[(sp - 2) & stack->size_mask];
            if (d == c && stack->stack[spm1] > -1000000000)
            {
                if (c < KEY_PRESS)      /* --> no need to push multiple do-nothings */
                    stack->stack[spm1]--;
                return;
            }
        }
        else
        {
            d = stack->stack[spm1];
            if (d == c)
            {
                if (c < KEY_PRESS)      /* --> no need to push multiple do-nothings */
                {
                    stack->stack[sp] = -2;
                    edit_undoredo_stack_check_bottom (stack);
                }
                return;
            }
        }
    }

    stack->stack[sp] = c;
    edit_undoredo_stack_check_bottom (stack);
}

/* --------------------------------------------------------------------------------------------- */

static long
edit_pop_undoredo_action (edit_undoredo_stack_t * stack)
{
    long c;
    unsigned long sp = stack->pointer;

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    sp = (sp - 1) & stack->size_mask;
    c = stack->stack[sp];
    if (c >= 0)
    {
        stack->pointer = (stack->pointer - 1) & stack->size_mask;
        return c;
    }

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    c = stack->stack[(sp - 1) & stack->size_mask];

    if (stack->stack[sp] == -2)
        stack->pointer = sp;
    else
        stack->stack[sp]++;

    return c;
}

/* --------------------------------------------------------------------------------------------- */

static long
edit_get_prev_undoredo_action (const edit_undoredo_stack_t * stack)
{
    long c;
    unsigned long sp = stack->pointer;

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    sp = (sp - 1) & stack->size_mask;
    c = stack->stack[sp];
    if (c >= 0)
        return c;

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    c = stack->stack[(sp - 1) & stack->size_mask];

    return c;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
edit_undo_stack_init (edit_undo_stack_t * undo, struct WEdit *edit)
{
    edit_undoredo_stack_init (UNDOREDOSTACK (undo), edit);
    undo->disable = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

void
edit_undo_stack_free (edit_undo_stack_t * undo)
{
    g_free (undo->base.stack);
}

/* --------------------------------------------------------------------------------------------- */
/**
 * Recording stack for undo:
 * The following is an implementation of a compressed stack. Identical
 * pushes are recorded by a negative prefix indicating the number of times the
 * same char was pushed. This saves space for repeated curs-left or curs-right
 * delete etc.
 *
 * eg:
 *
 * pushed:       stored:
 *
 * a
 * b             a
 * b            -3
 * b             b
 * c  -->       -4
 * c             c
 * c             d
 * c
 * d
 *
 * If the stack long int is 0-255 it represents a normal insert (from a backspace),
 * 256-512 is an insert ahead (from a delete), If it is betwen 600 and 700 it is one
 * of the cursor functions define'd in undoredo.h. 1000 through 700'000'000 is to
 * set edit->mark1 position. 700'000'000 through 1400'000'000 is to set edit->mark2
 * position.
 *
 * The only way the cursor moves or the buffer is changed is through the routines:
 * insert, backspace, insert_ahead, delete, and cursor_move.
 * These record the reverse undo movements onto the stack each time they are
 * called.
 *
 * Each key press results in a set of actions (insert; delete ...). So each time
 * a key is pressed the current position of start_display is pushed as
 * KEY_PRESS + start_display. Then for undoing, we pop until we get to a number
 * over KEY_PRESS. We then assign this number less KEY_PRESS to start_display. So undo
 * tracks scrolling and key actions exactly. (KEY_PRESS is about (2^31) * (2/3) = 1400'000'000)
 *
 * @param undo undo stack object
 * @param c code of the action
 */

void
edit_push_undo_action (edit_undo_stack_t * undo, long c)
{
    edit_undoredo_stack_t *stack = UNDOREDOSTACK (undo);
    edit_redo_stack_t *redo = &stack->edit->redo_stack;

    /* Enlarge the stack if necessary */
    edit_undoredo_stack_enlarge (stack);

    if (undo->disable)
    {
        edit_push_redo_action (redo, KEY_PRESS);
        edit_push_redo_action (redo, c);
        return;
    }

    if (redo->reset)
    {
        UNDOREDOSTACK (redo)->bottom = 0;
        UNDOREDOSTACK (redo)->pointer = 0;
    }

    edit_push_undoredo_action (stack, c);
}

/* --------------------------------------------------------------------------------------------- */
/*
   TODO: if the user undos until the stack bottom, and the stack has not wrapped,
   then the file should be as it was when he loaded up. Then set edit->modified to 0.
 */

long
edit_pop_undo_action (edit_undo_stack_t * undo)
{
    return edit_pop_undoredo_action (UNDOREDOSTACK (undo));
}

/* --------------------------------------------------------------------------------------------- */

long
edit_get_prev_undo_action (const edit_undo_stack_t * undo)
{
    return edit_get_prev_undoredo_action (UNDOREDOSTACK (undo));
}

/* --------------------------------------------------------------------------------------------- */

void
edit_redo_stack_init (edit_redo_stack_t * redo, struct WEdit *edit)
{
    edit_undoredo_stack_init (UNDOREDOSTACK (redo), edit);
    redo->reset = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

void
edit_redo_stack_free (edit_redo_stack_t * redo)
{
    g_free (redo->base.stack);
}

/* --------------------------------------------------------------------------------------------- */

void
edit_push_redo_action (edit_redo_stack_t * redo, long c)
{
    edit_undoredo_stack_t *stack = UNDOREDOSTACK (redo);

    /* Enlarge the stack if necessary */
    edit_undoredo_stack_enlarge (stack);

    edit_push_undoredo_action (stack, c);
}

/* --------------------------------------------------------------------------------------------- */

long
edit_pop_redo_action (edit_redo_stack_t * redo)
{
    return edit_pop_undoredo_action (UNDOREDOSTACK (redo));
}

/* --------------------------------------------------------------------------------------------- */

long
edit_get_prev_redo_action (const edit_redo_stack_t * redo)
{
    return edit_get_prev_undoredo_action (UNDOREDOSTACK (redo));
}

/* --------------------------------------------------------------------------------------------- */
