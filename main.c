#include <windows.h>
#include <GL/gl.h>
#include <GL/glcorearb.h>

#define CLASS_NAME "NVWorldEdit"
#define WINDOW_TITLE "NVWorldEdit v0.0.1subalpha"
#define WINDOW_STYLE WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN

#define FILE_EXIT 100

BOOL done = FALSE;
HWND window = NULL;
HMENU menubar = NULL;
HMENU fileMenu = NULL;

LONG WINAPI WindowMessageHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    static PAINTSTRUCT paintStruct;
    switch (message) {
        case WM_CREATE:
            menubar = CreateMenu();
            fileMenu = CreateMenu();
            AppendMenu(fileMenu, MF_STRING, FILE_EXIT, "E&xit");
            AppendMenu(menubar, MF_POPUP, (UINT_PTR)fileMenu, "&File");
            SetMenu(window, menubar);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
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
    MSG message;
    WNDCLASS windowClass;
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
            TranslateMessage(&message);
            DispatchMessage(&message);
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