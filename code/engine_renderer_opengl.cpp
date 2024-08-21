void
OpenglCompileShader(opengl *Opengl, GLuint Type, opengl_shader *Shader)
{
    if(Shader->ID != 0)
    {
        Opengl->glDeleteShader(Shader->ID);
    }
    
    Shader->ID = Opengl->glCreateShader(Type);
    u8 *ShaderText = Shader->Text;
    Opengl->glShaderSource(Shader->ID, 1, &(GLchar *)ShaderText, NULL);
    Opengl->glCompileShader(Shader->ID);
    
    char *TypeStr = (Type == GL_VERTEX_SHADER) ? "vert" : "frag";
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetShaderiv(Shader->ID, GL_COMPILE_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetShaderInfoLog(Shader->ID, 2000, NULL, LogInfo);
        Opengl->glDeleteShader(Shader->ID);
        Shader->ID = 0;
        //Log->Add("[opengl]: %s shader compilaton error (info below):\n%s", TypeStr, LogInfo);
        InvalidCodePath;
    }
}

void
OpenglLinkProgram(opengl *Opengl, opengl_program *Program,
                  opengl_shader *VertShader, opengl_shader *FragShader)
{// TODO(ezexff): Поменять порядок параметров - последний должен быть аутпут, т.е. Program
    if(Program->ID != 0)
    {
        Opengl->glDeleteProgram(Program->ID);
    }
    
    Program->ID = Opengl->glCreateProgram();
    
    if(VertShader->ID != 0)
    {
        Opengl->glAttachShader(Program->ID, VertShader->ID);
    }
    else
    {
        //Log->Add("[program error]: vert shader null\n");
        InvalidCodePath;
    }
    
    if(FragShader->ID != 0)
    {
        Opengl->glAttachShader(Program->ID, FragShader->ID);
    }
    else
    {
        //Log->Add("[program error]: frag shader null\n");
        InvalidCodePath;
    }
    
    Opengl->glLinkProgram(Program->ID);
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetProgramiv(Program->ID, GL_LINK_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetProgramInfoLog(Program->ID, 2000, NULL, LogInfo);
        //Log->Add("[program error]: program linking error (info below):\n%s", LogInfo);
        InvalidCodePath;
    }
}

/*void
OpenglFillTerrainVBO(renderer_frame *Frame, u32 PositionsCount, v3 *Positions, u32 IndicesCount, u32 *Indices)
{
    //sfsfsfs;
}*/
void
OpenglDrawLine(v3 P1, v3 P2, v4 Color, r32 LineWidth)
{
    r32 VertPositions[] = 
    {
        P1.x, P1.y, P1.z, // 0
        P2.x, P2.y, P2.z, // 1
    };
    
    r32 VertColors[] = 
    {
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
    };
    
    glLineWidth(LineWidth);
    glEnable(GL_LINE_SMOOTH);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_LINES, 0, 2);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
    
    glPointSize(5);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(P1.x, P1.y, P1.z);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(P2.x, P2.y, P2.z);
    glEnd();
}

/* 
void
OpenglDrawTerrainChunk(u32 PositionsCount, v3 *Positions, u32 IndicesCount, u32 *Indices)
{
    glColor3f(0.0f, 1.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, Positions);
    //glColorPointer(3, GL_FLOAT, 0, VertColors);
    //glDrawElements(GL_TRIANGLES, IndicesCount, GL_UNSIGNED_INT, Indices);
    glDrawElements(GL_TRIANGLES, IndicesCount, GL_UNSIGNED_INT, Indices);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    //glDisableClientState(GL_COLOR_ARRAY);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(1.0f, 1.0f, 1.0f);
}
 */

void
OpenglDrawCube(v3 P, v3 Dim, v3 Color)
{
    r32 MinX = P.x;
    r32 MaxX = P.x + Dim.x;
    r32 MinY = P.y;
    r32 MaxY = P.y + Dim.y;
    r32 MinZ = P.z;
    r32 MaxZ = P.z + Dim.z;
    
    r32 VertPositions[] =
    {
        // front
        MinX, MinY, MaxZ, // 0
        MaxX, MinY, MaxZ, // 1
        MaxX, MaxY, MaxZ, // 2
        MinX, MaxY, MaxZ, // 3
        // back
        MinX, MaxY, MinZ, // 4
        MaxX, MaxY, MinZ, // 5
        MaxX, MinY, MinZ, // 6
        MinX, MinY, MinZ, // 7
        // right
        MaxX, MinY, MaxZ, // 8, 1
        MaxX, MinY, MinZ, // 9, 6
        MaxX, MaxY, MinZ, // 10, 5
        MaxX, MaxY, MaxZ, // 11, 2
        // left
        MinX, MinY, MinZ, // 12, 7
        MinX, MinY, MaxZ, // 13, 0
        MinX, MaxY, MaxZ, // 14, 3
        MinX, MaxY, MinZ, // 15, 4
        // top
        MinX, MaxY, MinZ, // 16
        MaxX, MaxY, MinZ, // 17
        MaxX, MaxY, MaxZ, // 18
        MinX, MaxY, MaxZ, // 19
        // bot
        MinX, MinY, MinZ, // 20
        MaxX, MinY, MinZ, // 21
        MaxX, MinY, MaxZ, // 22
        MinX, MinY, MaxZ, // 23
    };
    
#if 1
    r32 VertColors[] =
    {
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
        Color.x, Color.y, Color.z,
    };
#else
    // NOTE(ezexff): Test
    r32 VertColors[] =
    {
        1,   0, 0,   1,   0, 0,   1,   0, 0,   1,   0, 0,   // red
        0,   1, 0,   0,   1, 0,   0,   1, 0,   0,   1, 0,   // green
        0,   0, 1,   0,   0, 1,   0,   0, 1,   0,   0, 1,   // blue
        0.5, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, // purple
        1,   1, 0,   1,   1, 0,   1,   1, 0,   1,   1, 0,   // yellow
        1,   1, 1,   1,   1, 1,   1,   1, 1,   1,   1, 1    // white
    };
#endif
    
    u32 Indices[] =
    {
        0,  1,  2,  3,  // front - red
        4,  5,  6,  7,  // back - green
        8,  9,  10, 11, // right - blue
        12, 13, 14, 15, // left - purple
        16, 17, 18, 19, // top - yellow
        20, 21, 22, 23  // bot - white
    };
    
    s32 IndicesCount = ArrayCount(Indices);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawElements(GL_QUADS, IndicesCount, GL_UNSIGNED_INT, Indices);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void
OpenglDrawCubeOutline(v3 P, v3 Dim, v3 Color, r32 LineWidth)
{
    r32 MinX = P.x;
    r32 MaxX = P.x + Dim.x;
    r32 MinY = P.y;
    r32 MaxY = P.y + Dim.y;
    r32 MinZ = P.z;
    r32 MaxZ = P.z + Dim.z;
    
    r32 VertPositions[] = 
    {
        // bot
        MinX, MinY, MinZ, // 0
        MaxX, MinY, MinZ, // 1
        MaxX, MaxY, MinZ, // 2
        MinX, MaxY, MinZ, // 3
        // top
        MinX, MinY, MaxZ, // 4
        MaxX, MinY, MaxZ, // 5
        MaxX, MaxY, MaxZ, // 6
        MinX, MaxY, MaxZ, // 7
    };
    
    r32 VertColors[] = 
    {
        // bot
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
        // top
        Color.x, Color.y, Color.z, // 4
        Color.x, Color.y, Color.z, // 5
        Color.x, Color.y, Color.z, // 6
        Color.x, Color.y, Color.z  // 7
    };
    
    u32 Indices[] = 
    {
        0, 1, 1, 2, 2, 3, 3, 0, // bot
        4, 5, 5, 6, 6, 7, 7, 4, // top
        0, 4, 3, 7, 1, 5, 2, 6, // side
    };
    
    s32 IndicesCount = ArrayCount(Indices);
    
    glLineWidth(LineWidth);
    glEnable(GL_LINE_SMOOTH);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawElements(GL_LINES, IndicesCount, GL_UNSIGNED_INT, Indices);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
}

void
OpenglDrawRectOnScreen(rectangle2 R, v3 Color)
{
    r32 VertPositions[] = 
    {
        R.Min.x, R.Min.y, // 0
        R.Max.x, R.Min.y, // 1
        R.Max.x, R.Max.y, // 2
        R.Min.x, R.Max.y, // 3
    };
    
    r32 VertColors[] = {
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void
OpenglDrawRectOnGround(GLenum Mode, rectangle2 R, r32 Z, v3 Color, r32 LineWidth = 1.0f)
{
    r32 VertPositions[] = {
        // ground
        R.Min.x, R.Min.y, Z, // 0
        R.Max.x, R.Min.y, Z, // 1
        R.Max.x, R.Max.y, Z, // 2
        R.Min.x, R.Max.y, Z, // 3
    };
    
    r32 VertColors[] = {
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
    };
    
    if(Mode == GL_LINE_LOOP)
    {
        glLineWidth(LineWidth);
        glEnable(GL_LINE_SMOOTH);
    }
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(Mode, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    if(Mode == GL_LINE_LOOP)
    {
        glLineWidth(1);
        glDisable(GL_LINE_SMOOTH);
    }
}

void
OpenglDrawBitmapOnGround(opengl *Opengl, loaded_bitmap *Bitmap, v2 P, v2 Dim, r32 Repeat)
{
    if(Bitmap->OpenglID == 0)
    {
        glGenTextures(1, &Bitmap->OpenglID);
        glBindTexture(GL_TEXTURE_2D, Bitmap->OpenglID);
        
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        //texture to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0,
                     Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
        Opengl->glGenerateMipmap(GL_TEXTURE_2D);
    }
    
    r32 MinX = P.x;
    r32 MaxX = P.x + Dim.x;
    r32 MinY = P.y;
    r32 MaxY = P.y + Dim.y;
    r32 MinZ = 0.0f;
    
    r32 VertPositions[] =
    {
        // bot
        MinX, MinY, MinZ, // 0
        MaxX, MinY, MinZ, // 1
        MaxX, MaxY, MinZ, // 2
        MinX, MaxY, MinZ  // 3
    };
    
    r32 TexRectangle[] =
    {
        0,      Repeat, // 0
        Repeat, Repeat, // 1
        Repeat, 0,      // 2
        0,      0       // 3
    };
    
    glEnable(GL_TEXTURE_2D);
    
    Opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Bitmap->OpenglID);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glTexCoordPointer(2, GL_FLOAT, 0, TexRectangle);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
}

void
OpenglDrawBitmapOnScreen(opengl *Opengl, loaded_bitmap *Bitmap, v2 P, v2 Dim, v3 Color, r32 Repeat)
{
    if(Bitmap->OpenglID == 0)
    {
        glGenTextures(1, &Bitmap->OpenglID);
        glBindTexture(GL_TEXTURE_2D, Bitmap->OpenglID);
        
#if 0
        // NOTE(ezexff): Ver 1
        
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        //texture to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0,
                     Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
        Opengl->glGenerateMipmap(GL_TEXTURE_2D);
#else
        // NOTE(ezexff): Ver2
        /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);*/
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0,
                     Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
#endif
    }
    
    r32 MinX = P.x;
    r32 MaxX = P.x + Dim.x;
    r32 MinY = P.y;
    r32 MaxY = P.y + Dim.y;
    r32 VertPositions[] =
    {
        // bot
        MinX, MinY, // 0
        MaxX, MinY, // 1
        MaxX, MaxY, // 2
        MinX, MaxY  // 3
    };
    
    r32 TexRectangle[] =
    {
        0, 0,           // 0
        Repeat, 0,      // 1
        Repeat, Repeat, // 2
        0, Repeat       // 3
    };
    
    r32 Colors[] =
    {
        Color.x, Color.y, Color.z, 
        Color.x, Color.y, Color.z, 
        Color.x, Color.y, Color.z, 
        Color.x, Color.y, Color.z
    };
    
    glEnable(GL_TEXTURE_2D);
    
    Opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Bitmap->OpenglID);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VertPositions);
    glTexCoordPointer(2, GL_FLOAT, 0, TexRectangle);
    glColorPointer(3, GL_FLOAT, 0, Colors);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
}

void
OpenglInit(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): For ImGuiPreview window
    glGenTextures(1, &Frame->PreviewTexture);
#endif
    
    // NOTE(ezexff): Init push buffer
    Frame->MaxPushBufferSize = sizeof(Frame->PushBufferMemory);
    Frame->PushBufferBase = Frame->PushBufferMemory;
    
    // NOTE(ezexff): Init camera
    //Frame->Camera.P.z = 10.0f;
    
    // NOTE(ezexff): Init skybox
    {
        r32 SkyboxVertices[] = 
        {
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            
            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };
        
        glGenTextures(1, &Frame->SkyboxTexture);
        
        Opengl->glGenVertexArrays(1, &Frame->SkyboxVAO);
        Opengl->glGenBuffers(1, &Frame->SkyboxVBO);
        
        Opengl->glBindVertexArray(Frame->SkyboxVAO);
        Opengl->glBindBuffer(GL_ARRAY_BUFFER, Frame->SkyboxVBO);
        Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
        Opengl->glEnableVertexAttribArray(0);
        Opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(r32), (void*)0);
    }
    
    // NOTE(ezexff): Init frame textureas and VAO, VBO, EBO
    {
        Opengl->glGenFramebuffers(1, &Frame->FBO);
        glGenTextures(1, &Frame->ColorTexture);
        glGenTextures(1, &Frame->DepthTexture);
        
        u32 VertexCount = 6;
        frame_vertex Vertices[6];
        
        Vertices[0].Pos = {-1.0f, -1.0f, 0.0f};
        Vertices[1].Pos = {1.0f, -1.0f, 0.0f};
        Vertices[2].Pos = {-1.0f,  1.0f, 0.0f};
        Vertices[3].Pos = {1.0f, -1.0f, 0.0f};
        Vertices[4].Pos = {1.0f,  1.0f, 0.0f};
        Vertices[5].Pos = {-1.0f,  1.0f, 0.0f};
        
        Vertices[0].TexCoord  = {0.0f, 0.0f};
        Vertices[1].TexCoord  = {1.0f, 0.0f};
        Vertices[2].TexCoord  = {0.0f, 1.0f};
        Vertices[3].TexCoord  = {1.0f, 0.0f};
        Vertices[4].TexCoord  = {1.0f, 1.0f};
        Vertices[5].TexCoord  = {0.0f, 1.0f};
        
        //u32 Indices[6] = {0, 4, 1, 5, 4, 5};
        
        Opengl->glGenVertexArrays(1, &Frame->VAO);
        Opengl->glGenBuffers(1, &Frame->VBO);
        //Opengl->glGenBuffers(1, &Frame->EBO);
        
        Opengl->glBindVertexArray(Frame->VAO);
        Opengl->glBindBuffer(GL_ARRAY_BUFFER, Frame->VBO);
        
        Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(frame_vertex) * VertexCount, Vertices, GL_STATIC_DRAW);
        
        //Opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Frame->EBO);
        //Opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * VertexCount, Indices, GL_STATIC_DRAW);
        
#if 0
#define GL_OFFSET(x) ((const GLvoid *)(x))
        
        Opengl->glVertexAttribPointer(VERT_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), GL_OFFSET(0));
        Opengl->glEnableVertexAttribArray(VERT_POSITION);
        
        Opengl->glVertexAttribPointer(VERT_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), GL_OFFSET(sizeof(v3)));
        Opengl->glEnableVertexAttribArray(VERT_TEXCOORD);
        
#else
        // Positions
        Opengl->glEnableVertexAttribArray(0);
        Opengl->glVertexAttribPointer(VERT_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)0);
        // Texture coords
        Opengl->glEnableVertexAttribArray(1);
        Opengl->glVertexAttribPointer(VERT_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)offsetof(frame_vertex, TexCoord));
#endif
        
        Opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        Opengl->glBindVertexArray(0);
    }
    
    // NOTE(ezexff): Init shadowmap
    {
        u32 ShadowMapSizeMultiplyer = 8;
        Frame->ShadowMapDim.x = 1920 * ShadowMapSizeMultiplyer;
        Frame->ShadowMapDim.y = 1080 * ShadowMapSizeMultiplyer;
        
        // NOTE(ezexff):  FBO
        Opengl->glGenFramebuffers(1, &Frame->ShadowMapFBO);
        glGenTextures(1, &Frame->ShadowMap);
        
        // NOTE(ezexff): VBO
        glBindTexture(GL_TEXTURE_2D, Frame->ShadowMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Frame->ShadowMapDim.x, Frame->ShadowMapDim.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if 1
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
        // TODO(me): test
#define GL_CLAMP_TO_BORDER 0x812D
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float BorderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, BorderColor);
#endif
        
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->ShadowMapFBO);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Frame->ShadowMap, 0);
        if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    // NOTE(ezexff): Init water
    {
        Frame->WaterReflectionDim.x = 1920 / 4;
        Frame->WaterReflectionDim.y = 1080 / 4;
        
        Frame->WaterRefractionDim.x = 1920 / 2;
        Frame->WaterRefractionDim.y = 1080 / 2;
        
        // NOTE(ezexff): Reflection
        Opengl->glGenFramebuffers(1, &Frame->WaterReflectionFBO);
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->WaterReflectionFBO);
        
        // Create Texture Attachment
        glGenTextures(1, &Frame->WaterReflectionColorTexture);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterReflectionColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (u32)Frame->WaterReflectionDim.x, (u32)Frame->WaterReflectionDim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->WaterReflectionColorTexture, 0);
        if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
        
        // Create Render Buffer Object
        Opengl->glGenRenderbuffers(1, &Frame->WaterReflectionDepthRBO);
        Opengl->glBindRenderbuffer(GL_RENDERBUFFER, Frame->WaterReflectionDepthRBO);
        Opengl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (u32)Frame->WaterReflectionDim.x, (u32)Frame->WaterReflectionDim.y);
        Opengl->glBindRenderbuffer(GL_RENDERBUFFER, 0);
        Opengl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Frame->WaterReflectionDepthRBO);
        if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
        
        // NOTE(ezexff): Refraction
        Opengl->glGenFramebuffers(1, &Frame->WaterRefractionFBO);
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->WaterRefractionFBO);
        
        // Create Texture Attachment
        glGenTextures(1, &Frame->WaterRefractionColorTexture);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterRefractionColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (u32)Frame->WaterRefractionDim.x, (u32)Frame->WaterRefractionDim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->WaterRefractionColorTexture, 0);
        
        // Create Depth Texture Attachment
        glGenTextures(1, &Frame->WaterRefractionDepthTexture);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterRefractionDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, (u32)Frame->WaterRefractionDim.x, (u32)Frame->WaterRefractionDim.y, 0, GL_DEPTH_COMPONENT,
                     GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Frame->WaterRefractionDepthTexture, 0);
        if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
    }
}

void
OpenglBeginFrame(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    
#if ENGINE_INTERNAL
    if(!Frame->IsOpenglImGuiInitialized)
    {
        imgui *ImGuiHandle = Frame->ImGuiHandle;
        ImGui::SetCurrentContext(ImGuiHandle->ContextImGui);
        ImPlot::SetCurrentContext(ImGuiHandle->ContextImPlot);
        ImGui::SetAllocatorFunctions(ImGuiHandle->AllocFunc, ImGuiHandle->FreeFunc, ImGuiHandle->UserData);
        Log = &Frame->ImGuiHandle->Log;
        GlobalDebugTable = Frame->DebugTable;
        
        Frame->IsOpenglImGuiInitialized = true;
    }
#endif
    
    /*
    // сделаем текстуру активной
    glBindTexture(GL_TEXTURE_2D, Frame->RenderTexture);
    
    // установим параметры фильтрации текстуры - линейная фильтрация
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // установим параметры "оборачивания" текстуры - отсутствие оборачивания
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // создаем "пустую" текстуру
    GLint internalFormat = GL_RGBA8;
    GLenum format = GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Frame->Dim.x, Frame->Dim.y, 0, format, GL_UNSIGNED_BYTE, NULL);
    
    // делаем созданный FBO текущим
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->FBO);
    
    // присоединяем созданные текстуры к текущему FBO
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->RenderTexture, 0);
    //Opengl->glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, Frame->RenderTexture, 0);
    //Opengl->glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  depthTexture, 0);
    
    // проверим текущий FBO на корректность
    GLenum fboStatus;
    if((fboStatus = Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
    {
        //Log->Add("glCheckFramebufferStatus error 0x%X\n", fboStatus);
        InvalidCodePath;
        //LOG_ERROR("glCheckFramebufferStatus error 0x%X\n", fboStatus);
        //return false;
    }
    
    // делаем активным дефолтный FBO
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
*/
    
    
    //glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    
    // NOTE(ezexff): Init FBO
    
    /*glBindTexture(GL_TEXTURE_2D, Frame->RenderTexture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Buffer->Width, Buffer->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame->Dim.x, Frame->Dim.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->RenderTexture, 0);*/
    
    
#if 1
    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Frame->ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Frame->Dim.x, Frame->Dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    //  Create Depth Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Frame->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Frame->Dim.x, Frame->Dim.y, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Attach textures to FBO
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->FBO);
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->ColorTexture, 0);
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Frame->DepthTexture, 0);
    if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        InvalidCodePath;
    }
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
#else
    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Frame->RenderTexture);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Frame->Dim.x, Frame->Dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame->Dim.x, Frame->Dim.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Frame->RenderTexture, 0);
    
    //  Create Depth Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Frame->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Frame->Dim.x, Frame->Dim.y, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Frame->DepthTexture, 0);
#endif
    
    // NOTE(ezexff): Texture for preview window
#if ENGINE_INTERNAL
    loaded_bitmap Preview = Frame->Preview;
    if(Preview.Memory)
    {
        glBindTexture(GL_TEXTURE_2D, Frame->PreviewTexture);
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // filtering
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // texture to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Preview.Width, Preview.Height, 0,
                     Preview.BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Preview.Memory);
        //Opengl->glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif
    
    // NOTE(ezexff): Textures for skybox
    if(Frame->InitializeSkyboxTexture)
    {
        u32 LoadedIndex = 0;
        for(u32 Index = 0;
            Index < ArrayCount(Frame->Skybox);
            Index++)
        {
            loaded_bitmap Bitmap = Frame->Skybox[Index];
            if(Bitmap.Memory)
            {
                LoadedIndex++;
            }
        }
        
        if(LoadedIndex == ArrayCount(Frame->Skybox))
        {
            glBindTexture(GL_TEXTURE_CUBE_MAP, Frame->SkyboxTexture);
            for(u32 Index = 0;
                Index < ArrayCount(Frame->Skybox);
                Index++)
            {
                loaded_bitmap Bitmap = Frame->Skybox[Index];
                if(Bitmap.Memory)
                {
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
                    
                    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Index, 0, GL_RGB, Bitmap.Width, Bitmap.Height, 0, 
                                 Bitmap.BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap.Memory);
                }
            }
            Frame->InitializeSkyboxTexture = false;
        }
    }
    
    // NOTE(ezexff): Init textures
    {
        if(Frame->TerrainTexture)
        {
            if(!Frame->TerrainTexture->OpenglID && Frame->TerrainTexture->Memory)
            {
                glGenTextures(1, &Frame->TerrainTexture->OpenglID);
                glBindTexture(GL_TEXTURE_2D, Frame->TerrainTexture->OpenglID);
                // Texture wrapping or filtering options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame->TerrainTexture->Width, Frame->TerrainTexture->Height, 0, Frame->TerrainTexture->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Frame->TerrainTexture->Memory);
                Opengl->glGenerateMipmap(GL_TEXTURE_2D);
                
            }
        }
        if(Frame->WaterDUDVTexture)
        {
            if(!Frame->WaterDUDVTexture->OpenglID && Frame->WaterDUDVTexture->Memory)
            {
                glGenTextures(1, &Frame->WaterDUDVTexture->OpenglID);
                glBindTexture(GL_TEXTURE_2D, Frame->WaterDUDVTexture->OpenglID);
                // Texture wrapping or filtering options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame->WaterDUDVTexture->Width, Frame->WaterDUDVTexture->Height, 0, Frame->WaterDUDVTexture->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Frame->WaterDUDVTexture->Memory);
                Opengl->glGenerateMipmap(GL_TEXTURE_2D);
                
            }
        }
        if(Frame->WaterNormalMapTexture)
        {
            if(!Frame->WaterNormalMapTexture->OpenglID && Frame->WaterNormalMapTexture->Memory)
            {
                glGenTextures(1, &Frame->WaterNormalMapTexture->OpenglID);
                glBindTexture(GL_TEXTURE_2D, Frame->WaterNormalMapTexture->OpenglID);
                // Texture wrapping or filtering options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Frame->WaterNormalMapTexture->Width, Frame->WaterNormalMapTexture->Height, 0, Frame->WaterNormalMapTexture->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Frame->WaterNormalMapTexture->Memory);
                Opengl->glGenerateMipmap(GL_TEXTURE_2D);
                
            }
        }
    }
    
    // NOTE(ezexff): Compile shaders
    if(Frame->CompileShaders)
    {
        // Frame
        OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->Vert);
        OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->Frag);
        OpenglLinkProgram(Opengl, &Frame->Program, &Frame->Vert, &Frame->Frag);
#if ENGINE_INTERNAL
        Log->Add("[opengl] frame shader program compiled\n");
#endif
        
        // Default
        OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->DefaultVert);
        OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->DefaultFrag);
        OpenglLinkProgram(Opengl, &Frame->DefaultProg, &Frame->DefaultVert, &Frame->DefaultFrag);
#if ENGINE_INTERNAL
        Log->Add("[opengl] default shader program compiled\n");
#endif
        // Skybox
        OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->SkyboxVert);
        OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->SkyboxFrag);
        OpenglLinkProgram(Opengl, &Frame->SkyboxProgram, &Frame->SkyboxVert, &Frame->SkyboxFrag);
#if ENGINE_INTERNAL
        Log->Add("[opengl] skybox shader program compiled\n");
#endif
        
        // ShadowMap
        OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->ShadowMapVert);
        OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->ShadowMapFrag);
        OpenglLinkProgram(Opengl, &Frame->ShadowMapProg, &Frame->ShadowMapVert, &Frame->ShadowMapFrag);
#if ENGINE_INTERNAL
        Log->Add("[opengl] shadowmap shader program compiled\n");
#endif
        
        // Water
        OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->WaterVert);
        OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->WaterFrag);
        OpenglLinkProgram(Opengl, &Frame->WaterProg, &Frame->WaterVert, &Frame->WaterFrag);
#if ENGINE_INTERNAL
        Log->Add("[opengl] water shader program compiled\n");
#endif
        
        Frame->CompileShaders = false;
    }
    
    // NOTE(ezexff): Init terrain VBO
    /* 
        if(!Frame->IsTerrainVBOInitialized)
        {
            Opengl->glGenVertexArrays(1, &Frame->TerrainVAO);
            Opengl->glGenBuffers(1, &Frame->TerrainVBO);
            Opengl->glGenBuffers(1, &Frame->TerrainEBO);
            
            Frame->IsTerrainVBOInitialized = true;
        }
     */
}

void
ShadowMapUniforms(renderer_frame *Frame, u32 ShaderProgramID)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    // NOTE(ezexff): Send shadow matrices and parameters into shader
    // Proj
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-Frame->ShadowMapSize, Frame->ShadowMapSize, -Frame->ShadowMapSize, Frame->ShadowMapSize,
            Frame->ShadowMapNearPlane, Frame->ShadowMapFarPlane);
    r32 MatProjShadows[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProjShadows);
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, MatProjShadows);
    
    // View
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-Frame->ShadowMapCameraPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-Frame->ShadowMapCameraYaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Frame->ShadowMapCameraPos.x, -Frame->ShadowMapCameraPos.y, -Frame->ShadowMapCameraPos.z);
    r32 MatViewShadows[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatViewShadows);
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, MatViewShadows);
    
    // Model
    m4x4 MatModel = Identity();
    //MatModel = Translate(Frame->WorldOrigin);
    MatModel = Transpose(MatModel); // opengl to glsl format
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
}

void
DefaultUniforms(renderer_frame *Frame, u32 ShaderProgramID)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    // NOTE(ezexff): Send transform matrices into shader
    {
        // Proj
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-Frame->AspectRatio * Renderer->FOV, Frame->AspectRatio * Renderer->FOV, -Renderer->FOV, Renderer->FOV, Renderer->FOV * 2, 1000);
        r32 MatProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, MatProj);
        
        // View
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
        glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
        glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
        glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        //glTranslatef(-Frame->WorldOrigin.x, -Frame->WorldOrigin.y, -Frame->CameraZ);
        r32 MatView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, MatView);
        
        // Model
        m4x4 MatModel = Identity();
        //MatModel = Translate(Frame->WorldOrigin);
        MatModel = Transpose(MatModel); // opengl to glsl format
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
        
        // Proj ShadowMap
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-Frame->ShadowMapSize, Frame->ShadowMapSize, -Frame->ShadowMapSize, Frame->ShadowMapSize,
                Frame->ShadowMapNearPlane, Frame->ShadowMapFarPlane);
        r32 MatProjShadows[16]; 
        glGetFloatv(GL_PROJECTION_MATRIX, MatProjShadows);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProjShadowMap"), 1, GL_FALSE, MatProjShadows);
        
        // View ShadowMap
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-Frame->ShadowMapCameraPitch, 1.0f, 0.0f, 0.0f);
        glRotatef(-Frame->ShadowMapCameraYaw, 0.0f, 0.0f, 1.0f);
        glTranslatef(-Frame->ShadowMapCameraPos.x, -Frame->ShadowMapCameraPos.y, -Frame->ShadowMapCameraPos.z);
        r32 MatViewShadows[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatViewShadows);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uViewShadowMap"), 1, GL_FALSE, MatViewShadows);
    }
    
    // NOTE(ezexff): Send camera pos into shader for light calc
    {
        // TODO(ezexff): Позиция солнца должна быть рассчитана относительно позиции в мировом пространстве
        //Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, SunP.E);
        //Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, Frame->WorldOrigin.E);
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, Frame->TestSun2P.E);
    }
    
    // NOTE(ezexff): Send material into shader
    {
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.AmbientColor"), 1, Frame->TerrainMaterial.Ambient.E);
        
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.DiffuseColor"), 1, Frame->TerrainMaterial.Diffuse.E);
        
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.SpecularColor"), 1, Frame->TerrainMaterial.Specular.E);
    }
    
    /* 
        if(Mesh->Material.WithTexture)
        {
            glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
            glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
            glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
        }
     */
    // NOTE(ezexff): Texture
    {
        Opengl->glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, Frame->TerrainTexture->OpenglID);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uSampler"), 0);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uSamplerSpecularExponent"), 0);
    }
    
    // NOTE(ezexff): Send light sources into shader
    {
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.Color"), 1, Frame->DirLight.Base.Color.E);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.AmbientIntensity"), Frame->DirLight.Base.AmbientIntensity);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.DiffuseIntensity"), Frame->DirLight.Base.DiffuseIntensity);
        v3 DirLightDirection = Frame->DirLight.WorldDirection;
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Direction"), 1, DirLightDirection.E);
    }
    
    // NOTE(ezexff): Send shadowmap and bias into shader
    {
        Opengl->glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Frame->ShadowMap);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uShadowMap"), 1);
        
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uBias"), Frame->ShadowMapBias);
    }
}

void
WaterUniforms(renderer_frame *Frame, u32 ShaderProgramID)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    // NOTE(ezexff): Send transform matrices into shader
    {
        // Proj
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-Frame->AspectRatio * Renderer->FOV, Frame->AspectRatio * Renderer->FOV, -Renderer->FOV, Renderer->FOV, Renderer->FOV * 2, 1000);
        r32 MatProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, MatProj);
        
        // View
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
        glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
        glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
        glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        //glTranslatef(-Frame->WorldOrigin.x, -Frame->WorldOrigin.y, -Frame->CameraZ);
        r32 MatView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, MatView);
        
        // Model
        m4x4 MatModel = Identity();
        //MatModel = Translate(Frame->TestWaterRelP);
        MatModel = Transpose(MatModel); // opengl to glsl format
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
    }
    
    // NOTE(ezexff): Vars
    {
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraP"), 1, Camera->P.E);
        //v3 SunP = V3(-10, 10, 10);
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uSunP"), 1, Frame->TestSunRelP.E);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uTiling"), Frame->WaterTiling);
        
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uMoveFactor"), Frame->WaterMoveFactor);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uWaveStrength"), Frame->WaterWaveStrength);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uShineDamper"), Frame->WaterShineDamper);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uReflectivity"), Frame->WaterReflectivity);
        
        // glUniform3fv(glGetUniformLocation(ShaderProgramID, "SunPosition"), 1, Render->DirLight.WorldDirection.E);
        //v3 TestSunPos = V3(-Render->SunPos.x, -Render->SunPos.y, -Render->SunPos.z);
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uLightColor"), 1, Frame->DirLight.Base.Color.E);
        //v3 RelPlayerP = GetRelPos(World, Player->P, Player->TmpZ);
        // glUniform3fv(glGetUniformLocation(ShaderProgramID, "CameraPosition"), 1, Player->Position.E);
    }
    // NOTE(ezexff): Textures
    {
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterReflectionColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uReflectionColorTexture"), 0);
        
        Opengl->glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterRefractionColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uRefractionColorTexture"), 1);
        
        Opengl->glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterDUDVTexture->OpenglID);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uDUDVTexture"), 2);
        
        Opengl->glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterNormalMapTexture->OpenglID);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uNormalMapTexture"), 3);
        
        Opengl->glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, Frame->WaterRefractionDepthTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uRefractionDepthTexture"), 4);
    }
}

void
OpenglDrawWater(renderer_frame *Frame)
{
    TIMED_FUNCTION();
    
    opengl *Opengl = &Frame->Opengl;
    
    Opengl->glUseProgram(Frame->WaterProg.ID);
    WaterUniforms(Frame, Frame->WaterProg.ID);
    
    for(u32 BaseAddress = 0;
        BaseAddress < Frame->PushBufferSize;
        )
    {
        renderer_entry_header *Header = (renderer_entry_header *)
        (Frame->PushBufferBase + BaseAddress);
        BaseAddress += sizeof(*Header);
        
        void *Data = (u8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
            case RendererEntryType_renderer_entry_clear:
            {
                renderer_entry_clear *Entry = (renderer_entry_clear *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_on_ground:
            {
                renderer_entry_rect_on_ground *Entry = (renderer_entry_rect_on_ground *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_outline_on_ground:
            {
                renderer_entry_rect_outline_on_ground *Entry = (renderer_entry_rect_outline_on_ground *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_bitmap_on_ground:
            {
                renderer_entry_bitmap_on_ground *Entry = (renderer_entry_bitmap_on_ground *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_cube:
            {
                renderer_entry_cube *Entry = (renderer_entry_cube *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_cube_outline:
            {
                renderer_entry_cube_outline *Entry = (renderer_entry_cube_outline *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_on_screen:
            {
                renderer_entry_rect_on_screen *Entry = (renderer_entry_rect_on_screen *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_bitmap_on_screen:
            {
                renderer_entry_bitmap_on_screen *Entry = (renderer_entry_bitmap_on_screen *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_terrain_chunk:
            {
                renderer_entry_terrain_chunk *Entry = (renderer_entry_terrain_chunk *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_line:
            {
                renderer_entry_line *Entry = (renderer_entry_line *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_water:
            {
                renderer_entry_water *Entry = (renderer_entry_water *)Data;
                
                //OpenglDrawWater({Entry->P.xy, Entry->P.xy + Entry->Dim});
                rectangle2 R = {Entry->P.xy, Entry->P.xy + Entry->Dim};
                r32 z = 0;
                r32 VertPositions[] = 
                {
                    // ground
                    R.Min.x, R.Min.y, z, // 0
                    R.Max.x, R.Min.y, z, // 1
                    R.Max.x, R.Max.y, z, // 2
                    R.Min.x, R.Max.y, z, // 3
                };
                
                glEnableClientState(GL_VERTEX_ARRAY);
                
                glVertexPointer(3, GL_FLOAT, 0, VertPositions);
                glDrawArrays(GL_QUADS, 0, 4);
                
                glDisableClientState(GL_VERTEX_ARRAY);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            InvalidDefaultCase;
        }
    }
    
    Opengl->glUseProgram(0);
}

void
OpenglDrawSkybox(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    glDepthFunc(GL_LEQUAL);
    Opengl->glUseProgram(Frame->SkyboxProgram.ID);
    
    // NOTE(ezexff): Send transform matrices into shader
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-Frame->AspectRatio * Renderer->FOV, Frame->AspectRatio * Renderer->FOV, -Renderer->FOV, Renderer->FOV, Renderer->FOV * 2, 1000);
        r32 MatProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "Proj"), 1, GL_FALSE, MatProj);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
        glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
        glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
        //glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        r32 MatView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "View"), 1, GL_FALSE, MatView);
        
        m4x4 MatModel = Identity();
        //MatModel = Translate(Camera->P) * XRotation(90) * Scale(200);
        MatModel = XRotation(90) * Scale(200);
        MatModel = Transpose(MatModel); // opengl to glsl format
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "Model"), 1, GL_FALSE, (const GLfloat *)&MatModel);
    }
    
    // NOTE(ezexff): Send texture into shader
    {
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Frame->SkyboxTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Frame->Program.ID, "SkyboxTexture"), 0);
    }
    
    // NOTE(ezexff): Draw
    {
        Opengl->glBindVertexArray(Frame->SkyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        Opengl->glBindVertexArray(0);
    }
    
    Opengl->glUseProgram(0);
    glDepthFunc(GL_LESS);
}

void
OpenglDrawTerrain(renderer_frame *Frame, u32 ShaderProgramID)
{
    TIMED_FUNCTION();
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    // NOTE(ezexff): Terrain
    if(IsSet(Renderer, RendererFlag_Terrain))
    {
        // TODO(ezexff): Test only, delete this
        /*
        #define GL_ARRAY_BUFFER_BINDING 0x8894
        #define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
                glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &Frame->TestDefaultVAO);
                glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &Frame->TestDefaultEBO);
*/
        
        // NOTE(ezexff): Bind VAO
        {
            /*Opengl->glBindVertexArray(Frame->TerrainVAO);
            Opengl->glBindBuffer(GL_ARRAY_BUFFER, Frame->TerrainVBO);
            Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_vertex) * Frame->TerrainVerticesCount, Frame->TerrainVertices, GL_STATIC_DRAW);
            
            Opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Frame->TerrainEBO);
            Opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Frame->TerrainIndicesCount, Frame->TerrainIndices, GL_STATIC_DRAW);
            
            Opengl->glEnableVertexAttribArray(0);
            Opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)0);
            Opengl->glEnableVertexAttribArray(1);
            Opengl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, Normal));
            Opengl->glEnableVertexAttribArray(2);
            Opengl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, TexCoords));
            
            Opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);
            Opengl->glBindVertexArray(0);*/
        }
        
        // отправка плоскости отсечения в шейдер
        //glUniform4fv(glGetUniformLocation(ShaderProg, "CutPlane"), 1, Render->CutPlane.E);
        
        // отправка позиции камеры (игрока) в шейдер
        //v3 RelPlayerP = GetRelPos(World, Player->P, Player->TmpZ);
        //glUniform3fv(glGetUniformLocation(ShaderProg, "gCameraWorldPos"), 1, RelPlayerP.E);
        // glUniform3fv(glGetUniformLocation(ShaderProg, "gCameraWorldPos"), 1, Player->Position.E);
        
        /*if(Frame->IsTerrainInLinePolygonMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        
        // NOTE(ezexff): Draw
        {
            Opengl->glBindVertexArray(Frame->TerrainVAO);
            // (void *)(sizeof(u32) * BaseIndex
            //glDrawElementsBaseVertex(GL_TRIANGLES, Frame->TerrainIndicesCount, GL_UNSIGNED_INT, 0);
            glDrawElements(GL_TRIANGLES, Frame->TerrainIndicesCount, GL_UNSIGNED_INT, 0);
            Opengl->glBindVertexArray(0);
        }
        
        if(Frame->IsTerrainInLinePolygonMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }*/
        
        
        
        
        // TODO(ezexff): Test Terrain v2.0
        {
            for(u32 GroundBufferIndex = 0;
                GroundBufferIndex < Frame->GroundBufferCount;
                ++GroundBufferIndex)
            {
                ground_buffer *GroundBuffer = Frame->GroundBuffers + GroundBufferIndex;
                
                if(GroundBuffer->IsFilled)
                {
                    if(!GroundBuffer->IsInitialized)
                    {
                        Opengl->glGenVertexArrays(1, &GroundBuffer->VAO);
                        Opengl->glGenBuffers(1, &GroundBuffer->VBO);
                        Opengl->glGenBuffers(1, &GroundBuffer->EBO);
                        GroundBuffer->IsInitialized = true;
                    }
                    Opengl->glBindVertexArray(GroundBuffer->VAO);
                    Opengl->glBindBuffer(GL_ARRAY_BUFFER, GroundBuffer->VBO);
                    Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_vertex) * Frame->ChunkVertexCount, GroundBuffer->Vertices, GL_STATIC_DRAW);
                    
                    Opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundBuffer->EBO);
                    Opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Frame->ChunkIndexCount, Frame->ChunkIndices, GL_STATIC_DRAW);
                    
                    Opengl->glEnableVertexAttribArray(0);
                    Opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)0);
                    Opengl->glEnableVertexAttribArray(1);
                    Opengl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, Normal));
                    Opengl->glEnableVertexAttribArray(2);
                    Opengl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, TexCoords));
                    
                    Opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);
                    Opengl->glBindVertexArray(0);
                    
                    //v3 OffsetP = GroundBuffer->OffsetP;
                    //Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uOffsetP"), 1, OffsetP.E);
                    
                    m4x4 MatModel = Identity();
                    MatModel = Translate(GroundBuffer->OffsetP);
                    MatModel = Transpose(MatModel); // opengl to glsl format
                    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
                    
                    if(Frame->IsTerrainInLinePolygonMode)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    }
                    
                    Opengl->glBindVertexArray(GroundBuffer->VAO);
                    glDrawElements(GL_TRIANGLES, Frame->ChunkIndexCount, GL_UNSIGNED_INT, 0);
                    Opengl->glBindVertexArray(0);
                    
                    if(Frame->IsTerrainInLinePolygonMode)
                    {
                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    }
                }
            }
        }
    }
}

void
OpenglDrawDebugShapes(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    // Proj
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Frame->AspectRatio * Renderer->FOV, Frame->AspectRatio * Renderer->FOV, -Renderer->FOV, Renderer->FOV, Renderer->FOV * 2, 1000);
    
    // View
    glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
    glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
    glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
    glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
    
    // NOTE(ezexff): Draw world push buffer
    for(u32 BaseAddress = 0;
        BaseAddress < Frame->PushBufferSize;
        )
    {
        renderer_entry_header *Header = (renderer_entry_header *)
        (Frame->PushBufferBase + BaseAddress);
        BaseAddress += sizeof(*Header);
        
        void *Data = (u8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
            case RendererEntryType_renderer_entry_clear:
            {
                renderer_entry_clear *Entry = (renderer_entry_clear *)Data;
                
                /*glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);*/
                // TODO(ezexff): Remove this! Replaced with Frame->Color
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_on_ground:
            {
                renderer_entry_rect_on_ground *Entry = (renderer_entry_rect_on_ground *)Data;
                OpenglDrawRectOnGround(GL_QUADS,
                                       {Entry->P, Entry->P + Entry->Dim}, 0.0f, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_outline_on_ground:
            {
                renderer_entry_rect_outline_on_ground *Entry = (renderer_entry_rect_outline_on_ground *)Data;
                OpenglDrawRectOnGround(GL_LINE_LOOP,
                                       {Entry->P, Entry->P + Entry->Dim}, 0.0f, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z), Entry->LineWidth);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_bitmap_on_ground:
            {
                renderer_entry_bitmap_on_ground *Entry = (renderer_entry_bitmap_on_ground *)Data;
                OpenglDrawBitmapOnGround(&Frame->Opengl, Entry->Bitmap, Entry->P, Entry->Dim, Entry->Repeat);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_cube:
            {
                renderer_entry_cube *Entry = (renderer_entry_cube *)Data;
                OpenglDrawCube(Entry->P, Entry->Dim, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_cube_outline:
            {
                renderer_entry_cube_outline *Entry = (renderer_entry_cube_outline *)Data;
                OpenglDrawCubeOutline(Entry->P, Entry->Dim, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z),
                                      Entry->LineWidth);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_on_screen:
            {
                renderer_entry_rect_on_screen *Entry = (renderer_entry_rect_on_screen *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_bitmap_on_screen:
            {
                renderer_entry_bitmap_on_screen *Entry = (renderer_entry_bitmap_on_screen *)Data;
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_terrain_chunk:
            {
                renderer_entry_terrain_chunk *Entry = (renderer_entry_terrain_chunk *)Data;
                //OpenglDrawTerrainChunk(Entry->PositionsCount, Entry->Positions,
                //Entry->IndicesCount, Entry->Indices);
                //OpenglFillTerrainVBO(Frame, Entry->PositionsCount, Entry->Positions,
                //Entry->IndicesCount, Entry->Indices);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_line:
            {
                renderer_entry_line *Entry = (renderer_entry_line *)Data;
                OpenglDrawLine(Entry->P1, Entry->P2, Entry->Color, Entry->LineWidth);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_water:
            {
                renderer_entry_water *Entry = (renderer_entry_water *)Data;
                
                //OpenglDrawRectOnGround(GL_QUADS, {Entry->P.xy, Entry->P.xy + Entry->Dim}, 0.0f, V3(0, 0, 1));
                
                /* 
                                Opengl->glUseProgram(Frame->WaterProg.ID);
                                WaterUniforms(Frame, Frame->WaterProg.ID);
                                OpenglDrawWater({Entry->P.xy, Entry->P.xy + Entry->Dim});
                                Opengl->glUseProgram(0);
                 */
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            InvalidDefaultCase;
        }
    }
}

void
OpenglEndFrame(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    
    Assert(Opengl);
    Assert(Renderer);
    Assert(Camera);
    Assert(Frame->Dim.x > 0);
    Assert(Frame->Dim.y > 0);
    
    glEnable(GL_DEPTH_TEST);
    
    //~NOTE(ezexff): ShadowMap
    glViewport(0, 0, Frame->ShadowMapDim.x, Frame->ShadowMapDim.y);
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->ShadowMapFBO);
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        {
            Opengl->glUseProgram(Frame->ShadowMapProg.ID);
            {
                ShadowMapUniforms(Frame, Frame->ShadowMapProg.ID);
                OpenglDrawTerrain(Frame, Frame->ShadowMapProg.ID);
            }
            Opengl->glUseProgram(0);
        }
        glCullFace(GL_BACK);
    }
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    //~ NOTE(ezexff): Water reflection and refraction
    {
        r32 WaterZ = 0.0f;
        
        // NOTE(ezexff): Reflection
        glEnable(GL_CLIP_DISTANCE0);
        {
            r32 OldCameraPitch = Camera->Angle.x;
            r32 OldCameraZ = Camera->P.z;
            r32 WaterZ = 0.0f;
            r32 ReflectionCameraZ = OldCameraZ;
            r32 ReflectionCameraOffset = 2 * (ReflectionCameraZ - WaterZ);
            ReflectionCameraZ -= ReflectionCameraOffset;
            
            Camera->Angle.x = Frame->CameraPitchInverted;
            Camera->P.z = ReflectionCameraZ;
            Frame->CutPlane = V4(0, 0, 1, -WaterZ);
            glViewport(0, 0, Frame->WaterReflectionDim.x, Frame->WaterReflectionDim.y);
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->WaterReflectionFBO);
            {
                glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                Opengl->glUseProgram(Frame->DefaultProg.ID);
                {
                    Opengl->glUniform4fv(Opengl->glGetUniformLocation(Frame->DefaultProg.ID, "uCutPlane"), 1, Frame->CutPlane.E);
                    
                    DefaultUniforms(Frame, Frame->DefaultProg.ID);
                    OpenglDrawTerrain(Frame, Frame->DefaultProg.ID);
                }
                Opengl->glUseProgram(0);
                
                // NOTE(ezexff): Skybox
                if(IsSet(Renderer, RendererFlag_Skybox))
                {
                    OpenglDrawSkybox(Frame);
                }
            }
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            Camera->Angle.x = OldCameraPitch;
            Camera->P.z = OldCameraZ;
        }
        
        // NOTE(ezexff): Refraction
        {
            Frame->CutPlane = V4(0, 0, -1, WaterZ);
            glViewport(0, 0, Frame->WaterRefractionDim.x, Frame->WaterRefractionDim.y);
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->WaterRefractionFBO);
            {
                glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                Opengl->glUseProgram(Frame->DefaultProg.ID);
                {
                    Opengl->glUniform4fv(Opengl->glGetUniformLocation(Frame->DefaultProg.ID, "uCutPlane"), 1, Frame->CutPlane.E);
                    
                    DefaultUniforms(Frame, Frame->DefaultProg.ID);
                    OpenglDrawTerrain(Frame, Frame->DefaultProg.ID);
                }
                Opengl->glUseProgram(0);
                
                // NOTE(ezexff): Skybox
                if(IsSet(Renderer, RendererFlag_Skybox))
                {
                    OpenglDrawSkybox(Frame);
                }
            }
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
            Frame->CutPlane = V4(0, 0, -1, 100000);
        }
        glDisable(GL_CLIP_DISTANCE0);
    }
    
    //~ NOTE(ezexff): Game
    glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->FBO);
    {
        glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // NOTE(ezexff): Draw scene with default shader
        Opengl->glUseProgram(Frame->DefaultProg.ID);
        {
            DefaultUniforms(Frame, Frame->DefaultProg.ID);
            OpenglDrawTerrain(Frame, Frame->DefaultProg.ID);
            //if(Frame->PushBufferWithLight)
            {
                //OpenglDrawPushBuffer(Frame);
            }
            // TODO(ezexff): 
        }
        Opengl->glUseProgram(0);
        
        OpenglDrawWater(Frame);
        OpenglDrawDebugShapes(Frame);
        /* 
                // NOTE(ezexff): Water
                Opengl->glUseProgram(Frame->WaterProg.ID);
                WaterUniforms(Frame, Frame->WaterProg.ID);
                OpenglDrawWater(Frame);
                Opengl->glUseProgram(0);
         */
        
        // NOTE(ezexff): Skybox
        if(IsSet(Renderer, RendererFlag_Skybox))
        {
            OpenglDrawSkybox(Frame);
        }
    }
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    
    //~ NOTE(ezexff): Frame texture
    {
        Opengl->glUseProgram(Frame->Program.ID);
        
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Frame->Program.ID, "EffectID"), Frame->FragEffect);
        
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Frame->ColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Frame->Program.ID, "ColorTexture"), 0);
        
        glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
        
        Opengl->glBindVertexArray(Frame->VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        Opengl->glBindVertexArray(0);
        
        Opengl->glUseProgram(0);
    }
    
    //~ NOTE(ezexff): UI
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // NOTE(ezexff): Draw screen push buffer
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, (r32)Frame->Dim.x, (r32)Frame->Dim.y, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // NOTE(ezexff): Center y and scale
        s32 ScreenCenterY = (Frame->Dim.y + 1) / 2;
        glTranslatef(0, (r32)ScreenCenterY, 0);
        glScalef(0.5f, 0.5f, 1);
        
        for(u32 BaseAddress = 0;
            BaseAddress < Frame->PushBufferSize;
            )
        {
            renderer_entry_header *Header = (renderer_entry_header *)
            (Frame->PushBufferBase + BaseAddress);
            BaseAddress += sizeof(*Header);
            
            void *Data = (u8 *)Header + sizeof(*Header);
            switch(Header->Type)
            {
                case RendererEntryType_renderer_entry_clear:
                {
                    renderer_entry_clear *Entry = (renderer_entry_clear *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_rect_on_ground:
                {
                    renderer_entry_rect_on_ground *Entry = (renderer_entry_rect_on_ground *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_rect_outline_on_ground:
                {
                    renderer_entry_rect_outline_on_ground *Entry = (renderer_entry_rect_outline_on_ground *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_bitmap_on_ground:
                {
                    renderer_entry_bitmap_on_ground *Entry = (renderer_entry_bitmap_on_ground *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_cube:
                {
                    renderer_entry_cube *Entry = (renderer_entry_cube *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_cube_outline:
                {
                    renderer_entry_cube_outline *Entry = (renderer_entry_cube_outline *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_rect_on_screen:
                {
                    renderer_entry_rect_on_screen *Entry = (renderer_entry_rect_on_screen *)Data;
                    OpenglDrawRectOnScreen({Entry->P, Entry->P + Entry->Dim}, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_bitmap_on_screen:
                {
                    renderer_entry_bitmap_on_screen *Entry = (renderer_entry_bitmap_on_screen *)Data;
                    OpenglDrawBitmapOnScreen(&Frame->Opengl, Entry->Bitmap, Entry->P, Entry->Dim, 
                                             V3(0, 1, 0), Entry->Repeat);
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_terrain_chunk:
                {
                    renderer_entry_terrain_chunk *Entry = (renderer_entry_terrain_chunk *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_line:
                {
                    renderer_entry_line *Entry = (renderer_entry_line *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_water:
                {
                    renderer_entry_water *Entry = (renderer_entry_water *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                InvalidDefaultCase;
            }
        }
        
        glDisable(GL_BLEND);
    }
    
    /*
    // NOTE(ezexff): Draw Player Crosshair
        r32 Crosshair[] = {0, -1, 0, 1, -1, 0, 1, 0};
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, Crosshair);
        glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef((Frame->Dim.x - 1) / 2.0f, (Frame->Dim.y - 1) / 2.0f, 0);
        glScalef(15, 15, 1);
        glLineWidth(3);
        glDisable(GL_TEXTURE_2D);
        glDrawArrays(GL_LINES, 0, 4);
        glPopMatrix();
        glDisableClientState(GL_VERTEX_ARRAY);*/
    
    
    // NOTE(ezexff): Red rectangle
    /*{
    glColor3f(1, 0, 0);
    glBegin(GL_QUADS);
    
    float OffsetFromCenter = 0.5f;
    glVertex2f(-OffsetFromCenter, -OffsetFromCenter);
    glVertex2f(OffsetFromCenter, -OffsetFromCenter);
    glVertex2f(OffsetFromCenter, OffsetFromCenter);
    glVertex2f(-OffsetFromCenter, OffsetFromCenter);
    
    glEnd();
}*/
}