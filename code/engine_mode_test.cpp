void
LoadShaderText(platform_api *Platform, memory_arena *ConstArena, char *FileType, u32 FileSize, u8 *DestShaderText)
{
    //u32 FileSize = ArrayCount(Test->ShaderText);
    
    platform_file_group *FileGroup = Platform->GetAllFilesOfTypeBegin(FileType);
    u32 FileCount = FileGroup->FileCount;
    FileCount = 1;
    platform_file_handle *FileHandles = PushArray(ConstArena, FileCount, platform_file_handle);
    for(u32 FileIndex = 0;
        FileIndex < FileCount;
        FileIndex++)
    {
        platform_file_handle *FileHandle = FileHandles + FileIndex;
        FileHandle = Platform->OpenNextFile(FileGroup);
        
        if(FileHandle->NoErrors)
        {
            //u8 *ReadContents = (u8 *)PushSize(ConstArena, FileSize);
            //u8 *ReadContents = Test->ShaderText;
            Platform->ReadDataFromFile(FileHandle, 0, FileSize, DestShaderText);
        }
        else
        {
            InvalidCodePath;
        }
    }
}

void
CompileShader(opengl *Opengl, GLuint Type, u8 *ShaderText, u32 *DestShader)
{
    if(*DestShader != 0)
    {
        Opengl->glDeleteShader(*DestShader);
    }
    
    *DestShader = Opengl->glCreateShader(Type);
    Opengl->glShaderSource(*DestShader, 1, &(GLchar *)ShaderText, NULL);
    Opengl->glCompileShader(*DestShader);
    
    GLint Ok;
    GLchar LogInfo[2000];
    Opengl->glGetShaderiv(*DestShader, GL_COMPILE_STATUS, &Ok);
    if(!Ok)
    {
        Opengl->glGetShaderInfoLog(*DestShader, 2000, NULL, LogInfo);
        Opengl->glDeleteShader(*DestShader);
        *DestShader = 0;
        Log->Add("[opengl]: shader compilaton error (info below):\n%s", LogInfo);
    }
    else
    {
        Log->Add("[opengl]: shader successfully compiled\n");
        
    }
}

void
LinkShaderProgram(opengl *Opengl, u32 Shader, u32 *DestProgram)
{
    if(Shader != 0)
    {
        if(*DestProgram != 0)
        {
            Opengl->glDeleteProgram(*DestProgram);
        }
        
        *DestProgram = Opengl->glCreateProgram();
        //glAttachShader(ShaderProgram, ShaderVert);
        Opengl->glAttachShader(*DestProgram, Shader);
        Opengl->glLinkProgram(*DestProgram);
        
        GLint Ok;
        GLchar LogInfo[2000];
        Opengl->glGetProgramiv(*DestProgram, GL_LINK_STATUS, &Ok);
        if(!Ok)
        {
            Opengl->glGetProgramInfoLog(*DestProgram, 2000, NULL, LogInfo);
            Log->Add("[opengl]: shader program linking error (info below):\n%s", LogInfo);
        }
        else
        {
            Log->Add("[opengl]: shader program successfully linked\n");
        }
    }
    else
    {
        Log->Add("[opengl]: shader program can't link with null shader\n");
    }
}

void
UpdateAndRenderTest(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    test *Test = &GameState->Test;
    
    renderer_frame *Frame = &Memory->Frame;
    opengl *Opengl = &Frame->Opengl;
    memory_arena *ConstArena = &GameState->ConstArena;
    platform_api *Platform = &Memory->PlatformAPI;
    
    if(!Test->IsInitialized)
    {
        Test->ClearColor = {0, 0, 0, 1};
        
        Test->UseShaderProgram = true;
        Test->ShaderProgram = 0;
        
        LoadShaderText(Platform, ConstArena, "frag", ArrayCount(Test->ShaderText), Test->ShaderText);
        CompileShader(Opengl, GL_FRAGMENT_SHADER, Test->ShaderText, &Test->Shader);
        LinkShaderProgram(Opengl, Test->Shader, &Test->ShaderProgram);
        
        
        Test->IsInitialized = true;
    }
    
    Frame->Opengl.UseShaderProgram = Test->UseShaderProgram;
    Frame->Opengl.ShaderProgram = Test->ShaderProgram;
    
    PushClear(&Memory->Frame, Test->ClearColor);
    PushRectOnGround(&Memory->Frame, V3(0, 0, 0), V2(5, 5), V4(1, 0, 0, 1));
    
#if ENGINE_INTERNAL
    //renderer_frame *Frame = &Memory->Frame;
    imgui *ImGuiHandle = &Frame->ImGuiHandle;
    if(ImGuiHandle->ShowImGuiWindows)
    {
        ImGui::Begin("Shader");
        ImGui::Text("Debug window for mode test shader...");
        
        ImGui::Text("Shader = %d", Test->Shader);
        ImGui::Text("Program = %d", Test->ShaderProgram);
        
        static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
        // InputTextMultiline(label, text, bufferSize, [w=0, h=0, ImGuiInputTextFlags=0])
        ImGui::InputTextMultiline("##source", (char *)Test->ShaderText, IM_ARRAYSIZE(Test->ShaderText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 32), flags);
        if(ImGui::Button("Recompile shader and relink shader program"))
        {
            CompileShader(Opengl, GL_FRAGMENT_SHADER, Test->ShaderText, &Test->Shader);
            LinkShaderProgram(Opengl, Test->Shader, &Test->ShaderProgram);
        }
        ImGui::End();
    }
#endif
}