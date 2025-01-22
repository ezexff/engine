ui_state *UI_State;

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

ui_node *UI_AddNode(u32 Flags, char *String)
{
    ui_node *NewNode = PushStruct(UI_State->TranArena, ui_node);
    NewNode->Style.Flags = Flags;
    NewNode->String = PushString(UI_State->TranArena, String);
    
    NewNode->Parent = UI_State->Root;
    if(!UI_State->Root->First)
    {
        UI_State->Root->First = NewNode;
    }
    else
    {
        NewNode->Prev = UI_State->Root->Last;
        UI_State->Root->Last->Next = NewNode;
    }
    
    UI_State->Root->Last = NewNode;
    
    return(NewNode);
}

inline void AddFlags(ui_node *Node, u32 Flag)
{
    Node->StateFlags |= Flag;
}

inline b32 IsSet(ui_node_style *Style, u32 Flag)
{
    b32 Result = Style->Flags & Flag;
    return(Result);
}

u32 UI_GetNodeState(ui_node *Node)
{
    ui_style_template *Template = &UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex];
    ui_node_style *Style = &Node->Style;
    
    Node->FixedSize = Template->FixedSize;
    
    if(IsSet(Style, UI_NodeStyleFlag_DrawBackground))
    {
        Style->BackgroundColor = Template->BackgroundColor;
    }
    
    if(IsSet(Style, UI_NodeStyleFlag_Clickable))
    {
        v2 MouseP = V2((r32)UI_State->Input->MouseP.x, (r32)UI_State->Input->MouseP.y);
        rectangle2 Rect = {V2(0, 0), Node->FixedSize};
        if(IsInRectangle(Rect, MouseP))
        {
            AddFlags(Node, UI_NodeStateFlag_Hovering);
            //Log->Add("[ui] in button rect\n");
            
            Style->BackgroundColor = UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex].HoveringColor;
            if(WasPressed(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
            {
                AddFlags(Node, UI_NodeStateFlag_Clicked);
            }
            
            if(IsDown(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
            {
                AddFlags(Node, UI_NodeStateFlag_Pressed);
            }
        }
    }
    return(Node->StateFlags);
}

void UI_UseStyleTemplate(u32 Index)
{
    Assert(Index < UI_StyleTemplate_Count);
    UI_State->SelectedTemplateIndex = Index;
}

u32 UI_Button(char *String)
{
    ui_node *Node = UI_AddNode(UI_NodeStyleFlag_Clickable|
                               UI_NodeStyleFlag_DrawBorder|
                               UI_NodeStyleFlag_DrawText|
                               UI_NodeStyleFlag_DrawBackground|
                               UI_NodeStyleFlag_HotAnimation|
                               UI_NodeStyleFlag_ActiveAnimation,
                               String);
    UI_UseStyleTemplate(UI_StyleTemplate_Button);
    u32 State = UI_GetNodeState(Node);
    return(State);
}

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

internal void
UI_Init(memory_arena *TranArena)
{
    UI_State = PushStruct(TranArena, ui_state);
    
    // NOTE(ezexff): Style templates
    for(u32 Index = 0;
        Index < UI_StyleTemplate_Count;
        ++Index)
    {
        ui_style_template *StyleTemplate = &UI_State->StyleTemplateArray[Index];
        switch(Index)
        {
            case UI_StyleTemplate_Default:
            {
                StyleTemplate->BackgroundColor = V4(0.5f, 0, 0.5f, 1);
                //StyleTemplate.HoveringColor = V4(0, 1, 1, 1);
                /* 
                                StyleTemplate.ClickedColor = V4(0, 0, 0, 1);
                                StyleTemplate.PressedColor = V4(0, 0, 0, 1);
                 */
                StyleTemplate->FixedSize = V2(100, 100);
            } break;
            
            case UI_StyleTemplate_Button:
            {
                StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                /* 
                                StyleTemplate.ClickedColor = V4(0, 0, 0, 1);
                                StyleTemplate.PressedColor = V4(0, 0, 0, 1);
                 */
                StyleTemplate->FixedSize = V2(70, 50);
            } break;
            
            InvalidDefaultCase;
        }
    }
}

internal void
UI_BeginFrame(memory_arena *TranArena, renderer_frame *Frame, game_input *Input)
{
    // TODO(ezexff): Mb rework (do without engine services)?
    UI_State->TranArena = TranArena;
    UI_State->Frame = Frame;
    UI_State->Input = Input;
    
    UI_State->FrameMemory = BeginTemporaryMemory(UI_State->TranArena);
    
    // NOTE(ezexff): Root ui_node
    UI_State->Root = PushStruct(UI_State->TranArena, ui_node);
    UI_State->Root->First = 0;
    UI_State->Root->Last = 0;
    UI_State->Root->Next = 0;
    UI_State->Root->Prev = 0;
    UI_State->Root->String = PushString(UI_State->TranArena, "Root");
    //UI_State->Root->FixedSizeRect = {V2(0, 0), V2((r32)UI_State->Frame->Dim.x, (r32)UI_State->Frame->Dim.y)};
    UI_State->Root->FixedSize = V2((r32)UI_State->Frame->Dim.x, (r32)UI_State->Frame->Dim.y);
    UI_State->Root->Style.Flags = UI_NodeStyleFlag_DrawBackground;
    UI_State->Root->Style.BackgroundColor = V4(0.5f, 0, 0.5f, 1);
}

internal void
UI_DrawNodeTree(ui_node *Node)
{
    /* 
        for(ui_node *CurrentNode = Node;
            CurrentNode != 0;
            CurrentNode = Node->Next)
     */
    ui_node_style *Style = &Node->Style;
    if(IsSet(Style, UI_NodeStyleFlag_DrawBackground))
    {
        renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
        rectangle2 Rect = {V2(0, 0), Node->FixedSize};
        PushRectOnScreen(&Renderer->PushBufferUI, Rect.Min, Rect.Max, Style->BackgroundColor, 100);
    }
    
    // NOTE(ezexff): Process childs
    if(Node->First)
    {
        for(ui_node *ChildNode = Node->First;
            ChildNode != 0;
            ChildNode = ChildNode->Next)
        {
            UI_DrawNodeTree(ChildNode);
            /* 
                        ui_node_style *ChildNodeStyle = &ChildNode->Style;
                        renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
                        PushRectOnScreen(&Renderer->PushBufferUI, ChildNode->FixedSizeRect.Min, ChildNode->FixedSizeRect.Max, ChildNodeStyle->BackgroundColor, 100);
             */
        }
    }
}

internal void
UI_EndFrame()
{
    UI_DrawNodeTree(UI_State->Root);
    /* 
        UI_State->Root->First = 0;
        UI_State->Root->Last = 0;
        UI_State->Root->Next = 0;
        UI_State->Root->Prev = 0;
     */
    
    EndTemporaryMemory(UI_State->FrameMemory);
}

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
        UI_Init(&TranState->TranArena);
        
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
        UI_BeginFrame(&TranState->TranArena, Frame, Input);
        
        
        //ui_layout Layout = UI_MakeLayout(V2(100, 100), V2(200, 200));
        //UI_PushLayout(&Layout);
        if(UI_IsClicked(UI_Button("Foo")))
        {
            Log->Add("[ui] Button was clicked\n");
        }
        
        /* 
                if(UI_Button("Foo").Pressed)
                {
                    Log->Add("[ui] Button is pressed\n");
                }
         */
        //UI_PopLayout();
        
        // NOTE(ezexff): Draw
        //UI_DrawLayout(&Renderer->PushBufferUI, Layout);
        UI_EndFrame();
    }
    END_BLOCK();
    
    /* 
        bitmap_id BitmapID = GetFirstBitmapFrom(TranState->Assets, Asset_DuDvMap);
        v2 Dim = V2(1000, 1000);
        v2 Pos = V2((r32)Frame->Dim.x - Dim.x / 2, -Dim.y / 2);
        
        PushBitmapOnScreen(Frame, TranState->Assets, BitmapID, Pos, Dim, 1.0f);
        
        if(Frame->DrawDebugTextLine)
        {
            char *TestString = "The quick brown fox jumps over a lazy dog.";
            DEBUGTextLine(Frame, TranState->Assets, TestString);
        }
     */
    
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