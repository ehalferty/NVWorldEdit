#include <windows.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>
#include <type_traits>
LPCSTR CONST className = "NVWorldEdit";
LPCSTR CONST windowTitle = "NVWorldEdit v0.0.1subalpha";
DWORD CONST windowStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
POINT CONST windowSize = { 640, 480};
enum class MenuItems {
    FILE_OPEN_MASTER = 101,
    FILE_NEW_MOD,
    FILE_OPEN_MOD,
    FILE_SAVE_MOD,
    FILE_EXIT
};
ACCEL CONST acceleratorsTable[] = {
        { (UINT)FCONTROL | (UINT)FVIRTKEY, 'M', (INT)MenuItems::FILE_OPEN_MASTER },
        { (UINT)FCONTROL | (UINT)FVIRTKEY, 'N', (INT)MenuItems::FILE_NEW_MOD },
        { (UINT)FCONTROL | (UINT)FVIRTKEY, 'O', (INT)MenuItems::FILE_OPEN_MOD },
        { (UINT)FCONTROL | (UINT)FVIRTKEY, 'S', (INT)MenuItems::FILE_SAVE_MOD },
        { (UINT)FCONTROL | (UINT)FVIRTKEY, 'Q', (INT)MenuItems::FILE_EXIT }};
SIZE_T CONST acceleratorsTableLength = std::extent<decltype(acceleratorsTable)>::value;
BOOL done = FALSE;
HWND window = nullptr;
HMENU menubar = nullptr;
HMENU fileMenu = nullptr;
HACCEL accelerators = nullptr;
MSG message = {};
WNDCLASS windowClass = {};
OPENFILENAME openFileName;
UINT8 fileName[MAX_PATH] = "";
UINT8 *masterFileContents = nullptr;
UINT8 *modFileContents = nullptr;
void HandleFileOpenMaster() {
    // TODO: Guess where NV is https://gamedev.stackexchange.com/questions/124100/is-there-a-reliable-and-fast-way-of-getting-a-list-of-all-installed-games-on-a-w
    // TODO: Expand this to a custom modal dialog which has you find a folder and then lists all ESMs
    // TODO:  and stores stuff in registry.
    ZeroMemory(&fileName, sizeof(fileName));
    ZeroMemory(&openFileName, sizeof(openFileName));
    openFileName.lStructSize = sizeof(openFileName);
    openFileName.hwndOwner = window;
    openFileName.lpstrFilter = "ESM Files (*.esm)\0*.esm\0All Files (*.*)\0*.*\0";
    openFileName.lpstrFile = (LPSTR)fileName;
    openFileName.nMaxFile = MAX_PATH;
    openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;
    openFileName.lpstrDefExt = "esm";
    int getFileOpenNameResult = GetOpenFileName(&openFileName);
    if (getFileOpenNameResult) {
        HANDLE fileHandle = CreateFile(openFileName.lpstrFile, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        LARGE_INTEGER fileSizeInBytes;
        BOOL getFileSizeResult = GetFileSizeEx(fileHandle, &fileSizeInBytes);
        masterFileContents = (UINT8 *)HeapAlloc(GetProcessHeap(), 0, (size_t)fileSizeInBytes.QuadPart);
        DWORD numberOfBytesActuallyRead = 0;
        BOOL readFileResult = ReadFile(fileHandle, masterFileContents, (size_t)fileSizeInBytes.QuadPart, &numberOfBytesActuallyRead, nullptr);
        // Find CELL
        HeapFree(GetProcessHeap(), 0, masterFileContents);
    }
    EnableMenuItem(fileMenu, (INT)MenuItems::FILE_NEW_MOD, MF_ENABLED);
    EnableMenuItem(fileMenu, (INT)MenuItems::FILE_OPEN_MOD, MF_ENABLED);
}
LONG WINAPI WindowMessageHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT paintStruct;
    switch (message) {
        case WM_CREATE:
            menubar = CreateMenu();
            fileMenu = CreateMenu();
            AppendMenu(fileMenu, MF_STRING, (INT)MenuItems::FILE_OPEN_MASTER, "Open &Master...\tCtrl+M");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, (INT)MenuItems::FILE_NEW_MOD, "&New Mod...\tCtrl+N");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, (INT)MenuItems::FILE_OPEN_MOD, "&Open Mod...\tCtrl+O");
            AppendMenu(fileMenu, MF_SEPARATOR, 0, "-");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, (INT)MenuItems::FILE_SAVE_MOD, "&Save Mod...\tCtrl+S");
            AppendMenu(fileMenu, MF_SEPARATOR, 0, "-");
            AppendMenu(fileMenu, MF_STRING, (INT)MenuItems::FILE_EXIT, "E&xit\tCtrl-Q");
            AppendMenu(menubar, MF_POPUP, (UINT_PTR)fileMenu, "&File");
            SetMenu(window, menubar);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
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
            return (LONG)DefWindowProc(window, message, wParam, lParam);
    }
}
int WINAPI WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPTSTR commandLine, int commandShow) {
    accelerators = CreateAcceleratorTable((LPACCEL)acceleratorsTable, acceleratorsTableLength);
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
    window = CreateWindow(className, windowTitle, windowStyle, 0, 0, windowSize.x, windowSize.y, nullptr, nullptr, instance, nullptr);
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











//struct RecordNodeMetadata {
//    // Node type. 0 = regular, 1 = group, 2 = sub-record
//    CHAR nodeType;
//    // Where in the file was it located when parsed?
//    UINT32 offset;
//    // Has this been changed since reading from disk?
//    BOOL dirty;
//    // If it has a parent, cache a pointer to it here.
//    struct Record *parent;
//};
//
//struct RecordHeader {
//    UINT8 type[4];
//    UINT32 dataSize;
//    UINT32 flags;
//    UINT32 formID;
//    UINT32 versionControlInfo;
//};
//
//struct GroupHeader {
//    UINT8 type[4];
//    UINT32 groupSize;
//    UINT8 label[4];
//    INT32 groupType;
//    UINT32 stamp;
//};
//
//struct SubRecordHeader {
//    UINT8 subType[4];
//    UINT16 dataSize;
//};
//
//struct RecordNode {
//    struct RecordNodeMetadata recordNodeMetadata;
//    struct RecordHeader recordHeader;
//    struct GroupHeader groupHeader;
//    struct SubRecordHeader subRecordHeader;
//    UINT8 *data;
//    UINT8 *uncompressedData;
//    struct RecordNode *subRecords;
//    struct RecordNode *next;
//    struct RecordNode *head;
//    struct RecordNode *tail;
//};

//                        BuildRecordsListFromFileContents(&masterFileRecords, masterFileContents , fileSizeInBytes);

//int asdfasd = 23423;


//                        for (int i = 0; i < (fileSizeInBytes.QuadPart / 1024); i++) {
//                            CHAR buffer[1024];
////                            CopyMemory(fileContents + i * 1024, &buffer, 1024);
//                        }
//                        CHAR masterFileContents[16*1024*1024];
//                        ZeroMemory(&masterFileContents, sizeof(masterFileContents));
//                        DWORD numberOfBytesActuallyRead = 0;
//                        BOOL readFileResult = ReadFile(fileHandle, &masterFileContents, 16*1024*1024, &numberOfBytesActuallyRead, NULL);
//                        // TODO: Parse ESM structure
//                        // TODO: Build tree of records
//                        // TODO: Print out tree to file
int sdaf = 23234;
//struct RecordNode *masterFileRecords = NULL;
//struct RecordNode *modFileRecords = NULL;

// Add new record to the linked list
//struct RecordNode *AppendRecordNode(struct RecordNode **recordsList) {
//    struct RecordNode *newNode = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct RecordNode));
//    newNode->head = *recordsList;
//    if (*recordsList) {
//        if ((*recordsList)->tail) {
//            (*recordsList)->tail->next = newNode;
//        } else {
//            (*recordsList)->next = newNode;
//        }
//        (*recordsList)->tail = newNode;
//    } else {
//        *recordsList = newNode;
//    }
//    return newNode;
//}
//
//// Set header data on record node
//VOID SetGroupHeader(struct RecordNode *recordNode, UINT32 groupSize, CONST UINT8 *label, INT32 groupType, UINT32 stamp) {
//    recordNode->groupHeader.type[0] = 'G';
//    recordNode->groupHeader.type[1] = 'R';
//    recordNode->groupHeader.type[2] = 'U';
//    recordNode->groupHeader.type[3] = 'P';
//    recordNode->groupHeader.groupSize = groupSize;
//    for (int i = 0; i < 4; i++) {
//        recordNode->groupHeader.label[i] = label[i];
//    }
//    recordNode->groupHeader.groupType = groupType;
//    recordNode->groupHeader.stamp = stamp;
//}
//
//BOOL fourCharsMatch(UINT8 CONST *fourChars, CHAR CONST fourChars2[5]) {
//    return strncmp((CHAR*)fourChars, fourChars2, 4) == 0;
//}
//
//void BuildRecordsListFromFileContents(struct RecordNode **recordsList, UINT8 CONST *fileContents, LARGE_INTEGER fileSizeInBytes) {
//    // TODO: Verify that file starts with the right stuff... 'TES4'... 'HEDR'... 'GRUP'... etc. etc.
//    // Locate all top-level records
//    // Find the first GRUP
//    UINT64 location = 0;
//    while (!fourCharsMatch(&fileContents[location], "GRUP")) { location++; }
//    // TODO: Handle GRUP not found
//    while (location < fileSizeInBytes.QuadPart) {
//        // Is there a GRUP?
//        if (fourCharsMatch(&fileContents[location], "GRUP")) {
//            CHAR thisGrupBuffer[1024];
//            for (int i = 0; i < 1024; i++) {
//                thisGrupBuffer[i] = fileContents[location + i];
//            }
//            // Read in groupSize.
//            UINT32 groupSize = fileContents[location + 4] +
//                               (fileContents[location + 5] << 8) +
//                               (fileContents[location + 6] << 16) +
//                               (fileContents[location + 7] << 24);
//            UINT64 nextGrup = location + groupSize;
//            // Load next GRUP into buffer. I just did this to verify that the next GRUP is properly aligned. Remove.
//            CHAR nextGrupBuffer[1024];
//            for (int i = 0; i < 1024; i++) {
//                nextGrupBuffer[i] = fileContents[nextGrup + i];
//            }
//            struct RecordNode *newNode = AppendRecordNode(recordsList);
//            UINT8 label[4];
//            for (int i = 0; i < 4; i++) {
//                label[i] = fileContents[location + 8 + i];
//            }
//            INT32 groupType = fileContents[location + 12] +
//                              (fileContents[location + 13] << 8) +
//                              (fileContents[location + 14] << 16) +
//                              (fileContents[location + 15] << 24);
//            UINT32 stamp = fileContents[location + 16] +
//                           (fileContents[location + 17] << 8) +
//                           (fileContents[location + 18] << 16) +
//                           (fileContents[location + 19] << 24);
//            SetGroupHeader(newNode, groupSize, label, groupType, stamp);
//            // Do we care about this group of records?
//            if (fourCharsMatch(label, "CELL")) {
//                // We definitely care about CELL records.
//                // TODO: Ge all sun-records.
//                int asdfasd = 234234;
//            } else if (fourCharsMatch(label, "REFR")) {
//                // Objects placed in cells.
//            }
//
//            // TODO:
//            int jhsdjkfhdjkh = 2343;
//            location = nextGrup;
//        } else {
//            // Unknown top-level data!
//            int sdfdfkjkdf = 2343;
//        }
//    }
//}



//HWND window;
//HDC deviceContext;
//HGLRC openGLRenderingContext;
//POINT windowSize = { 4, 3 };
//BOOL perspectiveChanged = FALSE;
//BOOL active = FALSE;
//BOOL done = FALSE;
//GLuint vertexArrayObject;
//GLuint vertexBufferObject;
//GLfloat points[] = {
//        0.0f, 0.5f, 0.0f,
//        0.5f, -0.5f, 0.0f,
//        -0.5f, -0.5f, 0.0f
//};
//const char* vertex_shader = ""
//                            "#version 410\n"
//                            "in vec3 vp;"
//                            "void main () {"
//                            "	gl_Position = vec4 (vp, 1.0);"
//                            "}";
//const char* fragment_shader = ""
//                              "#version 410\n"
//                              "out vec4 frag_colour;"
//                              "void main () {"
//                              "   frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
//                              "}";
//GLuint vertexShader, fragmentShader;
//GLuint shaderProgram;
//
//void (*glGenBuffers)(GLsizei n, GLuint * buffers);
//void (*glBindBuffer)(GLenum target, GLuint buffer);
//void (*glBufferData)(GLenum target, GLsizeiptr size, const GLvoid * data,
//                     GLenum usage);
//void (*glGenVertexArrays)(GLsizei n, GLuint *arrays);
//void (*glBindVertexArray)(GLuint array);
//void (*glEnableVertexAttribArray)(GLuint index);
//void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type,
//                              GLboolean normalized, GLsizei stride,
//                              const GLvoid * pointer);
//GLuint (*glCreateShader)(GLenum shaderType);
//void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar **string,
//                       const GLint *length);
//void (*glCompileShader)(GLuint shader);
//GLuint (*glCreateProgram)();
//void (*glAttachShader)(GLuint program, GLuint shader);
//void (*glLinkProgram)(GLuint program);
//void (*glUseProgram)(GLuint program);
//
//void LoadOpenGLFunctions() {
//    glGenBuffers = (void (*)(GLsizei, GLuint *))
//            wglGetProcAddress("glGenBuffers");
//    glBindBuffer = (void (*)(GLenum, GLuint))
//            wglGetProcAddress("glBindBuffer");
//    glBufferData = (void (*)(GLenum, GLsizeiptr, const GLvoid *, GLenum))
//            wglGetProcAddress("glBufferData");
//    glGenVertexArrays = (void (*)(GLsizei, GLuint *))
//            wglGetProcAddress("glGenVertexArrays");
//    glBindVertexArray = (void (*)(GLuint))
//            wglGetProcAddress("glBindVertexArray");
//    glEnableVertexAttribArray = (void (*)(GLuint))
//            wglGetProcAddress("glEnableVertexAttribArray");
//    glVertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei,
//                                      const GLvoid *))
//            wglGetProcAddress("glVertexAttribPointer");
//    glCreateShader = (GLuint (*)(GLenum))
//            wglGetProcAddress("glCreateShader");
//    glShaderSource = (void (*)(GLuint, GLsizei, const GLchar **, const GLint *))
//            wglGetProcAddress("glShaderSource");
//    glCompileShader = (void (*)(GLuint))
//            wglGetProcAddress("glCompileShader");
//    glCreateProgram = (GLuint (*)())
//            wglGetProcAddress("glCreateProgram");
//    glAttachShader = (void (*)(GLuint, GLuint))
//            wglGetProcAddress("glAttachShader");
//    glLinkProgram = (void (*)(GLuint))
//            wglGetProcAddress("glLinkProgram");
//    glUseProgram = (void (*)(GLuint))
//            wglGetProcAddress("glUseProgram");
//}
//
//
//
//LONG WINAPI WindowProc(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam) {
//    static PAINTSTRUCT paintStruct;
//    switch (uMsg) {
//        case WM_PAINT:
////            Render();
//            BeginPaint(window, &paintStruct);
//            EndPaint(window, &paintStruct);
//            return 0;
//        case WM_SIZE:
//            windowSize.x = LOWORD(lParam);
//            windowSize.y = HIWORD(lParam);
//            PostMessage(window, WM_PAINT, 0, 0);
//            perspectiveChanged = TRUE;
////            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
////            PostMessage(window, WM_PAINT, 0, 0);
//            return 0;
//        case WM_SYSCOMMAND:
//            if (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER) {
//                return 0;
//            }
//            break;
//        case WM_ACTIVATE:
//            active = !HIWORD(wParam);
//            return 0;
//        case WM_KEYDOWN:
//        case WM_SYSKEYDOWN:
////            keys[wParam] = TRUE;
//            return 0;
//        case WM_KEYUP:
//        case WM_SYSKEYUP:
////            keys[wParam] = FALSE;
//            return 0;
//        case WM_CLOSE:
//            PostQuitMessage(0);
//            return 0;
//        default:
//            break;
//    }
//    return (LONG)DefWindowProc(window, uMsg, wParam, lParam);
//}
//
//int WINAPI WinMain(
//        HINSTANCE hInstance,
//        HINSTANCE hPrevInstance,
//        LPTSTR lpCmdLine,
//        int nCmdShow) {
//    MSG   message;
//    WNDCLASS wc;
//    int pf;
//    PIXELFORMATDESCRIPTOR pfd;
//    hInstance = GetModuleHandle(NULL);
//    wc.style = CS_OWNDC;
//    wc.lpfnWndProc = (WNDPROC)WindowProc;
//    wc.cbClsExtra = 0;
//    wc.cbWndExtra = 0;
//    wc.hInstance = hInstance;
//    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
//    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//    wc.hbrBackground = NULL;
//    wc.lpszMenuName = NULL;
//    wc.lpszClassName = "WinParabolicRealms";
//    RegisterClass(&wc);
//    window = CreateWindow("WinParabolicRealms", "Hi there", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 640, 480, NULL, NULL, hInstance, NULL);
//    deviceContext = GetDC(window);
//    memset(&pfd, 0, sizeof(pfd));
//    pfd.nSize = sizeof(pfd);
//    pfd.nVersion = 1;
//    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
//    pfd.iPixelType = PFD_TYPE_RGBA;
//    pfd.cColorBits = 32;
//    pf = ChoosePixelFormat(deviceContext, &pfd);
//    SetPixelFormat(deviceContext, pf, &pfd);
//    DescribePixelFormat(deviceContext, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
//    ReleaseDC(window, deviceContext);
//    deviceContext = GetDC(window);
//    openGLRenderingContext = wglCreateContext(deviceContext);
//    wglMakeCurrent(deviceContext, openGLRenderingContext);
//    ShowWindow(window, nCmdShow);
//
//    LoadOpenGLFunctions();
//    glEnable(GL_DEPTH_TEST);
//    glDepthFunc(GL_LESS);
//    glGenBuffers(1, &vertexBufferObject);
//    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
//    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);
//    glGenVertexArrays(1, &vertexArrayObject);
//    glBindVertexArray(vertexArrayObject);
//    glEnableVertexAttribArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//    vertexShader = glCreateShader(GL_VERTEX_SHADER);
//    glShaderSource(vertexShader, 1, &vertex_shader, NULL);
//    glCompileShader(vertexShader);
//    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//    glShaderSource(fragmentShader, 1, &fragment_shader, NULL);
//    glCompileShader(fragmentShader);
//    shaderProgram = glCreateProgram();
//    glAttachShader(shaderProgram, fragmentShader);
//    glAttachShader(shaderProgram, vertexShader);
//    glLinkProgram(shaderProgram);
//
//    while (!done && message.message != WM_QUIT) {
//        if(PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
//            TranslateMessage(&message);
//            DispatchMessage(&message);
//        } else {
//            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//            glUseProgram(shaderProgram);
//            glBindVertexArray(vertexArrayObject);
//            glDrawArrays(GL_TRIANGLES, 0, 3);
//            glFlush();
//            SwapBuffers(deviceContext);
//            /*
//            if (active) {
//                if (keys[VK_ESCAPE]) {
//                    done = true;
//                }
////                Render();
//
//                if (perspectiveChanged) {
//                    // Adjust projectionMatrix
//                    // TODO: GLM clone in pure c
//                    perspectiveChanged = FALSE;
//                }
//            }*/
//            // Wait what is this for again?
//            // TODO: Recover your timer stuff.
//            Sleep(50);
//        }
//    }
//
//    wglMakeCurrent(NULL, NULL);
//    ReleaseDC(window, deviceContext);
//    wglDeleteContext(openGLRenderingContext);
//    DestroyWindow(window);
//    return (int)message.wParam;
//}