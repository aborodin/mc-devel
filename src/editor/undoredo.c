/*
   Editor undo/redo engine.

   Copyright (C) 2016
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru> 2016

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
 *  \brief Source: editor undo/redo action.
 *  \author Andrew Borodin
 *  \date 2016
 */

#include <config.h>

#include <stdlib.h>

#include "lib/global.h"

#include "edit-impl.h"
#include "editwidget.h"         /* WEdit */
#include "undoredo.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
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
 * of the cursor functions define'd in edit-impl.h. 1000 through 700'000'000 is to
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
 *
 *
 * @param edit editor object
 * @param c code of the action
 */

void
edit_undo_push_action (edit_action_stack_t * stack, long c)
{
    unsigned long sp = stack->pointer;
    unsigned long spm1;

    /* first enlarge the stack if necessary */
    if (sp > stack->size - 10)
    {                           /* say */
        if (option_max_undo < 256)
            option_max_undo = 256;

        if (stack->size < (unsigned long) option_max_undo)
        {
            long *t;

            t = g_realloc (stack->stack, (stack->size * 2 + 10) * sizeof (long));
            if (t != NULL)
            {
                stack->stack = t;
                stack->size *= 2;
                stack->size_mask = stack->size - 1;
            }
        }
    }

    spm1 = (stack->pointer - 1) & stack->size_mask;

    if (stack->disable)
    {
        edit_push_redo_action (edit, KEY_PRESS);
        edit_push_redo_action (edit, c);
        return;
    }

    if (stack->reset)
        stack->bottom = stack->pointer = 0;

    if (stack->bottom != sp && spm1 != stack->bottom && ((sp - 2) & stack->size_mask) != stack->bottom)
    {
        long d;

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
                if (c >= KEY_PRESS)
                    return;     /* --> no need to push multiple do-nothings */
                stack->stack[sp] = -2;
                goto check_bottom;
            }
        }
    }
    stack->stack[sp] = c;

  check_bottom:
    stack->pointer = (stack->pointer + 1) & stack->size_mask;

    /* if the sp wraps round and catches the undo_stack.bottom then erase
     * the first set of actions on the stack to make space - by moving
     * undo_stack.bottom forward one "key press" */
    c = (stack->pointer + 2) & stack->size_mask;
    if ((unsigned long) c == stack->bottom ||
        (((unsigned long) c + 1) & stack->size_mask) == stack->bottom)
        do
        {
            stack->bottom = (stack->bottom + 1) & stack->size_mask;
        }
        while (stack->stack[stack->bottom] < KEY_PRESS
               && stack->bottom != stack->pointer);

    /* If a single key produced enough pushes to wrap all the way round
     * then we would notice that the [undo_stack.bottom] does not contain KEY_PRESS.
     * The stack is then initialised:
     */
    if (stack->pointer != stack->bottom
        && stack->stack[stack->bottom] < KEY_PRESS)
        stack->bottom = stack->pointer = 0;
}

/* --------------------------------------------------------------------------------------------- */

/*
   TODO: if the user undos until the stack bottom, and the stack has not wrapped,
   then the file should be as it was when he loaded up. Then set edit->modified to 0.
 */

long
edit_undo_pop_action (edit_action_stack_t * stack)
{
    long c;
    unsigned long sp = stack->pointer;

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    sp = (sp - 1) & stack->size_mask;
    c = stack->stack[sp];
    if (c >= 0)
    {
        /*      stack->stack[sp] = '@'; */
        stack->pointer = (stack->pointer - 1) & stack->size_mask;
        return c;
    }

    if (sp == stack->bottom)
        return STACK_BOTTOM;

    c = stack->stack[(sp - 1) & stack->size_mask];
    if (stack->stack[sp] == -2)
    {
        /*      stack->stack[sp] = '@'; */
        stack->pointer = sp;
    }
    else
        stack->stack[sp]++;

    return c;
}

/* --------------------------------------------------------------------------------------------- */

long
edit_undo_get_prev_action (const edit_action_stack_t * stack)
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

void
edit_redo_push_action (edit_action_stack_t * stack, long c)
{
    unsigned long sp = stack->pointer;
    unsigned long spm1;

    /* first enlarge the stack if necessary */
    if (sp > stack->size - 10)
    {                           /* say */
        if (option_max_undo < 256)
            option_max_undo = 256;

        if (stack->size < (unsigned long) option_max_undo)
        {
            long *t;

            t = g_realloc (stack->stack,
                           (stack->size * 2 + 10) * sizeof (long));
            if (t != NULL)
            {
                stack->stack = t;
                stack->size <<= 1;
                stack->size_mask = stack->size - 1;
            }
        }
    }

    spm1 = (stack->pointer - 1) & stack->size_mask;

    if (stack->bottom != sp
        && spm1 != stack->bottom
        && ((sp - 2) & stack->size_mask) != stack->bottom)
    {
        long d;

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
                if (c >= KEY_PRESS)
                    return;     /* --> no need to push multiple do-nothings */
                stack->stack[sp] = -2;
                goto redo_check_bottom;
            }
        }
    }
    stack->stack[sp] = c;

  redo_check_bottom:
    stack->pointer = (stack->pointer + 1) & stack->size_mask;

    /* if the sp wraps round and catches the redo_stack.bottom then erase
     * the first set of actions on the stack to make space - by moving
     * redo_stack.bottom forward one "key press" */
    c = (stack->pointer + 2) & stack->size_mask;
    if ((unsigned long) c == stack->bottom ||
        (((unsigned long) c + 1) & stack->size_mask) == stack->bottom)
        do
        {
            stack->bottom = (stack->bottom + 1) & stack->size_mask;
        }
        while (stack->stack[stack->bottom] < KEY_PRESS
               && stack->bottom != stack->pointer);

    /*
     * If a single key produced enough pushes to wrap all the way round then
     * we would notice that the [redo_stack.bottom] does not contain KEY_PRESS.
     * The stack is then initialised:
     */
    if (stack->pointer != stack->bottom
        && stack->stack[stack->bottom] < KEY_PRESS)
        stack->bottom = stack->pointer = 0;
}

/* --------------------------------------------------------------------------------------------- */

long
edit_redo_pop_action (edit_action_stack_t * stack)
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
