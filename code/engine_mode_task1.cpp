internal bool DisableButton = false;
internal bool BeginDisabledButton = false;

inline b32
IsNumber(wchar_t C)
{
    b32 Result = ((C >= L'0') && (C <= L'9'));
    
    return(Result);
}

inline void
CheckHResult(HRESULT hr, char* message)
{
    if(FAILED(hr))
    {
        //wprintf(L"Error: %s (HRESULT: 0x%08X)\n", message, hr);
        Log->Add("Error: %s (HRESULT: 0x%08X)\n", message, hr);
    }
}

internal void
ParseKML(mode_task1 *ModeTask1, char *FileName)
{
    HRESULT hr = S_OK;
    IXmlReader* pReader = NULL;
    IStream* pFileStream = NULL;
    
    // Create a file stream for the XML file
    hr = SHCreateStreamOnFile(FileName, STGM_READ, &pFileStream);
    CheckHResult(hr, "Failed to create file stream.");
    
    // Create the XmlReader
    hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL);
    CheckHResult(hr, "Failed to create XmlReader.");
    
    // Set the input stream for the reader
    hr = pReader->SetInput(pFileStream);
    CheckHResult(hr, "Failed to set input stream.");
    
    XmlNodeType nodeType;
    const WCHAR* pwszLocalName = NULL;
    const WCHAR* pwszValue = NULL;
    
    b32 NameFound = false;
    // Read through the XML nodes
    while(S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch(nodeType)
        {
            //~
            case XmlNodeType_XmlDeclaration:
            {
                Log->Add("XmlDeclaration\n");
            } break;
            
            //~
            case XmlNodeType_Element:
            {
                hr = pReader->GetLocalName(&pwszLocalName, NULL);
                CheckHResult(hr, "Failed to get local name for element.");
                //Log->Add("%sElement: %s\n", SpaceBuffer, wideCharToUtf8(pwszLocalName).c_str());
            } break;
            
            //~
            case XmlNodeType_Text:
            {
                UINT cwchValue = 0;
                hr = pReader->GetValue(&pwszValue, &cwchValue);
                CheckHResult(hr, "Failed to get value for text node.");
                //Log->Add("%sText: %s\n", SpaceBuffer, wideCharToUtf8(pwszValue).c_str());
                
                if((wcscmp(pwszLocalName, L"name") == 0) && (wcscmp(pwszValue, L"58:15:0491406:109") == 0))
                {
                    NameFound = true;
                    Log->Add("Found name\n");
                }
                else if(NameFound && (wcscmp(pwszLocalName, L"coordinates") == 0))
                {
                    Log->Add("Found coordinates by name\n");
                    u32 CoordinatesCount = 0;
                    wchar_t *EndCursor = (wchar_t *)pwszValue;
                    while(*EndCursor)
                    {
                        if(IsNumber(*EndCursor))
                        {
                            long double X = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L','
                            long double Y = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L','
                            long double Z = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L' '
                            //Log->Add("%lf %lf\n", X, Y);
                            
                            // TODO(ezexff): mb start use double? r32 for testing
                            //ModeTask1->CoordinateArray[CoordinatesCount].x = (r32)((X - 45.0f) * 100000.0f);
                            //ModeTask1->CoordinateArray[CoordinatesCount].y = (r32)((Y - 52.0f) * 100000.0f);
                            ModeTask1->CoordinateArray[CoordinatesCount].x = (r32)X;
                            ModeTask1->CoordinateArray[CoordinatesCount].y = (r32)Y;
                            Log->Add("%f %f\n",
                                     ModeTask1->CoordinateArray[CoordinatesCount].x, ModeTask1->CoordinateArray[CoordinatesCount].y);
                            
                            CoordinatesCount++;
                            Assert(CoordinatesCount < ArrayCount(ModeTask1->CoordinateArray));
                        }
                        else
                        {
                            ++EndCursor; // skip unknown symbol
                        }
                    };
                    ModeTask1->CoordinateArrayCount = CoordinatesCount;
                    Log->Add("Read %u cordinates\n", CoordinatesCount);
                    goto end_parse;
                }
            } break;
            
            //~
            case XmlNodeType_EndElement:
            {
                hr = pReader->GetLocalName(&pwszLocalName, NULL);
                CheckHResult(hr, "Failed to get local name for end element.");
                //Log->Add("%sEnd Element: %s\n", SpaceBuffer, wideCharToUtf8(pwszLocalName).c_str());
                
            } break;
            
            default:{} break;
        }
    }
    
    end_parse:
    
    if(hr != S_FALSE) // S_FALSE indicates end of document
    {
        CheckHResult(hr, "Error during XML reading.");
    }
    
    //wprintf(L"Finished reading XML.\n");
    Log->Add("Finished reading XML.\n");
    
    // Clean up
    if(pReader) pReader->Release();
    if(pFileStream) pFileStream->Release();
}

std::string wideCharToUtf8(const std::wstring& wstr)
{
    if(wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if(size_needed == 0)
    {
        // Handle error, e.g., GetLastError()
        return std::string();
    }
    
    std::string utf8_str(size_needed - 1, 0); // -1 to exclude null terminator from size
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], size_needed, nullptr, nullptr);
    return utf8_str;
}

/* 
inline b32
IsAlpha(char C)
{
    bool Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));
    
    return(Result);
}

inline char *
GetTag(char *Start)
{
    char *At = Start;
    while(At[0])
    {
        if((At == Start) && (At[1] == '/'))
        {
            // process close tag
            ++At;
        }
        else if(At == Start)
        {
            // process open tag
            // identifier  && (IsAlpha(*At))
            ++At;
            
        }
        else if(*At == '>')
        {
            break;
        }
        else
        {
            InvalidCodePath;
        }
        ++At;
    }
    
    return(At);
}

u32 ProcessOpen(char *At, char *Open, u32 OpenLen)
{
    while(*At)
    {
        if(*At == '>')
        {
            // tag ended
            OpenLen++;
            Open[OpenLen] = 0;
            return(OpenLen);
        }
        if(!IsAlpha(*At))
        {
            OpenLen = 0;
            break;
        }
        Open[OpenLen] = *At;
        OpenLen++;
        
        ++At;
    }
    
    return(0);
}
 */

inline r32
PerpendicularDistance(v2 p, v2 start, v2 end)
{
    r32 dx = end.x - start.x;
    r32 dy = end.y - start.y;
    r32 t = ((p.x - start.x) * dx + (p.y - start.y) * dy) / (dx * dx + dy * dy);
    
    if (t < 0) t = 0;
    else if (t > 1) t = 1;
    
    r32 closestX = start.x + t * dx;
    r32 closestY = start.y + t * dy;
    
    r32 Result = SquareRoot(Square(p.x - closestX) + Square(p.y - closestY));
    return(Result);
}

internal void
DouglasPeucker(v2 *PointArray, u32 PointArrayCount,
               r32 Epsilon, s32 Start, s32 End,
               v2 *ResultArray, u32 *ResultArrayCount)
{
    r32 MaxDistance = 0.0f;
    s32 FarthestIndex = -1;
    for(s32 Index = Start + 1;
        Index < End;
        ++Index)
    {
        r32 PerpDistance = PerpendicularDistance(PointArray[Index], PointArray[Start], PointArray[End]);
        if(PerpDistance > MaxDistance)
        {
            MaxDistance = PerpDistance;
            FarthestIndex = Index;
        }
    }
    
    if(MaxDistance > Epsilon)
    {
        DouglasPeucker(PointArray, PointArrayCount,
                       Epsilon, Start, FarthestIndex,
                       ResultArray, ResultArrayCount);
        DouglasPeucker(PointArray, PointArrayCount,
                       Epsilon, FarthestIndex, End,
                       ResultArray, ResultArrayCount);
    }
    else
    {
        if(*ResultArrayCount == 0)
        {
            ResultArray[*ResultArrayCount] = PointArray[Start];
            ++*ResultArrayCount;
        }
        ResultArray[*ResultArrayCount] = PointArray[End];
        ++*ResultArrayCount;
    }
}

struct douglas_peucker_work
{
    task_with_memory *Task;
    
    mode_task1 *ModeTask1;
};
internal PLATFORM_WORK_QUEUE_CALLBACK(DouglasPeuckerWork)
{
    douglas_peucker_work *Work = (douglas_peucker_work *)Data;
    
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    //ParseKML(ModeTask1, FileName);
#endif
#endif
    
    mode_task1 *ModeTask1 = Work->ModeTask1;
    if(ModeTask1)
    {
        for(u32 Index = 0;
            Index < 5;
            ++Index)
        {
            Log->Add("[enginework] DouglasPeuckerWork tick %u of 5\n", Index + 1);
            Sleep(1000);
        }
        
        DouglasPeucker(ModeTask1->CoordinateArray, ModeTask1->CoordinateArrayCount,
                       ModeTask1->Epsilon, 0, ModeTask1->CoordinateArrayCount - 1,
                       ModeTask1->SimplifiedArray, &ModeTask1->SimplifiedArrayCount);
        Log->Add("[enginework] DouglasPeuckerWork completed\n");
        DisableButton = false;
    }
    else
    {
        InvalidCodePath;
    }
    
    EndTaskWithMemory(Work->Task);
}

internal void
UpdateAndRenderTask1(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    memory_arena *ConstArena = &GameState->ConstArena;
    memory_arena *TranArena = &TranState->TranArena;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_task1 *ModeTask1 = &GameState->ModeTask1;
    if(!ModeTask1->IsInitialized)
    {
        //ModeTask1->Scale = 1.0f;
        // TODO(ezexff): only for testing
        Renderer->Camera.P = V3(45.577f, 52.539f, 35.0f);
        ModeTask1->Epsilon = 0.00001f;
        /* 
                ModeTask1->XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://earth.google.com/kml/2.1\"><Placemark><name>Геленджик</name><description><![CDATA[<p>Геленджик, Краснодарский край, Россия.</p>Город располагается по&amp;nbsp;берегам Геленджикской бухты, но&amp;nbsp;не&amp;nbsp;равномерно (восточный берег исторически более населён).]]></description><LookAt id=\"khLookAt540_copy0\"><longitude>38.0576198113139</longitude><latitude>44.56963150481845</latitude><altitude>0</altitude><range>14693.40972993507</range><tilt>49.10268313434742</tilt><heading>37.85562764777833</heading></LookAt><Style><IconStyle><scale>0.9</scale><Icon><href>root://icons/palette-4.png</href><x>32</x><y>128</y><w>32</w><h>32</h></Icon></IconStyle><LabelStyle><scale>0.9</scale></LabelStyle></Style><Point id=\"khPoint541_copy0\"><coordinates>38.06284424434902,44.56842733252498,0</coordinates></Point></Placemark></kml>";
         */
        /* 
                ModeTask1->XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document><Placemark><name>New York City</name><description>New York City</description><Point><coordinates>-74.006393,40.714172,0</coordinates></Point></Placemark></Document></kml>";
         */
        
        /* 
                char *At = ModeTask1->XML;
                while(*At)
                {
                    u32 OpenLen = 0;
                    char Open[256] = {};
                    u32 CloseLen = 0;
                    char Close[256] = {};
                    
                    // Ищем начало тега
                    if(*At == '<')
                    {
                        ++At;
                        if(*At == '\\')
                        {
                            // mb close tag
                            ProcessClose(At);
                            continue;
                        }
                        else if(IsAlpha(*At))
                        {
                            // mb open tag
                            if(ProcessOpen(At, Open, OpenLen))
                            {
                                Log->Add("Open: %s\n", Open);
                            }
                            continue;
                        }
                        Log->Add("Skip: %c\n", *At);
                    }
                    else
                    {
                        Log->Add("Skip: %c\n", *At);
                    }
                    
                    ++At;
                }
         */
        //TestParseXML(ModeTask1->XML);
        /* 
                ++At;
                
                GetIdentifier
                
                // Если идентификатор, то продолжаем разбор
                if(IsAlpha(At[1]))
                   {
                       
                }
                else
                {
                    
                }
                        At = GetTag(At);
 */
        
        // NOTE(ezexff): Test
        /* 
                ModeTask1->PointArrayCount = 5;
                ModeTask1->PointArray = PushArray(&GameState->ConstArena, ModeTask1->PointArrayCount, v2);
                ModeTask1->PointArrayTmp = PushArray(&GameState->ConstArena, ModeTask1->PointArrayCount, v2);
                ModeTask1->SimplifiedArray = PushArray(&GameState->ConstArena, 1000, v2);
                
                random_series Series = RandomSeed(300);
                for(u32 Index = 0;
                    Index < ModeTask1->PointArrayCount;
                    ++Index)
                {
                    ModeTask1->PointArray[Index].x = 500 + (r32)Index * 100;
                    ModeTask1->PointArray[Index].y = 500 + RandomUnilateral(&Series) * 100;
                }
         */
        
        ModeTask1->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    if(IsDown(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        if(Input->dMouseP.x != 0)
        {
            //Log->Add("dMouseP.x = %3.f\n", Input->dMouseP.x);
            Renderer->Camera.P.x -= Input->dMouseP.x / Renderer->Camera.P.z / 750.0f / Frame->AspectRatio;
        }
        if(Input->dMouseP.y != 0)
        {
            //Log->Add("dMouseP.y = %3.f\n", Input->dMouseP.y);
            Renderer->Camera.P.y -= Input->dMouseP.y / Renderer->Camera.P.z / 750.0f / Frame->AspectRatio; 
        }
    }
    /*
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
        {
            ModeTask1->SimplifiedArrayCount = 0;
            DouglasPeucker(ModeTask1->PointArray, &ModeTask1->PointArrayCount,
                           ModeTask1->Epsilon, 0, ModeTask1->PointArrayCount - 1,
                           ModeTask1->SimplifiedArray, &ModeTask1->SimplifiedArrayCount);
        }
        
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
        {
            ModeTask1->SimplifiedArrayCount = 0;
        }
     */
    
    if(UI_State->Input->dMouseP.z != 0)
    {
        Renderer->Camera.P.z += UI_State->Input->dMouseP.z;
        Renderer->Camera.P.z = Clamp(1.0f, Renderer->Camera.P.z, 1000.0f);
        //ModeTask1->Scale += Input->dMouseP.z;
        //ModeTask1->Scale = Clamp(1, ModeTask1->Scale, 10);
        /* 
                ModeTask1->CameraP.x = Input->MouseP.x;
                ModeTask1->CameraP.y = Frame->Dim.y - Input->MouseP.y;
         */
        /* 
                v2 Center = V2(Frame->Dim.x / 2.0f, Frame->Dim.y / 2.0f);
                ModeTask1->CameraP = ModeTask1->CameraP + Center - V2(Input->MouseP.x, Input->MouseP.y);
         */
    }
    
    // NOTE(ezexff): draw
    /* 
        v4 OutlineColor = V4(1, 0, 0, 1);
        r32 CircleScale = 50;
        v2 Pos = V2(Input->MouseP.x, Input->MouseP.y);
        PushCircleOutlineOnScreen(&Renderer->PushBufferPhysics, Pos, CircleScale, 2, OutlineColor, 10001);
     */
    
    // NOTE(ezexff): points in camera space
    /* 
        for(u32 Index = 0;
            Index < ModeTask1->PointArrayCount;
            ++Index)
        {
            ModeTask1->PointArrayTmp[Index] = ModeTask1->PointArray[Index] * ModeTask1->Scale + ModeTask1->CameraP;
        }
     */
    
    /* 
        PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->PointArrayCount, ModeTask1->PointArrayTmp, 3, V4(0, 0, 0, 1), 10000);
        
        PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->SimplifiedArrayCount, ModeTask1->SimplifiedArray, 3, V4(0, 1, 0, 1), 10000);
     */
    
    if(ModeTask1->CoordinateArrayCount)
    {
        PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->CoordinateArrayCount, ModeTask1->CoordinateArray, 1, V4(0, 0, 0, 1), 10000);
    }
    if(ModeTask1->SimplifiedArrayCount)
    {
        PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->SimplifiedArrayCount, ModeTask1->SimplifiedArray, 1, V4(0, 1, 0, 1), 10000);
    }
}