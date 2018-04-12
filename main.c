#include <windows.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#define CLASS_NAME "NVWorldEdit"
#define WINDOW_TITLE "NVWorldEdit v0.0.1subalpha"
#define WINDOW_STYLE (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define FILE_OPEN_MASTER 100
#define FILE_NEW_MOD 101
#define FILE_OPEN_MOD 102
#define FILE_SAVE_MOD 103
#define FILE_EXIT 150

#define ACCELERATORS_TABLE_LENGTH 5
#define ACCELERATORS_TABLE {\
    { (UINT)FCONTROL | (UINT)FVIRTKEY, 'M', FILE_OPEN_MASTER },\
    { (UINT)FCONTROL | (UINT)FVIRTKEY, 'N', FILE_NEW_MOD },\
    { (UINT)FCONTROL | (UINT)FVIRTKEY, 'O', FILE_OPEN_MOD },\
    { (UINT)FCONTROL | (UINT)FVIRTKEY, 'S', FILE_SAVE_MOD },\
    { (UINT)FCONTROL | (UINT)FVIRTKEY, 'Q', FILE_EXIT }\
}

struct RecordNodeMetadata {
    // Node type. 0 = regular, 1 = group, 2 = sub-record
    CHAR nodeType;
    // Where in the file was it located when parsed?
    UINT32 offset;
    // Has this been changed since reading from disk?
    BOOL dirty;
    // If it has a parent, cache a pointer to it here.
    struct Record *parent;
};

struct RecordHeader {
    UINT8 type[4];
    UINT32 dataSize;
    UINT32 flags;
    UINT32 formID;
    UINT32 versionControlInfo;
};

struct GroupHeader {
    UINT8 type[4];
    UINT32 groupSize;
    UINT8 label[4];
    INT32 groupType;
    UINT32 stamp;
};

struct SubRecordHeader {
    UINT8 subType[4];
    UINT16 dataSize;
};

struct RecordNode {
    struct RecordNodeMetadata recordNodeMetadata;
    struct RecordHeader recordHeader;
    struct GroupHeader groupHeader;
    struct SubRecordHeader subRecordHeader;
    UINT8 *data;
    UINT8 *uncompressedData;
    struct RecordNode *subRecords;
    struct RecordNode *next;
    struct RecordNode *head;
    struct RecordNode *tail;
};

BOOL done = FALSE;
HWND window = NULL;
HMENU menubar = NULL;
HMENU fileMenu = NULL;
HACCEL accelerators = NULL;
MSG message = {};
WNDCLASS windowClass = {};
OPENFILENAME openFileName;
UINT8 fileName[MAX_PATH] = "";
UINT8 *masterFileContents = NULL;
struct Record *masterFileRecords = NULL;
UINT8 *modFileContents = NULL;
struct Record *modFileRecords = NULL;

LONG WINAPI WindowMessageHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT paintStruct;
    switch (message) {
        case WM_CREATE:
            menubar = CreateMenu();
            fileMenu = CreateMenu();
            AppendMenu(fileMenu, MF_STRING, FILE_OPEN_MASTER, "Open &Master...\tCtrl+M");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, FILE_NEW_MOD, "&New Mod...\tCtrl+N");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, FILE_OPEN_MOD, "&Open Mod...\tCtrl+O");
            AppendMenu(fileMenu, MF_SEPARATOR, 0, "-");
            AppendMenu(fileMenu, MF_STRING | MF_GRAYED, FILE_SAVE_MOD, "&Save Mod...\tCtrl+S");
            AppendMenu(fileMenu, MF_SEPARATOR, 0, "-");
            AppendMenu(fileMenu, MF_STRING, FILE_EXIT, "E&xit\tCtrl-Q");
            AppendMenu(menubar, MF_POPUP, (UINT_PTR)fileMenu, "&File");
            SetMenu(window, menubar);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case FILE_OPEN_MASTER:
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
                        HANDLE fileHandle = CreateFile(openFileName.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        LARGE_INTEGER fileSizeInBytes;
                        BOOL getFileSizeResult = GetFileSizeEx(fileHandle, &fileSizeInBytes);
                        masterFileContents = HeapAlloc(GetProcessHeap(), 0, (size_t)fileSizeInBytes.QuadPart);
                        DWORD numberOfBytesActuallyRead = 0;
                        BOOL readFileResult = ReadFile(fileHandle, masterFileContents, (size_t)fileSizeInBytes.QuadPart, &numberOfBytesActuallyRead, NULL);
                        // TODO: Verify that file starts with the right stuff... 'TES4'... 'HEDR'... 'GRUP'... etc. etc.
                        // Locate all top-level records
                        // Find the first GRUP
                        UINT64 location = 0;
                        while (!(masterFileContents[location + 0] == 'G' &&
                                masterFileContents[location + 1] == 'R' &&
                                masterFileContents[location + 2] == 'U' &&
                                masterFileContents[location + 3] == 'P')) { location++; }
                        // TODO: Handle GRUP not found
                        while (location < fileSizeInBytes.QuadPart) {
                            // Is there a GRUP?
                            if (masterFileContents[location + 0] == 'G' &&
                            masterFileContents[location + 1] == 'R' &&
                            masterFileContents[location + 2] == 'U' &&
                            masterFileContents[location + 3] == 'P') {
                                CHAR thisGrupBuffer[1024];
                                for (int i = 0; i < 1024; i++) {
                                    thisGrupBuffer[i] = masterFileContents[location + i];
                                }
                                // Read in size.
                                UINT64 size = masterFileContents[location + 4] +
                                              (masterFileContents[location + 5] << 8) +
                                              (masterFileContents[location + 6] << 16) +
                                              (masterFileContents[location + 7] << 24);
                                UINT64 nextGrup = location + size;
                                CHAR nextGrupBuffer[1024];
                                for (int i = 0; i < 1024; i++) {
                                    nextGrupBuffer[i] = masterFileContents[nextGrup + i];
                                }
                                int jhsdjkfhdjkh = 2343;
                                location = nextGrup;
                            } else {
                                // Unknown top-level data!
                                int sdfdfkjkdf = 2343;
                            }
                        }
                        int asdfasd = 23423;


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
                        HeapFree(GetProcessHeap(), 0, masterFileContents);
                    }

                    EnableMenuItem(fileMenu, FILE_NEW_MOD, MF_ENABLED);
                    EnableMenuItem(fileMenu, FILE_OPEN_MOD, MF_ENABLED);
                    break;
                case FILE_NEW_MOD:
                    MessageBox(NULL, "New mod...", "", MB_OK);
                    break;
                case FILE_OPEN_MOD:
                    MessageBox(NULL, "Open mod...", "", MB_OK);
                    break;
                case FILE_SAVE_MOD:
                    MessageBox(NULL, "Save mod...", "", MB_OK);
                    break;
                case FILE_EXIT:
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
    ACCEL acceleratorsTable[] = ACCELERATORS_TABLE;
    accelerators = CreateAcceleratorTable(acceleratorsTable, ACCELERATORS_TABLE_LENGTH);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = (WNDPROC)WindowMessageHandler;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = CLASS_NAME;
    RegisterClass(&windowClass);
    window = CreateWindow(CLASS_NAME, WINDOW_TITLE, WINDOW_STYLE, 0, 0, 640, 480, NULL, NULL, instance, NULL);
    ShowWindow(window, commandShow);

    while (!done && message.message != WM_QUIT) {
        if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
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