#include <windows.h>
#include <gl/gl.h>

#include "engine_platform.h"

#include "win32_engine_renderer.h"

#include "engine_math.h"

#include "engine_renderer.h"
#include "engine_renderer_opengl.h"
#include "engine_renderer_opengl.cpp"



#define GL_SHADING_LANGUAGE_VERSION 0x8B8C

// NOTE(casey): Windows-specific
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

typedef HGLRC WINAPI wgl_create_context_attribts_arb(HDC hDC, HGLRC hShareContext,
                                                     const int *attribList);

//typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
//global wgl_swap_interval_ext *wglSwapInterval;




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
        Frame->Opengl.GLVendorStr = (char *)glGetString(GL_VENDOR);
        Frame->Opengl.GLRendererStr = (char *)glGetString(GL_RENDERER);
        Frame->Opengl.GLVersionStr = (char *)glGetString(GL_VERSION);
        
        Frame->Opengl.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        Frame->Opengl.Extensions = (char *)glGetString(GL_EXTENSIONS);
        
        OutputDebugStringA(Frame->Opengl.ShadingLanguageVersion);
        OutputDebugStringA("\n");
        OutputDebugStringA(Frame->Opengl.Extensions);
        
        char *At = Frame->Opengl.Extensions;
        while(*At)
        {
            while(IsWhitespace(*At)) {++At;}
            char *End = At;
            while(*End && !IsWhitespace(*End)) {++End;}
            
            umm Count = End - At;        
            
            At = End;
        }
#endif
        
        /* 
                wgl_create_context_attribts_arb *wglCreateContextAttribsARB =
                (wgl_create_context_attribts_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
                if(wglCreateContextAttribsARB)
                {
                    // NOTE(casey): This is a modern version of OpenGL
                    int Attribs[] =
                    {
                        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
                        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
                        WGL_CONTEXT_FLAGS_ARB, 0
        #if ENGINE_INTERNAL
                        |WGL_CONTEXT_DEBUG_BIT_ARB
        #endif
                        ,
                        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                        0,
                    };
                    
                    HGLRC ShareContext = 0;
                    HGLRC ModernGLRC = wglCreateContextAttribsARB(WindowDC, ShareContext, Attribs);
                    if(ModernGLRC)
                    {
                        if(wglMakeCurrent(WindowDC, ModernGLRC))
                        {
                            wglDeleteContext(OpenGLRC);
                            OpenGLRC = ModernGLRC;
                        }
                    }
                }
                else
                {
                    Win32ErrorMessageBox("This is an antiquated version of OpenGL");
                    InvalidCodePath;
                }
         */
        
        /* 
                wglSwapInterval = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
                if(wglSwapInterval)
                {
                    wglSwapInterval(1);
                }
         */
        
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
        Win32GetOpenglFunction(glUniform1iv);
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
        
        // Instancing
        Win32GetOpenglFunction(glDrawArraysInstanced);
        Win32GetOpenglFunction(glTexImage3D);
        Win32GetOpenglFunction(glTexSubImage3D);
        
        //Win32GetOpenglFunction(glGetIntegerv);
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
#if ENGINE_IMGUI
    Frame->ImGuiHandle->WGL = wglGetCurrentDC();
#else
    BEGIN_BLOCK("SwapBuffers");
    SwapBuffers(wglGetCurrentDC());
    END_BLOCK();
#endif
#else
    SwapBuffers(wglGetCurrentDC());
#endif
}

WIN32_LOAD_RENDERER_ENTRY()
{
    Win32InitOpengl(Frame, WindowDC);
}