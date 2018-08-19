#include "main.hpp"

INT WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance,
                   LPTSTR commandLine, int commandShow) {
  accelerators = CreateAcceleratorTable((LPACCEL)acceleratorsTable,
                                        acceleratorsTableLength);
  windowClass.style = CS_OWNDC;
  windowClass.lpfnWndProc = (WNDPROC)WindowMessageHandler;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = 0;
  windowClass.hInstance = instance;
  windowClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
  windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  windowClass.hbrBackground = nullptr;
  windowClass.lpszMenuName = nullptr;
  windowClass.lpszClassName = className;
  RegisterClass(&windowClass);
  window = CreateWindow(className, windowTitle, windowStyle, 0, 0, windowSize.x,
                        windowSize.y, nullptr, nullptr, instance, nullptr);
  ShowWindow(window, commandShow);
  while (!done && message.message != WM_QUIT) {
    if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
      if (!TranslateAccelerator(window, accelerators, &message)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
      }
    } else {
      Sleep(50);
    }
  }
  return (int)message.wParam;
}

VOID HandleFileOpenMaster() {
  // TODO: Guess where NV is https://gamedev.stackexchange.com/q/124100
  // TODO: Expand this to a custom modal dialog which has you find a folder and
  // TODO:   then lists all ESMs and stores stuff in registry.
  ZeroMemory(&fileName, sizeof(fileName));
  ZeroMemory(&openFileName, sizeof(openFileName));
  openFileName.lStructSize = sizeof(openFileName);
  openFileName.hwndOwner = window;
  openFileName.lpstrFilter = "ESM Files (*.esm)\0*.esm\0All Files (*.*)\0*.*\0";
  openFileName.lpstrFile = (LPSTR)fileName;
  openFileName.nMaxFile = MAX_PATH;
  openFileName.Flags = Sys::OFN::EXPLORER |
          Sys::OFN::FILEMUSTEXIST;
  openFileName.lpstrDefExt = "esm";
  int getFileOpenNameResult = GetOpenFileName(&openFileName);
  if (getFileOpenNameResult) {
    HANDLE fileHandle = CreateFile(openFileName.lpstrFile, GENERIC_READ,
                                   FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
    LARGE_INTEGER fileSizeInBytes;
    BOOL getFileSizeResult = GetFileSizeEx(fileHandle, &fileSizeInBytes);
    masterFileContents = (UINT8 *)HeapAlloc(GetProcessHeap(), 0,
                                            (size_t)fileSizeInBytes.QuadPart);
    DWORD numberOfBytesActuallyRead = 0;
    BOOL readFileResult = ReadFile(fileHandle, masterFileContents,
                                   (DWORD)fileSizeInBytes.QuadPart,
                                   &numberOfBytesActuallyRead, nullptr);
//        BOOL foundCell = FALSE;
//        while (!foundCell) {
//
//        }
    // Find CELL
    HeapFree(GetProcessHeap(), 0, masterFileContents);
  }
  EnableMenuItem(fileMenu, (INT)MenuItems::FILE_NEW_MOD, MF_ENABLED);
  EnableMenuItem(fileMenu, (INT)MenuItems::FILE_OPEN_MOD, MF_ENABLED);
}

VOID DoInitialMenuBarSetup(HWND window) {
  menubar = CreateMenu();
  fileMenu = CreateMenu();
  AppendMenu(fileMenu, Sys::MF::STRING, (INT)MenuItems::FILE_OPEN_MASTER,
             "Open &Master...\tCtrl+M");
  AppendMenu(fileMenu, Sys::MF::STRING | Sys::MF::GRAYED,
          (INT)MenuItems::FILE_NEW_MOD, "&New Mod...\tCtrl+N");
  AppendMenu(fileMenu, Sys::MF::STRING | Sys::MF::GRAYED,
             (INT)MenuItems::FILE_OPEN_MOD, "&Open Mod...\tCtrl+O");
  AppendMenu(fileMenu, Sys::MF::SEPARATOR, 0, "-");
  AppendMenu(fileMenu, Sys::MF::STRING | Sys::MF::GRAYED,
             (INT)MenuItems::FILE_SAVE_MOD, "&Save Mod...\tCtrl+S");
  AppendMenu(fileMenu, Sys::MF::SEPARATOR, 0, "-");
  AppendMenu(fileMenu, Sys::MF::STRING, (INT)MenuItems::FILE_EXIT,
             "E&xit\tCtrl-Q");
  AppendMenu(menubar, Sys::MF::POPUP, (UINT_PTR)fileMenu, "&File");
  SetMenu(window, menubar);
}

VOID DoInitialToolbarSetup(HWND window) {
  toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, nullptr,
                           Sys::WS::CHILD |
                           Sys::TBSTYLE::WRAPABLE, 0, 0, 0, 0,
                           window, nullptr, nullptr, nullptr);
  toolbarImageList = ImageList_Create(16, 16,
          Sys::ILC::COLOR16 | Sys::ILC::MASK, 3, 0);
  SendMessage(toolbar, TB_SETIMAGELIST, (WPARAM)0,
              (LPARAM)toolbarImageList);
  SendMessage(toolbar, TB_LOADIMAGES, (WPARAM)IDB_STD_SMALL_COLOR,
              (LPARAM)HINST_COMMCTRL);
  SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
  SendMessage(toolbar, TB_ADDBUTTONS, (WPARAM)toolbarButtonsLength,
              (LPARAM)&toolbarButtons);
  SendMessage(toolbar, TB_AUTOSIZE, 0, 0);
  ShowWindow(toolbar, TRUE);
}

VOID HandleCreateMessage(HWND window) {
  DoInitialMenuBarSetup(window);
  DoInitialToolbarSetup(window);
}

LONG WINAPI WindowMessageHandler(HWND window, UINT message, WPARAM wParam,
                                 LPARAM lParam) {
  static PAINTSTRUCT paintStruct;
  switch (message) {
    case WM_CREATE:
      HandleCreateMessage(window);
      break;
    case WM_COMMAND:
      switch (wParam & 0xFFFFU) {
        case (INT)MenuItems::FILE_OPEN_MASTER:
          HandleFileOpenMaster();
          break;
        case (INT)MenuItems::FILE_NEW_MOD:
          MessageBox(nullptr, "New mod...", "", MB_OK);
          break;
        case (INT)MenuItems::FILE_OPEN_MOD:
          MessageBox(nullptr, "Open mod...", "", MB_OK);
          break;
        case (INT)MenuItems::FILE_SAVE_MOD:
          MessageBox(nullptr, "Save mod...", "", MB_OK);
          break;
        case (INT)MenuItems::FILE_EXIT:
          PostQuitMessage(0);
          break;
        default:
          break;
      }
      break;
    case WM_PAINT:
      BeginPaint(window, &paintStruct);
      EndPaint(window, &paintStruct);
      break;
    case WM_CLOSE:
      PostQuitMessage(0);
      break;
    default:
      break;
  }
  return (LONG)DefWindowProc(window, message, wParam, lParam);
}
