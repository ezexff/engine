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
OpenglInit(renderer_frame *Frame)
{
    // NOTE(ezexff): Init push buffer
    Frame->MaxPushBufferSize = sizeof(Frame->PushBufferMemory);
    Frame->PushBufferBase = Frame->PushBufferMemory;
    
    // NOTE(ezexff): Init camera
    Frame->Camera.P.z = 10.0f;
}

void
OpenglBeginFrame(renderer_frame *Frame)
{
    glViewport(0, 0, Frame->Width, Frame->Height);
}

void
OpenglEndFrame(renderer_frame *Frame)
{
    if(Frame->Opengl.UseShaderProgram)
    {
        Frame->Opengl.glUseProgram(Frame->Opengl.ShaderProgram);
    }
    
    r32 AspectRatio = (r32)Frame->Width / (r32)Frame->Height;
    r32 FOV = 0.1f;
    glEnable(GL_DEPTH_TEST);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);
    
    camera *Camera = &Frame->Camera;
    
    // NOTE(ezexff): Set camera
    glRotatef(-Camera->Angle.x, 1.0f, 0.0f, 0.0f);
    glRotatef(-Camera->Angle.y, 0.0f, 0.0f, 1.0f);
    //v2 WorldOrigin = {};
    glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
    
    // NOTE(ezexff): Draw info from push buffer
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
                
                glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                
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
            
            InvalidDefaultCase;
        }
    }
    
    if(Frame->Opengl.UseShaderProgram)
    {
        Frame->Opengl.glUseProgram(0);
    }
    
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