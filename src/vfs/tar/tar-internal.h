#ifndef MC__VFS_TAR_INTERNAL_H
#define MC__VFS_TAR_INTERNAL_H

#include <sys/stat.h>
#include <sys/types.h>

#include "lib/vfs/xdirentry.h"  /* vfs_s_super */

#include "tar.h"

/*** typedefs(not structures) and defined constants **********************************************/

/* tar files are made in basic blocks of this size.  */
#define BLOCKSIZE 512


/* Sparse files are not supported in POSIX ustar format.  For sparse files
   with a POSIX header, a GNU extra header is provided which holds overall
   sparse information and a few sparse descriptors.  When an old GNU header
   replaces both the POSIX header and the GNU extra header, it holds some
   sparse descriptors too.  Whether POSIX or not, if more sparse descriptors
   are still needed, they are put into as many successive sparse headers as
   necessary.  The following constants tell how many sparse descriptors fit
   in each kind of header able to hold them.  */

#define SPARSES_IN_EXTRA_HEADER  16
#define SPARSES_IN_OLDGNU_HEADER 4
#define SPARSES_IN_SPARSE_HEADER 21

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/* *INDENT-OFF* */

/* POSIX header */
struct posix_header
{                               /* byte offset */
    char name[100];             /*   0 */
    char mode[8];               /* 100 */
    char uid[8];                /* 108 */
    char gid[8];                /* 116 */
    char size[12];              /* 124 */
    char mtime[12];             /* 136 */
    char chksum[8];             /* 148 */
    char typeflag;              /* 156 */
    char linkname[100];         /* 157 */
    char magic[6];              /* 257 */
    char version[2];            /* 263 */
    char uname[32];             /* 265 */
    char gname[32];             /* 297 */
    char devmajor[8];           /* 329 */
    char devminor[8];           /* 337 */
    char prefix[155];           /* 345 */
                                /* 500 */
};

struct star_header
{                               /* byte offset */
    char name[100];             /*   0 */
    char mode[8];               /* 100 */
    char uid[8];                /* 108 */
    char gid[8];                /* 116 */
    char size[12];              /* 124 */
    char mtime[12];             /* 136 */
    char chksum[8];             /* 148 */
    char typeflag;              /* 156 */
    char linkname[100];         /* 157 */
    char magic[6];              /* 257 */
    char version[2];            /* 263 */
    char uname[32];             /* 265 */
    char gname[32];             /* 297 */
    char devmajor[8];           /* 329 */
    char devminor[8];           /* 337 */
    char prefix[131];           /* 345 */
    char atime[12];             /* 476 */
    char ctime[12];             /* 488 */
                                /* 500 */
};

/* Descriptor for a single file hole */
struct sparse
{                               /* byte offset */
    /* cppcheck-suppress unusedStructMember */
    char offset[12];            /*   0 */
    /* cppcheck-suppress unusedStructMember */
    char numbytes[12];          /*  12 */
                                /*  24 */
};

/* Extension header for sparse files, used immediately after the GNU extra
   header, and used only if all sparse information cannot fit into that
   extra header.  There might even be many such extension headers, one after
   the other, until all sparse information has been recorded.  */
struct sparse_header
{                               /* byte offset */
    struct sparse sp[SPARSES_IN_SPARSE_HEADER];
                                /*   0 */
    char isextended;            /* 504 */
                                /* 505 */
};

/* The old GNU format header conflicts with POSIX format in such a way that
   POSIX archives may fool old GNU tar's, and POSIX tar's might well be
   fooled by old GNU tar archives.  An old GNU format header uses the space
   used by the prefix field in a POSIX header, and cumulates information
   normally found in a GNU extra header.  With an old GNU tar header, we
   never see any POSIX header nor GNU extra header.  Supplementary sparse
   headers are allowed, however.  */
struct oldgnu_header
{                               /* byte offset */
    char unused_pad1[345];      /*   0 */
    char atime[12];             /* 345 */
    char ctime[12];             /* 357 */
    char offset[12];            /* 369 */
    char longnames[4];          /* 381 */
    char unused_pad2;           /* 385 */
    struct sparse sp[SPARSES_IN_OLDGNU_HEADER];
                                /* 386 */
    char isextended;            /* 482 */
    char realsize[12];          /* 483 */
                                /* 495 */
};

/* *INDENT-ON* */

/* tar Header Block, overall structure */
union block
{
    char buffer[BLOCKSIZE];
    struct posix_header header;
    struct star_header star_header;
    struct oldgnu_header oldgnu_header;
    struct sparse_header sparse_header;
};

enum archive_format
{
    TAR_UNKNOWN = 0,            /**< format to be decided later */
    TAR_V7,                     /**< old V7 tar format */
    TAR_OLDGNU,                 /**< GNU format as per before tar 1.12 */
    TAR_USTAR,                  /**< POSIX.1-1988 (ustar) format */
    TAR_POSIX,                  /**< POSIX.1-2001 format */
    TAR_STAR,                   /**< Star format defined in 1994 */
    TAR_GNU                     /**< almost same as OLDGNU_FORMAT */
};

typedef struct
{
    struct vfs_s_super base;    /* base class */

    int fd;
    struct stat st;
    enum archive_format type;   /**< type of the archive */
    union block *record_start;  /**< start of record of archive */
    GArray *extended_header;    /**< extended header (an array of chars) */
} tar_super_t;

struct tar_stat_info
{
    char *orig_file_name;       /**< name of file read from the archive header */
#if 0
    char *file_name;            /**< name of file for the current archive entry after being normalized */
#endif
    char *link_name;            /**< name of link for the current archive entry  */
#if 0
    char *uname;                /**< user name of owner */
    char *gname;                /**< group name of owner */
#endif
    struct stat stat;           /**< regular filesystem stat */
};

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/* tar.c */
void tar_assign_string (char **string, const char *value);
void tar_assign_string_n (char **string, const char *value, size_t n);
union block *tar_find_next_block (tar_super_t * archive);
gboolean tar_set_next_block_after (union block *block);

/* xheader.c */
gboolean xheader_decode (GArray * extended_header, struct tar_stat_info *st);
void xheader_read (tar_super_t * archive, union block *p, size_t size);

/*** inline functions ****************************************************************************/

#endif /* MC__VFS_TAR_INTERNAL_H */
