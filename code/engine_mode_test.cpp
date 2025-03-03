#include "engine_ui_core.cpp"
#include "engine_ui_widgets.cpp"

/* 
ui_node *UI_PushParent(ui_node *Node)
{
    //CurrentParent = Node;
    return(0);
}

ui_node *UI_PopParent(void)
{
    //CurrentParent = 0;
    return(0);
}
 */

/* 
inline void UI_AddFlags(ui_node *Node, u32 Flag)
{
    Node->StateFlags |= Flag;
}

inline void UI_ClearFlags(ui_node *Node, u32 Flag)
{
    Node->StateFlags &= ~Flag;
}

inline b32 UI_IsSet(ui_node *Node, u32 Flag)
{
    b32 Result = Node->StateFlags & Flag;
    return(Result);
}

inline b32 UI_IsSet(ui_node_style *Style, u32 Flag)
{
    b32 Result = Style->Flags & Flag;
    return(Result);
}
 */

#if 0
ui_node *UI_AddNode(u32 Flags, char *String)
{
    r32 LeftPadding = 0.0f;
    r32 TopPadding = 0.0f;
    r32 PaddingY = 2.0f;
    v2 Min = V2(LeftPadding, TopPadding);
    v2 Max = UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex].FixedSize;
    
    ui_node *NewNode = PushStruct(UI_State->TranArena, ui_node);
    ui_node_style *Style = &NewNode->Style;
    Style->Flags = Flags;
    NewNode->String = PushString(UI_State->TranArena, String);
    
    NewNode->Parent = UI_State->Root;
    if(!UI_State->Root->First)
    {
        UI_State->Root->First = NewNode;
        NewNode->Rect = {Min, UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex].FixedSize};
        
        if(IsSet(Style, UI_NodeStyleFlag_DrawText))
        {
            //Style->BackgroundColor = Template->BackgroundColor;
        }
    }
    else
    {
        NewNode->Prev = UI_State->Root->Last;
        UI_State->Root->Last->Next = NewNode;
        
        if(NewNode->Parent->LayoutAxis == Axis2_Y)
        {
            Min.y = NewNode->Prev->Rect.Max.y + PaddingY;
            Max.y = Min.y + UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex].FixedSize.y;
            
            //if(Max.y > NewNode->Parent->Rect.Max.y)
            if(!IsInRectangle(NewNode->Parent->Rect, Min) || !IsInRectangle(NewNode->Parent->Rect, Max))
            {
                InvalidCodePath;
            }
        }
        NewNode->Rect = {Min, Max};
    }
    
    UI_State->Root->Last = NewNode;
    
    return(NewNode);
}
#endif

internal void
UI_TestWindow(char *String, b32 *Value)
{
    r32 MinWindowWidth = 100.0f;
    r32 MinBodyHeight = 20.0f;
    local v2 P = {};
    local v2 Size = {};
    local b32 IsExpanded = true;
    
    UI_UseStyleTemplate(UI_StyleTemplate_Window);
    ui_node *Window = UI_AddNodeVer2(UI_State->Root, UI_NodeFlag_Floating,
                                     //UI_NodeFlag_DrawBackground|
                                     //UI_NodeFlag_DrawBorder,
                                     Concat(UI_State->TranArena, "Window#", String));
    Window->LayoutAxis = Axis2_Y;
    Window->Rect.Min = Window->Rect.Min + P;
    Window->Rect.Max = Window->Rect.Max + P;
    Window->Rect.Max.x = Window->Rect.Max.x + Size.x;
    r32 WindowWidth = Window->Rect.Max.x - Window->Rect.Min.x;
    if(WindowWidth < MinWindowWidth)
    {
        Window->Rect.Max.x = Window->Rect.Min.x + MinWindowWidth;
    }
    u32 WindowState = UI_GetNodeState(Window);
    
    // NOTE(ezexff): Title
    UI_UseStyleTemplate(UI_StyleTemplate_WindowTitle);
    ui_node *Title = UI_AddNodeVer2(Window,
                                    UI_NodeFlag_DrawBorder|
                                    UI_NodeFlag_DrawBackground,
                                    Concat(UI_State->TranArena, "Title#", String));
    u32 TitleState = UI_GetNodeState(Title);
    Title->LayoutAxis = Axis2_X;
    Title->Padding = 5.0f;
    
    // NOTE(ezexff): Title content
    {
        // NOTE(ezexff): Expand button
        char *ExpandString = ">";
        if(IsExpanded)
        {
            ExpandString = "v";
        }
        UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleExitButton);
        ui_node *ExpandButton = UI_AddNodeVer2(Title,
                                               UI_NodeFlag_Clickable|
                                               UI_NodeFlag_DrawBorder|
                                               UI_NodeFlag_DrawText|
                                               UI_NodeFlag_DrawBackground|
                                               UI_NodeFlag_HotAnimation|
                                               UI_NodeFlag_ActiveAnimation,
                                               Concat(UI_State->TranArena, 
                                                      Concat(UI_State->TranArena, ExpandString, "#"), String));
        u32 ExpandButtonState = UI_GetNodeState(ExpandButton);
        if(UI_IsPressed(ExpandButtonState))
        {
            IsExpanded = !IsExpanded;
            
            Log->Add("IsExpanded=%d\n", IsExpanded);
            Log->Add("ExpandButtonState=%d\n", ExpandButtonState);
        }
        
        // NOTE(ezexff): Label
        UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *TitleLabel = UI_AddNodeVer2(Title,
                                             UI_NodeFlag_DrawBorder|
                                             UI_NodeFlag_DrawText,
                                             Concat(UI_State->TranArena, 
                                                    Concat(UI_State->TranArena, String, "#"), String));
        // NOTE(ezexff): Empty space
        UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleEmptySpace);
        ui_node *TitleEmptySpace = UI_AddNodeVer2(Title,
                                                  UI_NodeFlag_Clickable|
                                                  UI_NodeFlag_DrawBorder|
                                                  UI_NodeFlag_DrawBackground,
                                                  Concat(UI_State->TranArena, "EmptySpace#", String));
        
        // NOTE(ezexff): Exit button
        UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleExitButton);
        ui_node *ExitButton = UI_AddNodeVer2(Title,
                                             UI_NodeFlag_Clickable|
                                             UI_NodeFlag_DrawBorder|
                                             UI_NodeFlag_DrawText|
                                             UI_NodeFlag_DrawBackground|
                                             UI_NodeFlag_HotAnimation|
                                             UI_NodeFlag_ActiveAnimation,
                                             Concat(UI_State->TranArena, "x#", String));
        
        // NOTE(ezexff): Post widgets work
        r32 ExitButtonWidth = ExitButton->Rect.Max.x - ExitButton->Rect.Min.x;
        r32 TitleEmptySpaceWidth = Title->Rect.Max.x - TitleLabel->Rect.Max.x - ExitButtonWidth;
        TitleEmptySpace->Rect.Min.x = TitleLabel->Rect.Max.x;
        TitleEmptySpace->Rect.Max.x = TitleEmptySpace->Rect.Min.x + TitleEmptySpaceWidth;
        ExitButton->Rect.Min.x = TitleEmptySpace->Rect.Max.x;
        ExitButton->Rect.Max.x = ExitButton->Rect.Min.x + ExitButtonWidth;
        u32 TitleExitState = UI_GetNodeState(ExitButton);
        if(UI_IsPressed(TitleExitState))
        {
            *Value = !*Value;
        }
        
        UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleEmptySpace);
        u32 TitleEmptySpaceState = UI_GetNodeState(TitleEmptySpace);
        if(UI_IsDragging(TitleEmptySpaceState))
        {
            P.x += UI_State->Input->dMouseP.x;
            P.y -= UI_State->Input->dMouseP.y;
        }
    }
    
    // NOTE(ezexff): Body
    if(IsExpanded)
    {
        UI_UseStyleTemplate(UI_StyleTemplate_WindowBody);
        ui_node *Body = UI_AddNodeVer2(Window,
                                       UI_NodeFlag_DrawBorder|
                                       UI_NodeFlag_DrawBackground,
                                       Concat(UI_State->TranArena, "WindowBody#", String));
        Body->Rect.Max.y = Body->Rect.Max.y + Size.y;
        r32 BodyHeight = Body->Rect.Max.y - Body->Rect.Min.y;
        if(BodyHeight < MinBodyHeight)
        {
            Body->Rect.Max.y = Body->Rect.Min.y + MinBodyHeight;
        }
        Body->Spacing = 5.0f;
        Body->Padding = 5.0f;
        Body->LayoutAxis = Axis2_Y;
        u32 BodyState = UI_GetNodeState(Body);
        
        // NOTE(ezexff):
        char *TestLabelString = "fkdshjfiusdfls43gfd#5435435";
        UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *TestLabel = UI_AddNodeVer2(Body,
                                            //UI_NodeFlag_DrawBackground|
                                            UI_NodeFlag_DrawBorder|
                                            UI_NodeFlag_DrawText,
                                            Concat(UI_State->TranArena, 
                                                   Concat(UI_State->TranArena, TestLabelString, "#"), String));
        
        char *TestButtonString = "Btn1";
        UI_UseStyleTemplate(UI_StyleTemplate_Button);
        ui_node *TestButton = UI_AddNodeVer2(Body,
                                             UI_NodeFlag_Clickable|
                                             UI_NodeFlag_DrawBorder|
                                             UI_NodeFlag_DrawText|
                                             UI_NodeFlag_DrawBackground|
                                             UI_NodeFlag_HotAnimation|
                                             UI_NodeFlag_ActiveAnimation,
                                             Concat(UI_State->TranArena, 
                                                    Concat(UI_State->TranArena, TestButtonString, "#"), String));
        u32 State = UI_GetNodeState(TestButton);
        
        char *TestButtonString2 = "Btn2";
        UI_UseStyleTemplate(UI_StyleTemplate_Button);
        ui_node *TestButton2 = UI_AddNodeVer2(Body,
                                              UI_NodeFlag_Clickable|
                                              UI_NodeFlag_DrawBorder|
                                              UI_NodeFlag_DrawText|
                                              UI_NodeFlag_DrawBackground|
                                              UI_NodeFlag_HotAnimation|
                                              UI_NodeFlag_ActiveAnimation,
                                              Concat(UI_State->TranArena, 
                                                     Concat(UI_State->TranArena, TestButtonString2, "#"), String));
        u32 State2 = UI_GetNodeState(TestButton2);
        
        
        char FpsString[16];
        r32 Fps = 1 / UI_State->Input->dtForFrame;
        _snprintf_s(FpsString, sizeof(FpsString), "FPS = %d", (u32)Fps);
        UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *TestLabel2 = UI_AddNodeVer2(Body,
                                             //UI_NodeFlag_DrawBackground|
                                             UI_NodeFlag_DrawBorder|
                                             UI_NodeFlag_DrawText,
                                             Concat(UI_State->TranArena, 
                                                    Concat(UI_State->TranArena, FpsString, "#"), String));
        
        char MsString[32];
        r32 Ms = UI_State->Input->dtForFrame * 1000.0f;
        _snprintf_s(MsString, sizeof(MsString), "MS = %fms/frame", Ms);
        UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *TestLabel3 = UI_AddNodeVer2(Body,
                                             //UI_NodeFlag_DrawBackground|
                                             UI_NodeFlag_DrawBorder|
                                             UI_NodeFlag_DrawText,
                                             Concat(UI_State->TranArena, 
                                                    Concat(UI_State->TranArena, MsString, "#"), String));
        
        
        
        UI_UseStyleTemplate(UI_StyleTemplate_WindowResizeButton);
        ui_node *ResizeButton = UI_AddNodeVer2(Body,
                                               UI_NodeFlag_Floating|
                                               UI_NodeFlag_Clickable|
                                               UI_NodeFlag_DrawBorder|
                                               UI_NodeFlag_DrawBackground|
                                               UI_NodeFlag_HotAnimation|
                                               UI_NodeFlag_ActiveAnimation,
                                               Concat(UI_State->TranArena, "ResizeButton#", String));
        //v2 WindowRightBottomCorner = V2(Window->Rect.Max.x, Window->Rect.Max.y + Title->Rect.Max.y - Title->Rect.Min.y);
        v2 BodyRightBottomCorner = V2(Body->Rect.Max.x, Body->Rect.Max.y);
        ResizeButton->Rect.Min = V2(BodyRightBottomCorner.x - 10.0f, BodyRightBottomCorner.y - 10.0f);
        ResizeButton->Rect.Max = BodyRightBottomCorner;
        u32 ResizeButtonState = UI_GetNodeState(ResizeButton);
        if(UI_IsDragging(ResizeButtonState))
        {
            v2 NewSize = {};
            NewSize.x += UI_State->Input->dMouseP.x;
            NewSize.y -= UI_State->Input->dMouseP.y;
            
            v2 NewWindowDim = GetDim(Window->Rect) + NewSize;
            if(NewWindowDim.x > MinWindowWidth)
            {
                Size.x += NewSize.x;
            }
            if(NewWindowDim.x > MinBodyHeight)
            {
                Size.y += NewSize.y;
            }
        }
    }
}

/* 
internal void
UI_WindowEnd(char *String, b32 *Value)
{
}
 */
/* 
void UI_DrawRect(ui_node *Element)
{
    
}
 */

/* 
ui_layout UI_MakeLayout(v2 Pos, v2 Dim)
{
    ui_layout Result = {};
    return(Result);
}

void UI_PushLayout(ui_layout *Layout)
{
    LastLayout = PushStruct(UI_TranArena, ui_layout);
}
void UI_PopLayout()
{
    LastLayout = 0;
}

void UI_DrawLayout(renderer_push_buffer *PushBufferUI, ui_layout Layout)
{
    //PushRectOnScreen(PushBufferUI, Layout.Pos, Layout.Dim, , 1);
}
 */

/* 
struct ui_state
{
    b32 IsInitialized;
        // TODO(ezexff): 
}
 */

void
UpdateAndRenderTest(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_test *ModeTest = &GameState->ModeTest;
    if(!ModeTest->IsInitialized)
    {
        UI_Init(&GameState->ConstArena, &TranState->TranArena);
        
        ModeTest->IsInitialized = true;
    }
    /* 
        UI_RectSetinel.Rect = {V2(0, 0), V2((r32)Frame->Dim.x, (r32)Frame->Dim.y)};
        UI_RectSetinel.BackgroundColor = V4(0, 1, 0, 1);
     */
    
    //ui_node *Node = UI_NodeCreate(UI_NodeStyleFlag_DrawBackground, String);
    //InvalidCodePath;
    // NOTE(ezexff): UI
    
    BEGIN_BLOCK("UI_TEST");
    {
        UI_BeginFrame(GameState, TranState, Frame, Input);
        
        /*         
                local u32 FooCount = 0;
                if(UI_IsClicked(UI_Button("Foo")))
                {
                    FooCount++;
                    Log->Add("[ui] Button Foo was clicked\n");
                }
                
                char TextBuffer[256];
                _snprintf_s(TextBuffer, sizeof(TextBuffer), "%d", FooCount);
                UI_Label(TextBuffer);
                
                if(UI_IsClicked(UI_Button("Bar")))
                {
                    Log->Add("[ui] Button Bar was clicked\n");
                }
                
                if(UI_IsClicked(UI_Button("Baz")))
                {
                    Log->Add("[ui] Button Baz was clicked\n");
                }
                
                UI_Label("Text13213");
                
                local b32 Checkbox = false;
                UI_Checkbox("Test", &Checkbox);
                _snprintf_s(TextBuffer, sizeof(TextBuffer), "Box=%d", Checkbox);
                UI_Label(TextBuffer);
         */
        
        local b32 IsWindowVisible = true;
        /* 
                if(UI_IsClicked(UI_Button("WndVis")))
                {
                    IsWindowVisible = !IsWindowVisible;
                }
         */
        
        if(IsWindowVisible)
        {
            //UI_TestWindow("Debug", &IsWindowVisible);
            
            UI_BeginWindow("DebugTest", &IsWindowVisible);
            {
                char FpsString[16];
                r32 Fps = 1 / UI_State->Input->dtForFrame;
                _snprintf_s(FpsString, sizeof(FpsString), "FPS = %d", (u32)Fps);
                UI_Label(FpsString);
                
                char MsString[32];
                r32 Ms = UI_State->Input->dtForFrame * 1000.0f;
                _snprintf_s(MsString, sizeof(MsString), "MS = %fms/frame", Ms);
                UI_Label(MsString);
                
                if(UI_IsPressed(UI_Button("Btn12345")))
                {
                    Log->Add("action when Btn12345 was pressed\n");
                }
            }
            UI_EndWindow();
        }
        
        
        
        UI_EndFrame();
    }
    END_BLOCK();
    
    /* 
            bitmap_id BitmapID = GetFirstBitmapFrom(TranState->Assets, Asset_DuDvMap);
            v2 Dim = V2(1000, 1000);
            v2 Pos = V2((r32)Frame->Dim.x - Dim.x / 2, -Dim.y / 2);
            
            PushBitmapOnScreen(Frame, TranState->Assets, BitmapID, Pos, Dim, 1.0f);
             */
    
    char *TestString = "The quick brown fox jumps over a lazy dog.";
    DEBUGTextLine(Frame, TranState->Assets, TestString, {100, 100});
    
    /* 
        renderer_frame *Frame = &Memory->Frame;
        
        mode_test *ModeTest = &GameState->ModeTest;
        
        memory_arena *ConstArena = &GameState->ConstArena;
        
        if(!ModeTest->IsInitialized)
        {
            ModeTest->ClearColor = {0, 0, 0, 1};
            
            ModeTest->IsInitialized = true;
        }
        
        Clear(Frame, ModeTest->ClearColor);
        PushRectOnScreen(Frame, V2(0, 0), V2(1000, 50), V4(1, 0, 0, 1));
        
    #if ENGINE_INTERNAL
        imgui *ImGuiHandle = &Memory->ImGuiHandle;
    #endif
     */
}