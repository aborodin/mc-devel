/*
   Event type specific functions

   Copyright (C) 2018
   The Free Software Foundation, Inc.

   Written by:
   Andrew Borodin, 2018

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
 *  \brief Source: Event type specific functions
 *  \author Andrew Borodin
 *  \date 2018
 */

#include <config.h>

#include "global.h"
#include "lib/widget/dialog-switch.h"   /* midnight_dlg */
#include "event-types.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/**
 *  Base queue_event_t desctuctor
 */
void
queue_event_deinit (queue_event_t * event)
{
    g_free (event->command);
    if (event->free != NULL)
        event->free (event);
    g_free (event);
}

/* --------------------------------------------------------------------------------------------- */
/**
 * qev_editor_run_t creation function
 *
 * @path VSF object for file name
 * @internal TRUE for internal editor, FALSE for external one
 * @start_line start line number to open @path
 *
 * @return newly-allocated qev_editor_run_t event
 */

queue_event_t *
qev_editor_run_init (struct vfs_path_t *path, gboolean internal, long start_line, gboolean direct)
{
    qev_editor_run_t *qer;
    queue_event_t *ev;

    qer = g_new (qev_editor_run_t, 1);
    ev = QUEUE_EVENT (qer);

    ev->receiver = midnight_dlg;
    ev->command = g_strdup (MCEVENT_EDITOR_RUN);
    ev->free = (GFreeFunc) qev_editor_run_deinit;

    qer->internal = internal;
    qer->path = path;
    qer->start_line = start_line;
    qer->direct = direct;

    return ev;
}

/* --------------------------------------------------------------------------------------------- */
/**
 * qev_editor_run_t desctuctor
 */

void
qev_editor_run_deinit (queue_event_t * event)
{
    qev_editor_run_t *ev = QEV_EDITOR_RUN (event);

    vfs_path_free (ev->path);
}

/* --------------------------------------------------------------------------------------------- */
/**
 * qev_viewer_run_t creation function
 *
 * @command command to view result of
 * @path VFS object for file name
 * @plain TRUE to view raw file, FALSE to view filtered one
 * @internal TRUE for internal editor, FALSE for external one
 * @start_line start line number to open @path
 * @search_start offset of search result start
 * @search_end offset of search result end
 *
 * @return newly-allocated qev_viewer_run_t event
 */

queue_event_t *
qev_viewer_run_init (char *command, struct vfs_path_t *path, gboolean plain, gboolean internal,
                     long start_line, off_t search_start, off_t search_end)
{
    qev_viewer_run_t *qvr;
    queue_event_t *ev;

    qvr = g_new (qev_viewer_run_t, 1);
    ev = QUEUE_EVENT (qvr);

    ev->receiver = midnight_dlg;
    ev->command = g_strdup (MCEVENT_VIEWER_RUN);
    ev->free = (GFreeFunc) qev_viewer_run_deinit;

    qvr->internal = internal;
    qvr->plain = plain;
    qvr->path = path;
    qvr->command = command;
    qvr->start_line = start_line;
    qvr->search_start = search_start;
    qvr->search_end = search_end;

    return ev;
}

/* --------------------------------------------------------------------------------------------- */
/**
 * qev_viewer_run_t desctuctor
 */

void
qev_viewer_run_deinit (queue_event_t * event)
{
    qev_viewer_run_t *ev = QEV_VIEWER_RUN (event);

    vfs_path_free (ev->path);
    g_free (ev->command);
}

/* --------------------------------------------------------------------------------------------- */
