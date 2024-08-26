#include <windows.h>
#include <gl/gl.h>

#include "engine_platform.h"

#include "win32_engine_renderer.h"

#include "engine_math.h"

#include "engine_renderer.h"
#include "engine_renderer_opengl.h"
#include "engine_renderer_opengl.cpp"


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
        //OutputDebugStringA("OpenGL context created\n");
        
        Frame->Opengl.GLVendorStr = (char *)glGetString(GL_VENDOR);
        Frame->Opengl.GLRendererStr = (char *)glGetString(GL_RENDERER);
        Frame->Opengl.GLVersionStr = (char *)glGetString(GL_VERSION);
#endif
        
#define Win32GetOpenglFunction(Name) Frame->Opengl.Name = (type_##Name *)wglGetProcAddress(#Name)
        
        // Load shader
        Win32GetOpenglFunction(glCreateShader);
        Win32GetOpenglFunction(glShaderSource);
        Win32GetOpenglFunction(glCompileShader);
        Win32GetOpenglFunction(glGetShaderiv);
        Win32GetOpenglFunction(glGetShaderInfoLog);
        
        // Link shader program
        Win32GetOpenglFunction(glCreateProgram);
        Win32GetOpenglFunction(glAttachShader);
        Win32GetOpenglFunction(glLinkProgram);
        Win32GetOpenglFunction(glGetProgramiv);
        Win32GetOpenglFunction(glGetProgramInfoLog);
        
        Win32GetOpenglFunction(glUseProgram);
        
        Win32GetOpenglFunction(glDeleteShader);
        Win32GetOpenglFunction(glDeleteProgram);
        
        // FBO
        Win32GetOpenglFunction(glGenFramebuffers);
        Win32GetOpenglFunction(glBindFramebuffer);
        Win32GetOpenglFunction(glFramebufferTexture2D);
        Win32GetOpenglFunction(glActiveTexture);
        
        // VAO, VBO, EBO
        Win32GetOpenglFunction(glGenVertexArrays);
        Win32GetOpenglFunction(glBindVertexArray);
        Win32GetOpenglFunction(glGenBuffers);
        Win32GetOpenglFunction(glBindBuffer);
        Win32GetOpenglFunction(glBufferData);
        Win32GetOpenglFunction(glEnableVertexAttribArray);
        Win32GetOpenglFunction(glVertexAttribPointer);
        
        // Send variable into shader
        Win32GetOpenglFunction(glGetUniformLocation);
        Win32GetOpenglFunction(glCheckFramebufferStatus);
        Win32GetOpenglFunction(glUniform1i);
        Win32GetOpenglFunction(glUniformMatrix4fv);
        Win32GetOpenglFunction(glUniform3fv);
        Win32GetOpenglFunction(glUniform4fv);
        Win32GetOpenglFunction(glUniform1f);
        
        // Mipmap for texture
        Win32GetOpenglFunction(glGenerateMipmap);
        
        // RBO
        Win32GetOpenglFunction(glGenRenderbuffers);
        Win32GetOpenglFunction(glBindRenderbuffer);
        Win32GetOpenglFunction(glRenderbufferStorage);
        Win32GetOpenglFunction(glFramebufferRenderbuffer);
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
    Frame->ImGuiHandle->WGL = wglGetCurrentDC();
#else
    SwapBuffers(wglGetCurrentDC());
#endif
}

WIN32_LOAD_RENDERER_ENTRY()
{
    Win32InitOpengl(Frame, WindowDC);
}