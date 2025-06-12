void
OpenglCompileShader(GLuint Type, loaded_shader *Shader)
{
    if(Shader->OpenglID != 0)
    {
        Opengl->glDeleteShader(Shader->OpenglID);
    }
    
    Shader->OpenglID = Opengl->glCreateShader(Type);
    u8 *ShaderText = Shader->Text;
    Opengl->glShaderSource(Shader->OpenglID, 1, &(GLchar *)ShaderText, NULL);
    Opengl->glCompileShader(Shader->OpenglID);
    
    char *TypeStr = (Type == GL_VERTEX_SHADER) ? "vert" : "frag";
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetShaderiv(Shader->OpenglID, GL_COMPILE_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetShaderInfoLog(Shader->OpenglID, 2000, NULL, LogInfo);
        Opengl->glDeleteShader(Shader->OpenglID);
        Shader->OpenglID = 0;
        //Log->Add("[opengl]: %s shader compilaton error (info below):\n%s", TypeStr, LogInfo);
        InvalidCodePath;
    }
}

void
OpenglLinkProgram(loaded_shader *VertShader, loaded_shader *FragShader, shader_program *Program)
{
    if(Program->OpenglID != 0)
    {
        Opengl->glDeleteProgram(Program->OpenglID);
    }
    
    Program->OpenglID = Opengl->glCreateProgram();
    
    if(VertShader->OpenglID != 0)
    {
        Opengl->glAttachShader(Program->OpenglID, VertShader->OpenglID);
    }
    else
    {
        //Log->Add("[program error]: vert shader null\n");
        InvalidCodePath;
    }
    
    if(FragShader->OpenglID != 0)
    {
        Opengl->glAttachShader(Program->OpenglID, FragShader->OpenglID);
    }
    else
    {
        //Log->Add("[program error]: frag shader null\n");
        InvalidCodePath;
    }
    
    Opengl->glLinkProgram(Program->OpenglID);
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetProgramiv(Program->OpenglID, GL_LINK_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetProgramInfoLog(Program->OpenglID, 2000, NULL, LogInfo);
        //Log->Add("[program error]: program linking error (info below):\n%s", LogInfo);
        InvalidCodePath;
    }
    else
    {
        Log->Add("[opengl] shader program compiled\n");
    }
}

void
OpenglInitRendererMatrices(renderer *Renderer, r32 AspectRatio)
{
    TIMED_FUNCTION();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-AspectRatio * Renderer->FOV, AspectRatio * Renderer->FOV, -Renderer->FOV, Renderer->FOV, Renderer->FOV * 2, 1000);
    glGetFloatv(GL_PROJECTION_MATRIX, (r32 *)Renderer->Proj.E);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-Renderer->Camera.Angle.x, 1.0f, 0.0f, 0.0f);
    glRotatef(-Renderer->Camera.Angle.y, 0.0f, 0.0f, 1.0f);
    glRotatef(-Renderer->Camera.Angle.z, 0.0f, 1.0f, 0.0f);
    glTranslatef(-Renderer->Camera.P.x, -Renderer->Camera.P.y, -Renderer->Camera.P.z);
    glGetFloatv(GL_MODELVIEW_MATRIX, (r32 *)Renderer->View.E);
    
    Renderer->Model = Identity();
    Renderer->Model = Transpose(Renderer->Model); // opengl to glsl format
}

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
OpenglDrawRectOnScreen(rectangle2 R, v4 Color)
{
    r32 VertPositions[] = 
    {
        R.Min.x, R.Min.y, // 0
        R.Max.x, R.Min.y, // 1
        R.Max.x, R.Max.y, // 2
        R.Min.x, R.Max.y, // 3
    };
    
    r32 VertColors[] = {
        Color.x, Color.y, Color.z, Color.a, // 0
        Color.x, Color.y, Color.z, Color.a, // 1
        Color.x, Color.y, Color.z, Color.a, // 2
        Color.x, Color.y, Color.z, Color.a, // 3
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VertPositions);
    glColorPointer(4, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void
OpenglDrawRectOutlineOnScreen(rectangle2 R, r32 LineWidth, v4 Color)
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
    
    glLineWidth(LineWidth);
    glEnable(GL_LINE_SMOOTH);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
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
OpenglDrawBitmapOnGround(loaded_bitmap *Bitmap, v2 P, v2 Dim, r32 Repeat)
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
OpenglDrawBitmapOnScreen(loaded_bitmap *Bitmap, v2 P, v2 Dim, v3 Color, r32 Repeat)
{
    if(!Bitmap->OpenglID)
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
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
    
    //glColor3f(0, 0, 1);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
}

void
OpenglDrawBitmapOnScreen(loaded_bitmap *Bitmap, rectangle2 R, v4 Color, r32 *TexCoords)
{
    if(!Bitmap->OpenglID)
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
        /* 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                 */
        /* 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         */
        /* 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
         */
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        /*         
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Bitmap->Width, Bitmap->Height, 0,
                             Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
                 */
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, Bitmap->Width, Bitmap->Height, 0,
                     Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
#endif
    }
    
    /* 
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
         */
    
    r32 VertPositions[] = 
    {
        R.Min.x, R.Min.y, // 0
        R.Max.x, R.Min.y, // 1
        R.Max.x, R.Max.y, // 2
        R.Min.x, R.Max.y, // 3
    };
    
    /* 
        r32 TexRectangle[] =
        {
            0, 0,           // 0
            Repeat, 0,      // 1
            Repeat, Repeat, // 2
            0, Repeat       // 3
        };
     */
    
    r32 Colors[] =
    {
        Color.x, Color.y, Color.z, Color.a,
        Color.x, Color.y, Color.z, Color.a,
        Color.x, Color.y, Color.z, Color.a,
        Color.x, Color.y, Color.z, Color.a,
    };
    
    glEnable(GL_TEXTURE_2D);
    
    Opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Bitmap->OpenglID);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VertPositions);
    glTexCoordPointer(2, GL_FLOAT, 0, TexCoords);
    glColorPointer(4, GL_FLOAT, 0, Colors);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
}

void
OpenglInitSkybox(renderer *Renderer)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Skybox))
    {
        Assert(Renderer->Skybox);
        renderer_skybox *Skybox = Renderer->Skybox;
        if(!Skybox->Texture)
        {
            r32 Vertices[] = 
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
            
            glGenTextures(1, &Skybox->Texture);
            
            Opengl->glGenVertexArrays(1, &Skybox->VAO);
            Opengl->glGenBuffers(1, &Skybox->VBO);
            
            Opengl->glBindVertexArray(Skybox->VAO);
            Opengl->glBindBuffer(GL_ARRAY_BUFFER, Skybox->VBO);
            Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), &Vertices, GL_STATIC_DRAW);
            Opengl->glEnableVertexAttribArray(0);
            Opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(r32), (void*)0);
        }
        
        if(Skybox->Texture && !Skybox->IsTextureParametersInitialized)
        {
            // TODO(ezexff): Only for testing
            u32 LoadedBitmapCount = 0;
            for(u32 Index = 0;
                Index < 6;
                Index++)
            {
                loaded_bitmap *Bitmap = &Skybox->Bitmaps[Index];
                if(Bitmap->Memory)
                {
                    LoadedBitmapCount++;
                }
            }
            
            if(LoadedBitmapCount == 6)
            {
                glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox->Texture);
                for(u32 Index = 0;
                    Index < 6;
                    Index++)
                {
                    loaded_bitmap *Bitmap = &Skybox->Bitmaps[Index];
                    if(Bitmap->Memory)
                    {
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  
                        
                        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + Index, 0, GL_RGB, Bitmap->Width, Bitmap->Height, 0, 
                                     Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Bitmap->Memory);
                    }
                }
                Skybox->IsTextureParametersInitialized = true;
            }
        }
    }
}

void
OpenglInitShadowMap(renderer *Renderer)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Shadows))
    {        
        Assert(Renderer->ShadowMap);
        renderer_shadowmap *ShadowMap = Renderer->ShadowMap;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ShadowMap->Size, ShadowMap->Size, -ShadowMap->Size, ShadowMap->Size,
                ShadowMap->NearPlane, ShadowMap->FarPlane);
        glGetFloatv(GL_PROJECTION_MATRIX, (r32 *)ShadowMap->Proj.E);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-ShadowMap->CameraPitch, 1.0f, 0.0f, 0.0f);
        glRotatef(-ShadowMap->CameraYaw, 0.0f, 0.0f, 1.0f);
        glTranslatef(-ShadowMap->CameraP.x, -ShadowMap->CameraP.y, -ShadowMap->CameraP.z);
        glGetFloatv(GL_MODELVIEW_MATRIX, (r32 *)ShadowMap->View.E);
        
        ShadowMap->Model = Identity();
        ShadowMap->Model = Transpose(ShadowMap->Model); // opengl to glsl format
        
        if(!ShadowMap->FBO)
        {
            u32 ShadowMapSizeMultiplyer = 8;
            ShadowMap->Dim.x = 1920 * ShadowMapSizeMultiplyer;
            ShadowMap->Dim.y = 1080 * ShadowMapSizeMultiplyer;
            
            Opengl->glGenFramebuffers(1, &ShadowMap->FBO);
            glGenTextures(1, &ShadowMap->Texture);
            
            glBindTexture(GL_TEXTURE_2D, ShadowMap->Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMap->Dim.x, ShadowMap->Dim.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
            
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, ShadowMap->FBO);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap->Texture, 0);
            if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                InvalidCodePath;
            }
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
}

void
OpenglInitWater(renderer *Renderer)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Water))
    {    
        Assert(Renderer->Water);
        renderer_water *Water = Renderer->Water;
        renderer_water_reflection *Reflection = &Water->Reflection;
        if(!Reflection->FBO)
        {
            u32 ReflectionDivider = 4;
            Reflection->Dim.x = 1920 / ReflectionDivider;
            Reflection->Dim.y = 1080 / ReflectionDivider;
            
            Opengl->glGenFramebuffers(1, &Reflection->FBO);
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Reflection->FBO);
            
            // Create Texture Attachment
            glGenTextures(1, &Reflection->ColorTexture);
            glBindTexture(GL_TEXTURE_2D, Reflection->ColorTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Reflection->Dim.x, Reflection->Dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Reflection->ColorTexture, 0);
            if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                InvalidCodePath;
            }
            
            // Create Render Buffer Object
            Opengl->glGenRenderbuffers(1, &Reflection->DepthRBO);
            Opengl->glBindRenderbuffer(GL_RENDERBUFFER, Reflection->DepthRBO);
            Opengl->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Reflection->Dim.x, Reflection->Dim.y);
            Opengl->glBindRenderbuffer(GL_RENDERBUFFER, 0);
            Opengl->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Reflection->DepthRBO);
            if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                InvalidCodePath;
            }
        }
        
        Assert(Reflection->DuDv);
        loaded_bitmap *DuDv = Reflection->DuDv;
        if(!DuDv->OpenglID && DuDv->Memory)
        {
            glGenTextures(1, &DuDv->OpenglID);
            glBindTexture(GL_TEXTURE_2D, DuDv->OpenglID);
            // Texture wrapping or filtering options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DuDv->Width, DuDv->Height, 0, DuDv->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, DuDv->Memory);
            Opengl->glGenerateMipmap(GL_TEXTURE_2D);
            
        }
        
        Assert(Reflection->NormalMap);
        loaded_bitmap *NormalMap = Reflection->NormalMap;
        if(!NormalMap->OpenglID && NormalMap->Memory)
        {
            glGenTextures(1, &NormalMap->OpenglID);
            glBindTexture(GL_TEXTURE_2D, NormalMap->OpenglID);
            // Texture wrapping or filtering options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, NormalMap->Width, NormalMap->Height, 0, NormalMap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NormalMap->Memory);
            Opengl->glGenerateMipmap(GL_TEXTURE_2D);
            
        }
        
        renderer_water_refraction *Refraction = &Water->Refraction;
        if(!Refraction->FBO)
        {
            u32 RefractionDivider = 2;
            Refraction->Dim.x = 1920 / RefractionDivider;
            Refraction->Dim.y = 1080 / RefractionDivider;
            
            Opengl->glGenFramebuffers(1, &Refraction->FBO);
            Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Refraction->FBO);
            
            // Create Texture Attachment
            glGenTextures(1, &Refraction->ColorTexture);
            glBindTexture(GL_TEXTURE_2D, Refraction->ColorTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Refraction->Dim.x, Refraction->Dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Refraction->ColorTexture, 0);
            
            // Create Depth Texture Attachment
            glGenTextures(1, &Refraction->DepthTexture);
            glBindTexture(GL_TEXTURE_2D, Refraction->DepthTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Refraction->Dim.x, Refraction->Dim.y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Opengl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Refraction->DepthTexture, 0);
            if(Opengl->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                InvalidCodePath;
            }
        }
    }
}

void
OpenglInitTerrain(renderer *Renderer)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Terrain))
    {
        Assert(Renderer->Terrain);
        renderer_terrain *Terrain = Renderer->Terrain;
        Assert(Terrain->Bitmap);
        if(Terrain->Bitmap)
        {
            if(!Terrain->Bitmap->OpenglID && Terrain->Bitmap->Memory)
            {
                glGenTextures(1, &Terrain->Bitmap->OpenglID);
                glBindTexture(GL_TEXTURE_2D, Terrain->Bitmap->OpenglID);
                // Texture wrapping or filtering options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Terrain->Bitmap->Width, Terrain->Bitmap->Height, 0, Terrain->Bitmap->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Terrain->Bitmap->Memory);
                Opengl->glGenerateMipmap(GL_TEXTURE_2D);
            }
        }
    }
}

void
OpenglCompileShaders(renderer_frame *Frame)
{
    TIMED_FUNCTION();
    if(Frame->CompileShaders)
    {        
        renderer *Renderer = (renderer *)Frame->Renderer;
        renderer_shaders *Shaders = &Frame->Shaders;
        renderer_programs *Programs = &Frame->Programs;
        
        OpenglCompileShader(GL_VERTEX_SHADER, &Shaders->FrameVert);
        OpenglCompileShader(GL_FRAGMENT_SHADER, &Shaders->FrameFrag);
        OpenglLinkProgram(&Shaders->FrameVert, &Shaders->FrameFrag, &Programs->Frame);
        
        OpenglCompileShader(GL_VERTEX_SHADER, &Shaders->SceneVert);
        OpenglCompileShader(GL_FRAGMENT_SHADER, &Shaders->SceneFrag);
        OpenglLinkProgram(&Shaders->SceneVert, &Shaders->SceneFrag, &Programs->Scene);
        
        OpenglCompileShader(GL_VERTEX_SHADER, &Shaders->SkyboxVert);
        OpenglCompileShader(GL_FRAGMENT_SHADER, &Shaders->SkyboxFrag);
        OpenglLinkProgram(&Shaders->SkyboxVert, &Shaders->SkyboxFrag, &Programs->Skybox);
        
        OpenglCompileShader(GL_VERTEX_SHADER, &Shaders->ShadowMapVert);
        OpenglCompileShader(GL_FRAGMENT_SHADER, &Shaders->ShadowMapFrag);
        OpenglLinkProgram(&Shaders->ShadowMapVert, &Shaders->ShadowMapFrag, &Programs->ShadowMap);
        
        OpenglCompileShader(GL_VERTEX_SHADER, &Shaders->WaterVert);
        OpenglCompileShader(GL_FRAGMENT_SHADER, &Shaders->WaterFrag);
        OpenglLinkProgram(&Shaders->WaterVert, &Shaders->WaterFrag, &Programs->Water);
        
        Frame->CompileShaders = false;
    }
}

void
OpenglInit(renderer_frame *Frame)
{
    Opengl = &Frame->Opengl;
    
    // NOTE(ezexff): Push buffer
    Frame->MaxPushBufferSize = sizeof(Frame->PushBufferMemory);
    Frame->PushBufferBase = Frame->PushBufferMemory;
    
    // TODO(ezexff): Test water push buffer
    Frame->WaterMaxPushBufferSize = sizeof(Frame->WaterPushBufferMemory);
    Frame->WaterPushBufferBase = Frame->WaterPushBufferMemory;
    
    // NOTE(ezexff): Camera
    //Frame->Camera.P.z = 10.0f;
    
    // NOTE(ezexff): Frame
    Opengl->glGenFramebuffers(1, &Frame->FBO);
    glGenTextures(1, &Frame->ColorTexture);
    glGenTextures(1, &Frame->DepthTexture);
    
    u32 VertexCount = 6;
    frame_vertex Vertices[6];
    
    Vertices[0].Position = {-1.0f, -1.0f, 0.0f};
    Vertices[1].Position = {1.0f, -1.0f, 0.0f};
    Vertices[2].Position = {-1.0f,  1.0f, 0.0f};
    Vertices[3].Position = {1.0f, -1.0f, 0.0f};
    Vertices[4].Position = {1.0f,  1.0f, 0.0f};
    Vertices[5].Position = {-1.0f,  1.0f, 0.0f};
    
    Vertices[0].TexCoords  = {0.0f, 0.0f};
    Vertices[1].TexCoords  = {1.0f, 0.0f};
    Vertices[2].TexCoords  = {0.0f, 1.0f};
    Vertices[3].TexCoords  = {1.0f, 0.0f};
    Vertices[4].TexCoords  = {1.0f, 1.0f};
    Vertices[5].TexCoords  = {0.0f, 1.0f};
    
    Opengl->glGenVertexArrays(1, &Frame->VAO);
    Opengl->glGenBuffers(1, &Frame->VBO);
    //Opengl->glGenBuffers(1, &Frame->EBO);
    
    Opengl->glBindVertexArray(Frame->VAO);
    Opengl->glBindBuffer(GL_ARRAY_BUFFER, Frame->VBO);
    
    Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(frame_vertex) * VertexCount, Vertices, GL_STATIC_DRAW);
#if 1
    //#define GL_OFFSET(x) ((const GLvoid *)(x))
#define GL_OFFSETOF(Type, Member) ((memory_index)&((Type *)0)->Member)
    
    Opengl->glEnableVertexAttribArray(VertexAttributeIndex_Position);
    Opengl->glVertexAttribPointer(VertexAttributeIndex_Position, 3, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)0);
    
    Opengl->glEnableVertexAttribArray(VertexAttributeIndex_TexCoord);
    Opengl->glVertexAttribPointer(VertexAttributeIndex_TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)GL_OFFSETOF(frame_vertex, TexCoords));
    
#else
    // Positions
    Opengl->glEnableVertexAttribArray(VertexAttributeIndex_Position);
    Opengl->glVertexAttribPointer(VertexAttributeIndex_Position, 3, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)0);
    // Texture coords
    Opengl->glEnableVertexAttribArray(VertexAttributeIndex_TexCoord);
    Opengl->glVertexAttribPointer(VertexAttributeIndex_TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(frame_vertex), (void *)offsetof(frame_vertex, TexCoords));
#endif
    
    Opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    Opengl->glBindVertexArray(0);
}

void
OpenglBeginFrame(renderer_frame *Frame)
{
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
    
    while(Frame->PushBufferSize--)
    {
        Frame->PushBufferMemory[Frame->PushBufferSize] = 0;
        
        // TODO(ezexff): Test
        Frame->WaterPushBufferMemory[Frame->WaterPushBufferSize] = 0;
    }
    Frame->PushBufferSize = 0;
    Frame->WaterPushBufferSize = 0;
}

void
OpenglMeshUniforms(renderer *Renderer, u32 ShaderProgramID)
{
    Opengl->glUniform4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCutPlane"), 1, Renderer->CutPlane.E);
    
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, (r32 *)Renderer->Proj.E);
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, (r32 *)Renderer->View.E);
    Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (r32 *)Renderer->Model.E);
    
    if(IsSet(Renderer, RendererFlag_Shadows))
    {
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProjShadowMap"), 1, GL_FALSE, (r32 *)Renderer->ShadowMap->Proj.E);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uViewShadowMap"), 1, GL_FALSE, (r32 *)Renderer->ShadowMap->View.E);
        
        Opengl->glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Renderer->ShadowMap->Texture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uShadowMap"), 1);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uBias"), Renderer->ShadowMap->Bias);
        
    }
    if(IsSet(Renderer, RendererFlag_Lighting))
    {
        renderer_lighting *Lighting = Renderer->Lighting;
        // NOTE(ezexff): Send camera pos into shader for light calc
        // TODO(ezexff): Позиция солнца должна быть рассчитана относительно позиции в мировом пространстве
        //Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, SunP.E);
        //Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, Frame->WorldOrigin.E);
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraLocalP"), 1, Lighting->TestSunP.E);
        
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.Color"), 1, Lighting->DirLight.Base.Color.E);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.AmbientIntensity"), Lighting->DirLight.Base.AmbientIntensity);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Base.DiffuseIntensity"), Lighting->DirLight.Base.DiffuseIntensity);
        v3 DirLightDirection = Lighting->DirLight.WorldDirection;
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uDirectionalLight.Direction"), 1, DirLightDirection.E);
        
        // TODO(ezexff): Replace this with material code per mesh
        if(IsSet(Renderer, RendererFlag_Terrain))
        {        
            renderer_terrain *Terrain = Renderer->Terrain;
            Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.AmbientColor"), 1, Terrain->Material.Ambient.E);
            Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.DiffuseColor"), 1, Terrain->Material.Diffuse.E);
            Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uMaterial.SpecularColor"), 1, Terrain->Material.Specular.E);
        }
    }
    if(IsSet(Renderer, RendererFlag_Terrain))
    {
        if(Renderer->Terrain->Bitmap->OpenglID)
        {
            Opengl->glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, Renderer->Terrain->Bitmap->OpenglID);
            Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uSampler"), 0);
            Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uSamplerSpecularExponent"), 0);
        }
    }
}

void
OpenglDrawSkybox(renderer *Renderer, u32 ShaderProgramID)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Skybox))
    {
        Assert(Renderer->Skybox);
        renderer_skybox *Skybox = Renderer->Skybox;
        
        glDepthFunc(GL_LEQUAL);
        Opengl->glUseProgram(ShaderProgramID);
        
        // NOTE(ezexff): Send matrices into shader
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "Proj"), 1, GL_FALSE, (r32 *)Renderer->Proj.E);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "View"), 1, GL_FALSE, (r32 *)Renderer->View.E);
        m4x4 Model = Identity();
        Model = XRotation(90) * Scale(200);
        Model = Transpose(Model); // opengl to glsl format
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "Model"), 1, GL_FALSE, (r32 *)Model.E);
        
        // NOTE(ezexff): Send texture into shader
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Skybox->Texture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "SkyboxTexture"), 0);
        
        // NOTE(ezexff): Draw
        Opengl->glBindVertexArray(Skybox->VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        Opengl->glBindVertexArray(0);
        
        Opengl->glUseProgram(0);
        glDepthFunc(GL_LESS);
    }
    
}

void
OpenglDrawTerrain(renderer *Renderer, u32 ShaderProgramID)
{
    if(IsSet(Renderer, RendererFlag_Terrain))
    {
        Assert(Renderer->Terrain);
        
        renderer_terrain *Terrain = Renderer->Terrain;
        ground_buffer_array *GroundBufferArray = &Terrain->GroundBufferArray;
        for(u32 GroundBufferIndex = 0;
            GroundBufferIndex < GroundBufferArray->Count;
            ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = GroundBufferArray->Buffers + GroundBufferIndex;
            
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
                Opengl->glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_vertex) * GroundBufferArray->VertexCount, GroundBuffer->Vertices, GL_STATIC_DRAW);
                
                Opengl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundBuffer->EBO);
                Opengl->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Terrain->IndexArray.Count, Terrain->IndexArray.Indices, GL_STATIC_DRAW);
                
                Opengl->glEnableVertexAttribArray(0);
                Opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)0);
                Opengl->glEnableVertexAttribArray(1);
                Opengl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, Normal));
                Opengl->glEnableVertexAttribArray(2);
                Opengl->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vbo_vertex), (void *)offsetof(vbo_vertex, TexCoords));
                
                Opengl->glBindBuffer(GL_ARRAY_BUFFER, 0);
                Opengl->glBindVertexArray(0);
                
                m4x4 MatModel = Identity();
                MatModel = Translate(GroundBuffer->OffsetP);
                MatModel = Transpose(MatModel); // opengl to glsl format
                Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
                
                Opengl->glBindVertexArray(GroundBuffer->VAO);
                glDrawElements(GL_TRIANGLES, Terrain->IndexArray.Count, GL_UNSIGNED_INT, 0);
                //glDrawElements(GL_LINES, Terrain->IndexArray.Count, GL_UNSIGNED_INT, 0);
                Opengl->glBindVertexArray(0);
            }
        }
    }
}

void
OpenglDrawDebugShapes(renderer_frame *Frame)
{
    TIMED_FUNCTION();
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
                OpenglDrawBitmapOnGround(Entry->Bitmap, Entry->P, Entry->Dim, Entry->Repeat);
                
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
                //renderer_entry_bitmap_on_screen *Entry = (renderer_entry_bitmap_on_screen *)Data;
                InvalidCodePath;
                
                //BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_line:
            {
                renderer_entry_line *Entry = (renderer_entry_line *)Data;
                OpenglDrawLine(Entry->P1, Entry->P2, Entry->Color, Entry->LineWidth);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            /* 
                        case RendererEntryType_renderer_entry_water:
                        {
                            renderer_entry_water *Entry = (renderer_entry_water *)Data;
                            
                            BaseAddress += sizeof(*Entry);
                        } break;
             */
            
            InvalidDefaultCase;
        }
    }
}

void
OpenglDrawMeshes(renderer *Renderer, u32 ShaderProgramID)
{
    TIMED_FUNCTION();
    Opengl->glUseProgram(ShaderProgramID);
    {
        OpenglMeshUniforms(Renderer, ShaderProgramID);
        OpenglDrawTerrain(Renderer, ShaderProgramID);
    }
    Opengl->glUseProgram(0);
}

void
OpenglDrawShadowMap(renderer *Renderer, u32 ShaderProgramID)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Shadows))
    {
        Assert(Renderer->ShadowMap);
        
        renderer_shadowmap *ShadowMap = Renderer->ShadowMap;
        glViewport(0, 0, ShadowMap->Dim.x, ShadowMap->Dim.y);
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, ShadowMap->FBO);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            glCullFace(GL_FRONT);
            Opengl->glUseProgram(ShaderProgramID);
            {
                Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, (r32 *)ShadowMap->Proj.E);
                Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, (r32 *)ShadowMap->View.E);
                Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (r32 *)ShadowMap->Model.E);
                
                OpenglDrawTerrain(Renderer, ShaderProgramID);
            }
            Opengl->glUseProgram(0);
            glCullFace(GL_BACK);
        }
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void
OpenglDrawWaterReflectionAndRefraction(renderer *Renderer, r32 AspectRatio, renderer_programs *Programs)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Water))
    {   
        Assert(Renderer->Water);
        
        //~ NOTE(ezexff): Reflection
        r32 WaterZ = 0.0f;
        
        // NOTE(ezexff): Inverted camera pos and view
        renderer_camera *Camera = &Renderer->Camera;
        m4x4 OldProj = Renderer->Proj;
        m4x4 OldView = Renderer->View;
        m4x4 OldModel = Renderer->Model;
        r32 OldCameraPitch = Camera->Angle.pitch;
        r32 OldCameraZ = Camera->P.z;
        Camera->Angle.pitch = 180 - OldCameraPitch;
        Camera->P.z = OldCameraZ - 2 * (OldCameraZ - WaterZ);
        
        glEnable(GL_CLIP_DISTANCE0);
        
        Renderer->CutPlane = V4(0, 0, 1, -WaterZ);
        OpenglInitRendererMatrices(Renderer, AspectRatio);
        
        glViewport(0, 0, Renderer->Water->Reflection.Dim.x, Renderer->Water->Reflection.Dim.y);
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Renderer->Water->Reflection.FBO);
        {
            glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            OpenglDrawMeshes(Renderer, Programs->Scene.OpenglID);
            OpenglDrawSkybox(Renderer, Programs->Skybox.OpenglID);
        }
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        Renderer->Proj = OldProj;
        Renderer->View = OldView;;
        Renderer->Model = OldModel;
        Camera->Angle.pitch = OldCameraPitch;
        Camera->P.z = OldCameraZ;
        
        //~ NOTE(ezexff): Refraction
        Renderer->CutPlane = V4(0, 0, -1, WaterZ);
        glViewport(0, 0, Renderer->Water->Refraction.Dim.x, Renderer->Water->Refraction.Dim.y);
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Renderer->Water->Refraction.FBO);
        {
            glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            OpenglDrawMeshes(Renderer, Programs->Scene.OpenglID);
            OpenglDrawSkybox(Renderer, Programs->Skybox.OpenglID);
        }
        Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        Renderer->CutPlane = V4(0, 0, -1, 100000);
        
        glDisable(GL_CLIP_DISTANCE0);
    }
}

void
OpenglDrawWater(renderer *Renderer, u32 ShaderProgramID, renderer_frame *Frame)
{
    TIMED_FUNCTION();
    if(IsSet(Renderer, RendererFlag_Water))
    {        
        Assert(Renderer->Water);
        renderer_water *Water = Renderer->Water;
        Opengl->glUseProgram(ShaderProgramID);
        
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uProj"), 1, GL_FALSE, (r32 *)Renderer->Proj.E);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uView"), 1, GL_FALSE, (r32 *)Renderer->View.E);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(ShaderProgramID, "uModel"), 1, GL_FALSE, (r32 *)Renderer->Model.E);
        
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uSunP"), 1, Frame->TestSunRelP.E);
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uCameraP"), 1, Renderer->Camera.P.E);
        
        Opengl->glUniform3fv(Opengl->glGetUniformLocation(ShaderProgramID, "uLightColor"), 1, Renderer->Lighting->DirLight.Base.Color.E);
        
        // NOTE(ezexff): Vars
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uTiling"), Water->Tiling);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uMoveFactor"), Water->MoveFactor);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uWaveStrength"), Water->WaveStrength);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uShineDamper"), Water->ShineDamper);
        Opengl->glUniform1f(Opengl->glGetUniformLocation(ShaderProgramID, "uReflectivity"), Water->Reflectivity);
        
        // glUniform3fv(glGetUniformLocation(ShaderProgramID, "SunPosition"), 1, Render->DirLight.WorldDirection.E);
        //v3 TestSunPos = V3(-Render->SunPos.x, -Render->SunPos.y, -Render->SunPos.z);
        //v3 RelPlayerP = GetRelPos(World, Player->P, Player->TmpZ);
        // glUniform3fv(glGetUniformLocation(ShaderProgramID, "CameraPosition"), 1, Player->Position.E);
        
        // NOTE(ezexff): Textures
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Water->Reflection.ColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uReflectionColorTexture"), 0);
        Opengl->glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, Water->Refraction.ColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uRefractionColorTexture"), 1);
        Opengl->glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, Water->Reflection.DuDv->OpenglID);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uDUDVTexture"), 2);
        Opengl->glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, Water->Reflection.NormalMap->OpenglID);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uNormalMapTexture"), 3);
        Opengl->glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, Water->Refraction.DepthTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(ShaderProgramID, "uRefractionDepthTexture"), 4);
        
#if 0        
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
#endif
        
        for(u32 BaseAddress = 0;
            BaseAddress < Frame->WaterPushBufferSize;
            )
        {
            renderer_entry_water *Entry = (renderer_entry_water *)(Frame->WaterPushBufferBase + BaseAddress);
            
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
        }
        
        
        Opengl->glUseProgram(0);
    }
}

void
OpenglInitBitmapPreview(renderer_frame *Frame)
{
    TIMED_FUNCTION();
    loaded_bitmap *Preview = &Frame->Preview;
    if(!Preview->OpenglID)
    {
        glGenTextures(1, &Preview->OpenglID);
    }
    if(Preview->Memory)
    {
        glBindTexture(GL_TEXTURE_2D, Preview->OpenglID);
        // wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // texture to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Preview->Width, Preview->Height, 0,
                     Preview->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, Preview->Memory);
        //Opengl->glGenerateMipmap(GL_TEXTURE_2D);
    }
}

void
OpenglInitFrameFBO(renderer_frame *Frame)
{
    TIMED_FUNCTION();
    Assert(Frame->Dim.x > 0);
    Assert(Frame->Dim.y > 0);
    
    // NOTE(ezexff): Update FBO texture
    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Frame->ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Frame->Dim.x, Frame->Dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Create Depth Texture Attachment
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
}

internal void
SortPushBufferEntries(renderer_push_buffer *PushBuffer)
{
    u32 Count = PushBuffer->ElementCount;
    tile_sort_entry *SortEntryArray = PushBuffer->SortEntryArray;
    
    for(u32 Outer = 0;
        Outer < Count;
        ++Outer)
    {
        b32 ListIsSorted = true;
        for(u32 Inner = 0;
            Inner < (Count - 1);
            ++Inner)
        {
            tile_sort_entry *EntryA = SortEntryArray + Inner;
            tile_sort_entry *EntryB = EntryA + 1;
            
            if(EntryA->SortKey > EntryB->SortKey)
            {
                tile_sort_entry Swap = *EntryB;
                *EntryB = *EntryA;
                *EntryA = Swap;
                ListIsSorted = false;
            }
        }
        
        if(ListIsSorted)
        {
            break;
        }
    }
}

void
OpenglDrawUI(renderer_frame *Frame)
{
    renderer *Renderer = (renderer *)Frame->Renderer;
    //~ NOTE(ezexff): Draw UI
    //glDisbale(GL_BLEND);    
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER, 0.99f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    //glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    
    // NOTE(ezexff): Draw screen push buffer
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (r32)Frame->Dim.x, (r32)Frame->Dim.y, 0, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* 
        glTranslatef(-(r32)Frame->Dim.x / 2, (r32)Frame->Dim.y / 2, 0.0f);
        glScalef(1.0f, -1.0f, 1.0f);
     */
    /* 
        glScalef(1.0f, -1.0f, 1.0f);
        glTranslatef(0, -(r32)Frame->Dim.y, 0.0f);
     */
    
    //glScalef(1.0f, -1.0f, 1.0f);
    // NOTE(ezexff): Center y and scale
    /* 
        s32 ScreenCenterY = (Frame->Dim.y + 1) / 2;
        glTranslatef(0, (r32)ScreenCenterY, 0);
        glScalef(0.5f, 0.5f, 1);
     */
    
    renderer_push_buffer *PushBufferUI = &Renderer->PushBufferUI;
    SortPushBufferEntries(PushBufferUI);
    
    tile_sort_entry *SortEntryArray = PushBufferUI->SortEntryArray;
    tile_sort_entry *SortEntry = SortEntryArray;
    for(u32 SortEntryIndex = 0;
        SortEntryIndex < PushBufferUI->ElementCount;
        ++SortEntryIndex, ++SortEntry)
    {
        renderer_entry_header *Header = (renderer_entry_header *)
        (PushBufferUI->Base + SortEntry->Offset);
        
        void *Data = (u8 *)Header + sizeof(*Header);
        
        switch(Header->Type)
        {
            case RendererOrthoEntryType_renderer_ortho_entry_rect:
            {
                renderer_ortho_entry_rect *Entry = (renderer_ortho_entry_rect *)Data;
                v2 Min = Entry->P;
                v2 Max = Entry->Dim;
                /* 
                                Max.y = Frame->Dim.y - Entry->P.y;
                                Min.y = Frame->Dim.y - Entry->Dim.y;
                 */
                
                OpenglDrawRectOnScreen({Min, Max}, Entry->Color);
                
                //BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererOrthoEntryType_renderer_ortho_entry_rect_outline:
            {
                renderer_ortho_entry_rect_outline *Entry = (renderer_ortho_entry_rect_outline *)Data;
                OpenglDrawRectOutlineOnScreen(Entry->Rect, Entry->LineWidth, Entry->Color);
                
                //BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererOrthoEntryType_renderer_ortho_entry_bitmap:
            {
                renderer_ortho_entry_bitmap *Entry = (renderer_ortho_entry_bitmap *)Data;
                /* 
                                v2 Min = Entry->P;
                                Min.y = Frame->Dim.y - Entry->P.y;
                                v2 Max = Entry->Dim;
                                Max.y = Frame->Dim.y - Entry->Dim.y;
                                OpenglDrawRectOnScreen({Min, Max}, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                 */
                //OpenglDrawBitmapOnScreen(Entry->Bitmap, Entry->P, Entry->Dim,  V3(0, 1, 0), Entry->Repeat);
                //v2 Min = Entry->P;
                //v2 Max = Entry->Dim;
                /* 
                                Max.y = Frame->Dim.y - Entry->P.y;
                                Min.y = Frame->Dim.y - Entry->Dim.y;
                 */
                
                OpenglDrawBitmapOnScreen(Entry->Bitmap, Entry->Rect, V4(0, 1, 0, 1), Entry->TexCoords);
                //BaseAddress += sizeof(*Entry);
            } break;
            
            /* 
                        case RendererOrthoEntryType_renderer_ortho_entry_bitmap:
                        {
                            //renderer_entry_rect_outline_on_ground *Entry = (renderer_entry_rect_outline_on_ground *)Data;
                            
                            BaseAddress += sizeof(*Entry);
                        } break;
             */
            
            InvalidDefaultCase;
        }
    }
    
    glDisable(GL_BLEND);
    
    /*     
        for(u32 BaseAddress = 0;
            BaseAddress < PushBufferUI->Size;
            )
        {
            renderer_entry_header *Header = (renderer_entry_header *)
            (PushBufferUI->Base + BaseAddress);
            BaseAddress += sizeof(*Header);
            
            void *Data = (u8 *)Header + sizeof(*Header);
            switch(Header->Type)
            {
                case RendererOrthoEntryType_renderer_ortho_entry_rect:
                {
                    renderer_ortho_entry_rect *Entry = (renderer_ortho_entry_rect *)Data;
                    OpenglDrawRectOnScreen({Entry->P, Entry->P + Entry->Dim}, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                InvalidDefaultCase;
            }
        }
         */
    
    
    /*     
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
                    OpenglDrawBitmapOnScreen(Entry->Bitmap, Entry->P, Entry->Dim, 
                                             V3(0, 1, 0), Entry->Repeat);
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                case RendererEntryType_renderer_entry_line:
                {
                    renderer_entry_line *Entry = (renderer_entry_line *)Data;
                    
                    BaseAddress += sizeof(*Entry);
                } break;
                
                InvalidDefaultCase;
            }
        }
     */
}

void
OpenglEndFrame(renderer_frame *Frame)
{
    renderer *Renderer = (renderer *)Frame->Renderer;
    renderer_camera *Camera = &Renderer->Camera;
    renderer_programs *Programs = &Frame->Programs;
    
    Assert(Renderer);
    
    //~ NOTE(ezexff): Init 
    BEGIN_BLOCK("OpenglInitRenderingScene");
    OpenglInitFrameFBO(Frame);
    OpenglInitRendererMatrices(Renderer, Frame->AspectRatio);
    OpenglInitSkybox(Renderer);
    if(IsSet(Renderer, RendererFlag_Lighting))
    {
        // TODO(ezexff): ???
    }
    OpenglInitShadowMap(Renderer);
    OpenglInitWater(Renderer);
    OpenglInitTerrain(Renderer);
    // NOTE(ezexff): Texture for preview window
#if ENGINE_INTERNAL
    OpenglInitBitmapPreview(Frame);
#endif
    
    OpenglCompileShaders(Frame);
    END_BLOCK();
    
    
    //~ NOTE(ezexff): Draw parts of scene into their FBOs
    glEnable(GL_DEPTH_TEST);
    OpenglDrawShadowMap(Renderer, Programs->ShadowMap.OpenglID);
    OpenglDrawWaterReflectionAndRefraction(Renderer, Frame->AspectRatio, &Frame->Programs);
    
    BEGIN_BLOCK("OpenglDrawScene");
    glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->FBO);
    {
        glClearColor(Renderer->ClearColor.x, Renderer->ClearColor.y, Renderer->ClearColor.z, Renderer->ClearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        OpenglDrawMeshes(Renderer, Programs->Scene.OpenglID);
        OpenglDrawWater(Renderer, Programs->Water.OpenglID, Frame);
        OpenglDrawDebugShapes(Frame);
        OpenglDrawSkybox(Renderer, Programs->Skybox.OpenglID);
    }
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    
    //~ NOTE(ezexff): Draw FBO texture on screen
    Opengl->glUseProgram(Programs->Frame.OpenglID);
    {
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Programs->Frame.OpenglID, "EffectID"), Frame->EffectID);
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Frame->ColorTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Programs->Frame.OpenglID, "ColorTexture"), 0);
        
        glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
        Opengl->glBindVertexArray(Frame->VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        Opengl->glBindVertexArray(0);
    }
    Opengl->glUseProgram(0);
    END_BLOCK();
    
    OpenglDrawUI(Frame);
    
    BEGIN_BLOCK("ClearPushBuffer");
    Renderer->Flags = 0;
    
    while(Renderer->PushBufferUI.Size--)
    {
        Renderer->PushBufferUI.Memory[Renderer->PushBufferUI.Size] = 0;
    }
    Renderer->PushBufferUI.Size = 0;
    Renderer->PushBufferUI.ElementCount = 0;
    END_BLOCK();
    
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