#include <windows.h>
#include <gl/gl.h>

#include "engine_platform.h"

#include "win32_engine_renderer.h"

#include "engine_intrinsics.h"
#include "engine_math.h"
#include "engine_renderer.h"
#include "engine_renderer_opengl.h"

void Win32ErrorMessageBox(char *Message)
{
    MessageBoxA(0, Message, 0, MB_OK);
}

void
Win32InitOpengl(renderer_frame *Frame, HDC WindowDC)
{
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    HGLRC OpenGLRC = wglCreateContext(WindowDC);
    if(wglMakeCurrent(WindowDC, OpenGLRC))        
    {
#if ENGINE_INTERNAL
        OutputDebugStringA("OpenGL context created\n");
        
        Frame->GLVendorStr = (char *)glGetString(GL_VENDOR);
        Frame->GLRendererStr = (char *)glGetString(GL_RENDERER);
        Frame->GLVersionStr = (char *)glGetString(GL_VERSION);
#endif
        
#define Win32GetOpenGLFunction(Name) Frame->Opengl.Name = (type_##Name *)wglGetProcAddress(#Name)
        
        // Load shader
        Win32GetOpenGLFunction(glCreateShader);
        Win32GetOpenGLFunction(glShaderSource);
        Win32GetOpenGLFunction(glCompileShader);
        Win32GetOpenGLFunction(glGetShaderiv);
        Win32GetOpenGLFunction(glGetShaderInfoLog);
        
        // Link shader program
        Win32GetOpenGLFunction(glCreateProgram);
        Win32GetOpenGLFunction(glAttachShader);
        Win32GetOpenGLFunction(glLinkProgram);
        Win32GetOpenGLFunction(glGetProgramiv);
        Win32GetOpenGLFunction(glGetProgramInfoLog);
        
        Win32GetOpenGLFunction(glUseProgram);
        
        Win32GetOpenGLFunction(glDeleteShader);
        Win32GetOpenGLFunction(glDeleteProgram);
    }
    else
    {
        Win32ErrorMessageBox("Can't make current wgl context");
        InvalidCodePath;
    }
    
    // NOTE(ezexff): OpenGL version in window title
    {
        char *TitlePrefix = "OPENGL VERSION: ";
        char *TitleSuffix = (char *)glGetString(GL_VERSION);
        char Title[128];
        for(int Index = 0;
            Index < sizeof(Title);
            Index++)
        {
            if(*TitlePrefix != 0)
            {
                Title[Index] = *TitlePrefix;
                TitlePrefix++;
            }
            else
            {
                Title[Index] = *TitleSuffix;
                if(*TitleSuffix == 0)
                {
                    break;
                }
                TitleSuffix++;
            }
        }
        SetWindowTextA(WindowFromDC(WindowDC), Title);
    }
    
    OpenglInit(Frame);
}


RENDERER_BEGIN_FRAME(Win32BeginFrame)
{
    OpenglBeginFrame(Frame);
}

RENDERER_END_FRAME(Win32EndFrame)
{
    OpenglEndFrame(Frame);
    
#if ENGINE_INTERNAL
    Frame->ImGuiHandle.WGL = wglGetCurrentDC();
#else
    SwapBuffers(wglGetCurrentDC());
#endif
}

WIN32_LOAD_RENDERER_ENTRY()
{
    Win32InitOpengl(Frame, WindowDC);
}