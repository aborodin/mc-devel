/*
   Virtual File System: GNU Tar file system.

   Copyright (C) 1995-2021
   Free Software Foundation, Inc.

   Written by:
   Andrew Borodin <aborodin@vmail.ru>, 2020

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

/**
 * \file
 * \brief Source: Virtual File System: GNU Tar file system
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "lib/global.h"

#include "tar-internal.h"

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/* General Interface */

/* Since tar VFS is read-only, inplement decodes only */
struct xhdr_tab
{
    const char *keyword;
    void (*decoder) (struct tar_stat_info * st, char *keyword, char *arg);
};

/*** file scope variables ************************************************************************/

/* Forward declarations */
static void dummy_handler (struct tar_stat_info *st, char *keyword, char *arg);
static void atime_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void gid_decoder (struct tar_stat_info *st, char *keyword, char *arg);
#if 0
static void gname_decoder (struct tar_stat_info *st, char *keyword, char *arg);
#endif
static void linkpath_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void mtime_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void ctime_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void path_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void size_decoder (struct tar_stat_info *st, char *keyword, char *arg);
static void uid_decoder (struct tar_stat_info *st, char *keyword, char *arg);
#if 0
static void uname_decoder (struct tar_stat_info *st, char *keyword, char *arg);
#endif

/* *INDENT-OFF* */
static struct xhdr_tab xhdr_tab[] =
{
    { "atime",    atime_decoder },
    { "comment",  dummy_handler },
    { "charset",  dummy_handler },
    { "ctime",    ctime_decoder },
    { "gid",      gid_decoder },
#if 0
    { "gname",    gname_decoder },
#endif
    { "linkpath", linkpath_decoder },
    { "mtime",    mtime_decoder },
    { "path",     path_decoder },
    { "size",     size_decoder },
    { "uid",      uid_decoder },
#if 0
    { "uname",    uname_decoder },
#endif
    { NULL }
};
/* *INDENT-ON* */

/* --------------------------------------------------------------------------------------------- */
/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

static void
dummy_handler (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) st;
    (void) keyword;
    (void) arg;
}

/* --------------------------------------------------------------------------------------------- */

static void
atime_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_atime = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

static void
gid_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_gid = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

#if 0
static void
gname_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->gname = g_strdup (arg);
}
#endif

/* --------------------------------------------------------------------------------------------- */

static void
linkpath_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->link_name = g_strdup (arg);
}

/* --------------------------------------------------------------------------------------------- */

static void
ctime_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_ctime = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

static void
mtime_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_mtime = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

static void
path_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    g_free (st->orig_file_name);
    st->orig_file_name = g_strdup (arg);
#if 0
    g_free (st->file_name);
    st->file_name = g_strdup (arg);
#endif
}

/* --------------------------------------------------------------------------------------------- */

static void
size_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_size = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

static void
uid_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->stat.st_uid = strtoul (arg, NULL, 0);
}

/* --------------------------------------------------------------------------------------------- */

#if 0
static void
uname_decoder (struct tar_stat_info *st, char *keyword, char *arg)
{
    (void) keyword;

    st->uname = g_strdup (arg);
}
#endif

/* --------------------------------------------------------------------------------------------- */

static struct xhdr_tab *
locate_handler (char *keyword)
{
    struct xhdr_tab *p;

    for (p = xhdr_tab; p->keyword != NULL; p++)
        if (strcmp (p->keyword, keyword) == 0)
            return p;

    return NULL;
}

/* --------------------------------------------------------------------------------------------- */

static gboolean
decode_record (char **p, struct tar_stat_info *st)
{
    size_t len;
    char *keyword, *eqp;
    char *start = *p;
    struct xhdr_tab *t;

    if (**p == '\0')
        return FALSE;

    len = strtoul (*p, p, 10);
    if (**p != ' ')
    {
#if 0
        message (D_ERROR, MSG_ERROR, _("Malformed extended headed"));
#endif
        return FALSE;
    }

    keyword = ++*p;
    for (; *p < start + len; ++*p)
        if (**p == '=')
            break;

    if (**p != '=')
    {
#if 0
        message (D_ERROR, MSG_ERROR, _("Malformed extended headed"));
#endif
        return FALSE;
    }

    eqp = *p;
    **p = '\0';
    t = locate_handler (keyword);
    if (t != NULL)
    {
        char endc;
        char *value;

        value = ++*p;

        endc = start[len - 1];
        start[len - 1] = '\0';
        t->decoder (st, keyword, value);
        start[len - 1] = endc;
    }

    *eqp = '=';
    *p = &start[len];

    return TRUE;
}

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

gboolean
xheader_decode (GArray * extended_header, struct tar_stat_info * st)
{
    char *p, *endp;

    p = &g_array_index (extended_header, char, 0);
    endp = &g_array_index (extended_header, char, extended_header->len - 1);

    while (p < endp)
        if (decode_record (&p, st))
            return TRUE;

    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

void
xheader_read (tar_super_t * archive, union block *p, size_t size)
{
    size_t i, nblocks;

    nblocks = (size + BLOCKSIZE - 1) / BLOCKSIZE;

    if (archive->extended_header == NULL)
        archive->extended_header = g_array_new (FALSE, FALSE, size + 1);
    else
        g_array_set_size (archive->extended_header, 0);

    tar_set_next_block_after (p);

    for (i = 0; i < nblocks; i++)
    {
        size_t len;

        p = tar_find_next_block (archive);
        len = MIN (size, BLOCKSIZE);
        g_array_append_vals (archive->extended_header, p->buffer, len);
        tar_set_next_block_after (p);

        size -= len;
    }
}

/* --------------------------------------------------------------------------------------------- */
