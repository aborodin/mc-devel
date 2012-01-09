#ifndef MC__EVENT_TYPES_H
#define MC__EVENT_TYPES_H

#include <stdarg.h>

/*** typedefs(not structures) and defined constants **********************************************/

/* Event groups for main modules */
#define MCEVENT_GROUP_CORE "Core"
#define MCEVENT_GROUP_DIALOG "Dialog"
#define MCEVENT_GROUP_EDITOR "Editor"
#define MCEVENT_GROUP_FILEMANAGER "FileManager"
#define MCEVENT_GROUP_VIEWER "Viewer"

/* Events */
#define MCEVENT_HISTORY_LOAD "history_load"
#define MCEVENT_HISTORY_SAVE "history_save"

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/* MCEVENT_GROUP_CORE:clipboard_text_from_file */
typedef struct
{
    char **text;
    gboolean ret;
} ev_clipboard_text_from_file_t;

/* MCEVENT_GROUP_CORE:help */
typedef struct
{
    const char *filename;
    const char *node;
} ev_help_t;

/* MCEVENT_GROUP_DIALOG:history_load */
/* MCEVENT_GROUP_DIALOG:history_save */
struct mc_config_t;
struct Widget;
typedef struct
{
    struct mc_config_t *cfg;
    struct Widget *receiver;    /* NULL means broadcast message */
} ev_history_load_save_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

/*** inline functions ****************************************************************************/

#endif /* MC__EVENT_TYPES_H */
