void
OpenglDrawRectOnGround(rectangle2 R, r32 Z, v3 Color)
{
    r32 VRectangle[] = {
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
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void
OpenglDrawRectOutlineOnGround(rectangle2 R, r32 Z, v3 Color, r32 LineWidth)
{
    r32 VRectangle[] = {
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
    
    glLineWidth(LineWidth);
    glEnable(GL_LINE_SMOOTH);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
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
    
    /*r32 MinX = P.x - 0.5f * Dim.x;
    r32 MinY = P.y - 0.5f * Dim.y;
    r32 MaxX = P.x + 0.5f * Dim.x;
    r32 MaxY = P.y + 0.5f * Dim.y;*/
    r32 MinX = P.x;
    r32 MaxX = P.x + Dim.x;
    r32 MinY = P.y;
    r32 MaxY = P.y + Dim.y;
    r32 MinZ = 0.0;
    
    r32 VRectangle[] =
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
    
    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glTexCoordPointer(2, GL_FLOAT, 0, TexRectangle);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
}

struct frame_vertex
{
    v3 Pos;
    v2 TexCoord;
};

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
    Frame->Camera.P.z = 10.0f;
    
    // NOTE(ezexff): Init skybox
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
    
    // NOTE(ezexff): Init screen
    Opengl->glGenFramebuffers(1, &Frame->FBO);
    glGenTextures(1, &Frame->ColorTexture);
    glGenTextures(1, &Frame->DepthTexture);
    
    // NOTE(ezexff): Init screen VAO, VBO, EBO
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

void
OpenglBeginFrame(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    
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
}

void
OpenglEndFrame(renderer_frame *Frame)
{
    opengl *Opengl = &Frame->Opengl;
    camera *Camera = &Frame->Camera;
    
    glEnable(GL_DEPTH_TEST);
    
    glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, Frame->FBO);
    
    glClearColor(0.5, 0.5, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    r32 AspectRatio = (r32)Frame->Dim.x / (r32)Frame->Dim.y;
    r32 FOV = 0.1f;
    //r32 FOV = 1.0f;
    
    // Proj
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);
    
    // View
    glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
    glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
    glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
    //glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
    // TODO(me): think about real z for camera 
    v2 WorldOrigin = {};
    glTranslatef(-WorldOrigin.x, -WorldOrigin.y, -Frame->CameraZ);
    
    // NOTE(ezexff): Draw push buffer
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
                
                //glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
                //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_on_ground:
            {
                renderer_entry_rect_on_ground *Entry = (renderer_entry_rect_on_ground *)Data;
                OpenglDrawRectOnGround({Entry->P, Entry->P + Entry->Dim}, 0.0f, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_rect_outline_on_ground:
            {
                renderer_entry_rect_outline_on_ground *Entry = (renderer_entry_rect_outline_on_ground *)Data;
                OpenglDrawRectOutlineOnGround({Entry->P, Entry->P + Entry->Dim}, 0.0f, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z), Entry->LineWidth);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            case RendererEntryType_renderer_entry_bitmap_on_ground:
            {
                renderer_entry_bitmap_on_ground *Entry = (renderer_entry_bitmap_on_ground *)Data;
                OpenglDrawBitmapOnGround(&Frame->Opengl, Entry->Bitmap, Entry->P, Entry->Dim, Entry->Repeat);
                
                BaseAddress += sizeof(*Entry);
            } break;
            
            InvalidDefaultCase;
        }
    }
    
#if 1
    // NOTE(ezexff): Skybox
    {
        glDepthFunc(GL_LEQUAL);
        Opengl->glUseProgram(Frame->SkyboxProgram.ID);
        
        // Proj
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);
        r32 MatProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "Proj"), 1, GL_FALSE, MatProj);
        
        // View
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
        glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
        glRotatef(-Camera->Angle.z, 0.0f, 1.0f, 0.0f);
        glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        r32 MatView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "View"), 1, GL_FALSE, MatView);
        
        // Model
        m4x4 MatModel = Identity();
        MatModel = Translate(Camera->P) * XRotation(90) * Scale(200);
        MatModel = Transpose(MatModel); // opengl to glsl format
        Opengl->glUniformMatrix4fv(Opengl->glGetUniformLocation(Frame->SkyboxProgram.ID, "Model"), 1, GL_FALSE, (const GLfloat *)&MatModel);
        
        // Draw
        Opengl->glBindVertexArray(Frame->SkyboxVAO);
        Opengl->glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, Frame->SkyboxTexture);
        Opengl->glUniform1i(Opengl->glGetUniformLocation(Frame->Program.ID, "SkyboxTexture"), 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 36);
        Opengl->glBindVertexArray(0);
        
        Opengl->glUseProgram(0);
        glDepthFunc(GL_LESS);
    }
#endif
    
    Opengl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    
#if 1
    // NOTE(ezexff): Draw frame texture
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
#else
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, Frame->Dim.x, Frame->Dim.y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Frame->Dim.x, 0, Frame->Dim.y, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    
    r32 MinX = 0.0f;
    r32 MaxX = (r32)Frame->Dim.x;
    r32 MinY = 0.0f;
    r32 MaxY = (r32)Frame->Dim.y;
    
    r32 VRect[] = {
        MinX, MinY, // 0
        MaxX, MinY, // 1
        MaxX, MaxY, // 2
        MinX, MaxY  // 3
    };
    
    r32 Repeat = 1.0f;
    r32 UVRect[] = {
        0,      0,      // 0
        Repeat, 0,      // 1
        Repeat, Repeat, // 2
        0,      Repeat  // 3
    };
    
    glEnable(GL_TEXTURE_2D);
    Opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Frame->RenderTexture);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, 0, VRect);
    glTexCoordPointer(2, GL_FLOAT, 0, UVRect);
    glDrawArrays(GL_QUADS, 0, 4);
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    glDisable(GL_TEXTURE_2D);
#endif
    
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
    
    // NOTE(ezexff): Clear push buffer memory
    while(Frame->PushBufferSize--)
    {
        Frame->PushBufferMemory[Frame->PushBufferSize] = 0;
    }
    Frame->PushBufferSize = 0;
}