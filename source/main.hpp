#pragma once

#include <windows.h>
#include <commctrl.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>
#include <type_traits>



/*****************************************************************************/
/*                                                                           */
/*                           CONSTANTS AND DEFINES                           */
/*                                                                           */
/*****************************************************************************/
LPCSTR CONST className = "NVWorldEdit";
LPCSTR CONST windowTitle = "NVWorldEdit v0.0.1subalpha";
DWORD CONST windowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
                          WS_CLIPCHILDREN;
POINT CONST windowSize = { 640, 480 };
enum class MenuItems {
  FILE_OPEN_MASTER,
  FILE_NEW_MOD,
  FILE_OPEN_MOD,
  FILE_SAVE_MOD,
  FILE_EXIT
};
#define CONTROLKEY(KEY, MENUITEM) \
{ (UINT)FCONTROL | (UINT)FVIRTKEY, KEY,(INT)MenuItems::MENUITEM }
ACCEL CONST acceleratorsTable[] = {
    CONTROLKEY('M', FILE_OPEN_MASTER),
    CONTROLKEY('N', FILE_NEW_MOD),
    CONTROLKEY('O', FILE_OPEN_MOD),
    CONTROLKEY('S', FILE_SAVE_MOD),
    CONTROLKEY('Q', FILE_EXIT)
};
#define ARRAYLENGTH(ARRAY) std::extent<decltype(ARRAY)>::value;
SIZE_T CONST acceleratorsTableLength = ARRAYLENGTH(acceleratorsTable);
#define TBSTATE_DISABLED 0
#define TOOLBARBUTTON(ICON, MENUITEM, STYLE, TEXT) { MAKELONG(ICON, 0), \
(INT)MenuItems::MENUITEM,  TBSTATE_##STYLE, BTNS_AUTOSIZE, { 0 }, 0, \
(INT_PTR)(TEXT) }
TBBUTTON toolbarButtons[] = {
    TOOLBARBUTTON(STD_FILENEW, FILE_OPEN_MASTER, ENABLED, "Open Master"),
    TOOLBARBUTTON(STD_FILEOPEN, FILE_NEW_MOD, DISABLED, "New Mod"),
    TOOLBARBUTTON(STD_FILEOPEN, FILE_OPEN_MOD, DISABLED, "Open Mod"),
    TOOLBARBUTTON(STD_FILESAVE, FILE_SAVE_MOD, DISABLED, "Save Mod")
};
SIZE_T CONST toolbarButtonsLength = ARRAYLENGTH(toolbarButtons);



/*****************************************************************************/
/*                                                                           */
/*                             GLOBAL VARIABLES                              */
/*                                                                           */
/*****************************************************************************/
BOOL done = FALSE;
HWND window = nullptr;
HMENU menubar = nullptr;
HMENU fileMenu = nullptr;
HACCEL accelerators = nullptr;
HWND toolbar = nullptr;
HIMAGELIST toolbarImageList = nullptr;
MSG message = {};
WNDCLASS windowClass = {};
OPENFILENAME openFileName;
UINT8 fileName[MAX_PATH] = "";
UINT8 *masterFileContents = nullptr;
UINT8 *modFileContents = nullptr;



/*****************************************************************************/
/*                                                                           */
/*                       FUNCTION FORWARD DECLARATIONS                       */
/*                                                                           */
/*****************************************************************************/

void HandleFileOpenMaster();

LONG WINAPI WindowMessageHandler(HWND, UINT, WPARAM, LPARAM);
