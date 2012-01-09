#ifndef MC__KEYBIND_H
#define MC__KEYBIND_H

#include <sys/types.h>
#include <sys/time.h>           /* time_t */

#include "lib/global.h"

/*** typedefs(not structures) and defined constants **********************************************/

/* keymap sections */
#define KEYMAP_SECTION_MAIN "main"
#define KEYMAP_SECTION_MAIN_EXT "main:xmap"
#define KEYMAP_SECTION_PANEL "panel"
#define KEYMAP_SECTION_DIALOG "dialog"
#define KEYMAP_SECTION_INPUT "input"
#define KEYMAP_SECTION_LISTBOX "listbox"
#define KEYMAP_SECTION_TREE "tree"
#define KEYMAP_SECTION_HELP "help"

#define KEYMAP_SHORTCUT_LENGTH 32       /* FIXME: is 32 bytes enough for shortcut? */

/*** enums ***************************************************************************************/

enum
{
    /* special commands */
    CK_InsertChar = -1,
    CK_IgnoreKey = 0,

    /* common */
    CK_Enter = 1,
    CK_Up,
    CK_Down,
    CK_Left,
    CK_Right,
    CK_Home,
    CK_End,
    CK_LeftQuick,
    CK_RightQuick,
    CK_PageUp,
    CK_PageDown,
    CK_HalfPageUp,
    CK_HalfPageDown,
    CK_Top,
    CK_Bottom,
    CK_TopOnScreen,
    CK_MiddleOnScreen,
    CK_BottomOnScreen,
    CK_WordLeft,
    CK_WordRight,
    CK_Copy,
    CK_Move,
    CK_Delete,
    CK_MakeDir,
    CK_ChangeMode,
    CK_ChangeOwn,
    CK_ChangeOwnAdvanced,
    CK_Remove,
    CK_BackSpace,
    CK_Redo,
    CK_Clear,
    CK_Menu,
    CK_MenuLastSelected,
    CK_UserMenu,
    CK_EditUserMenu,
    CK_Search,
    CK_SearchContinue,
    CK_Replace,
    CK_ReplaceContinue,
    CK_SearchStop,
    CK_Help,
    CK_Edit,
    CK_EditNew,
    CK_Shell,
    CK_SelectCodepage,
    CK_History,
    CK_HistoryNext,
    CK_HistoryPrev,
    CK_Complete,
    CK_Save,
    CK_SaveAs,
    CK_Goto,
    CK_Reread,
    CK_Refresh,
    CK_Suspend,
    CK_Swap,
    CK_Mark,
    CK_HotList,
    CK_ScreenList,
    CK_ScreenNext,
    CK_ScreenPrev,
    CK_FilePrev,
    CK_FileNext,
    CK_DeleteToHome,
    CK_DeleteToEnd,
    CK_DeleteToWordBegin,
    CK_DeleteToWordEnd,
    CK_ShowNumbers,
    CK_Store,
    CK_Cut,
    CK_Paste,
    CK_MarkLeft,
    CK_MarkRight,
    CK_MarkUp,
    CK_MarkDown,
    CK_MarkToWordBegin,
    CK_MarkToWordEnd,
    CK_MarkToHome,
    CK_MarkToEnd,
    CK_ToggleNavigation,
    CK_Sort,
    CK_Options,
    CK_LearnKeys,
    CK_Bookmark,
    CK_Quit,
    CK_QuitQuiet,
    /* C-x or similar */
    CK_ExtendedKeyMap,

    /* main commands */
    CK_EditForceInternal = 100,
    CK_View,
    CK_ViewRaw,
    CK_ViewFile,
    CK_ViewFiltered,
    CK_Find,
    CK_DirSize,
    CK_HotListAdd,
    CK_PanelListingChange,
    CK_CompareDirs,
    CK_OptionsVfs,
    CK_OptionsConfirm,
    CK_PutCurrentLink,
    CK_PutOtherLink,
    CK_OptionsDisplayBits,
    CK_EditExtensionsFile,
    CK_EditFileHighlightFile,
    CK_LinkSymbolicEdit,
    CK_ExternalPanelize,
    CK_Filter,
    CK_PanelInfo,
    CK_OptionsLayout,
    CK_Link,
    CK_PanelListing,
    CK_ListMode,
    CK_CdQuick,
    CK_VfsList,
    CK_SaveSetup,
    CK_LinkSymbolic,
    CK_PanelListingSwitch,
    CK_ShowHidden,
    CK_PanelTree,
    CK_Tree,
    CK_Undelete,
    CK_SplitVertHoriz,
    CK_CompareFiles,
    CK_OptionsPanel,
    CK_LinkSymbolicRelative,
    CK_PutCurrentPath,
    CK_PutOtherPath,
    CK_PutCurrentTagged,
    CK_PutOtherTagged,
    CK_Select,
    CK_Unselect,
    CK_SelectInvert,

    /* panels */
    CK_PanelOtherCd = 200,
    CK_PanelOtherCdLink,
    CK_Panelize,
    CK_CopySingle,
    CK_MoveSingle,
    CK_DeleteSingle,
    CK_CdChild,
    CK_CdParent,
    CK_CdParentSmart,
    CK_PanelOtherSync,
    CK_SortNext,
    CK_SortPrev,
    CK_SortReverse,
    CK_SortByName,
    CK_SortByExt,
    CK_SortBySize,
    CK_SortByMTime,

    /* dialog */
    CK_Ok = 300,
    CK_Cancel,

    /* input */
    CK_Yank = 350,

    /* help */
    CK_Index = 400,
    CK_Back,
    CK_LinkNext,
    CK_LinkPrev,
    CK_NodeNext,
    CK_NodePrev,

    /* tree */
    CK_Forget = 450,
};

/*** structures declarations (and typedefs of structures)*****************************************/

typedef struct name_keymap_t
{
    const char *name;
    unsigned long val;
} name_keymap_t;

typedef struct key_config_t
{
    time_t mtime;               /* mtime at the moment we read config file */
    GArray *keymap;
    GArray *ext_keymap;
    gchar *labels[10];
} key_config_t;

/* The global keymaps are of this type */
typedef struct global_keymap_t
{
    long key;
    unsigned long command;
    char caption[KEYMAP_SHORTCUT_LENGTH];
} global_keymap_t;

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void keybind_cmd_bind (GArray * keymap, const char *keybind, unsigned long action);
unsigned long keybind_lookup_action (const char *name);
const char *keybind_lookup_actionname (unsigned long action);
const char *keybind_lookup_keymap_shortcut (const global_keymap_t * keymap, unsigned long action);
unsigned long keybind_lookup_keymap_command (const global_keymap_t * keymap, long key);

/*** inline functions ****************************************************************************/

#endif /* MC__KEYBIND_H */
