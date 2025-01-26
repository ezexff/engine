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

internal void
DrawLabel(ui_node *Node)
{
    game_assets *Assets = UI_State->Assets;
    renderer_frame *Frame = UI_State->Frame;
    
    font_id FontID = GetFirstFontFrom(Assets, Asset_Font);
    FontID.Value++;
    loaded_font *Font = PushFont(Frame, Assets, FontID);
    if(Font)
    {
        eab_font *FontInfo = GetFontInfo(Assets, FontID);
        
        r32 FontScale = FontInfo->Scale;
        r32 LeftEdge = 0.0f;
        
        u32 PrevCodePoint = 0;
        r32 CharScale = FontScale;
        v4 Color = V4(1, 1, 1, 1);
        
        s32 LeftPadding = 5;
        s32 AtX = LeftPadding + (s32)Node->Rect.Min.x;
        r32 HalfRectY = (Node->Rect.Max.y - Node->Rect.Min.y) / 2;
        s32 AtY = (s32)Node->Rect.Min.y + (s32)(HalfRectY);
        
        r32 RectWidth = Node->Rect.Max.x - Node->Rect.Min.x;
        
        for(char *At = Node->String;
            *At;
            )
        {
            u32 CodePoint = *At;
            s32 Ascent = RoundR32ToS32(FontInfo->Ascent * FontScale);
            s32 LSB = Font->LSBs[CodePoint];
            s32 XOffset = s32(Font->GlyphOffsets[CodePoint].x);
            s32 YOffset = s32(Font->GlyphOffsets[CodePoint].y);
            
            if(CodePoint != ' ')
            {
                bitmap_id BitmapID = GetBitmapForGlyph(Assets, FontInfo, Font, CodePoint);
                eab_bitmap *GlyphInfo = GetBitmapInfo(Assets, BitmapID);
                
                v2 GlyphDim;
                GlyphDim.x = (r32)GlyphInfo->Dim[0];
                GlyphDim.y = (r32)GlyphInfo->Dim[1];
                v2 Pos = V2(0, 0);
                Pos.x = (r32)(AtX + XOffset);
                Pos.y = (r32)AtY + YOffset;
                //PushBitmapOnScreen(Frame, Assets, BitmapID, Pos, GlyphDim, 1.0f);
                renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
                v2 Min = Pos;
                v2 Max = Pos + GlyphDim;
                PushBitmapOnScreen(&Renderer->PushBufferUI, Assets, BitmapID, Min, Max, 100, 1.0f);
            }
            
            s32 AdvanceX = RoundR32ToS32(Font->Advances[CodePoint] * CharScale);
            AtX += AdvanceX;
            ++At;
            
            if(AtX > RectWidth)
            {
                InvalidCodePath;
            }
        }
    }
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
        
        if(NewNode->Parent->ChildLayoutAxis == Axis2_Y)
        {
            Min.y = NewNode->Prev->Rect.Max.y + PaddingY;
            Max.y = Min.y + UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex].FixedSize.y;
            
            if(Max.y > NewNode->Parent->Rect.Max.y)
            {
                InvalidCodePath;
            }
        }
        NewNode->Rect = {Min, Max};
    }
    
    UI_State->Root->Last = NewNode;
    
    return(NewNode);
}

u32 UI_GetNodeState(ui_node *Node)
{
    ui_style_template *Template = &UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex];
    ui_node_style *Style = &Node->Style;
    
    if(IsSet(Style, UI_NodeStyleFlag_DrawBackground))
    {
        Style->BackgroundColor = Template->BackgroundColor;
    }
    
    if(IsSet(Style, UI_NodeStyleFlag_Clickable))
    {
        v2 MouseP = V2((r32)UI_State->Input->MouseP.x, (r32)UI_State->Input->MouseP.y);
        //rectangle2 Rect = {V2(0, 0), Node->FixedSize};
        rectangle2 TestRect = Node->Rect;
        TestRect.Max.y = UI_State->Frame->Dim.y - Node->Rect.Min.y;
        TestRect.Min.y = UI_State->Frame->Dim.y - Node->Rect.Max.y;
        if(IsInRectangle(TestRect, MouseP))
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
    UI_UseStyleTemplate(UI_StyleTemplate_Button);
    ui_node *Node = UI_AddNode(UI_NodeStyleFlag_Clickable|
                               UI_NodeStyleFlag_DrawBorder|
                               UI_NodeStyleFlag_DrawText|
                               UI_NodeStyleFlag_DrawBackground|
                               UI_NodeStyleFlag_HotAnimation|
                               UI_NodeStyleFlag_ActiveAnimation,
                               String);
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
                /* 
                                StyleTemplate->PrefSize[Axis2_X].Type = UI_SizeKind_Pixels;
                                StyleTemplate->PrefSize[Axis2_X].Value = 50;
                                StyleTemplate->PrefSize[Axis2_Y].Type = UI_SizeKind_Pixels;
                                StyleTemplate->PrefSize[Axis2_Y].Value = 70;
                 */
            } break;
            
            InvalidDefaultCase;
        }
    }
}

internal void
UI_BeginFrame(tran_state *TranState, renderer_frame *Frame, game_input *Input)
{
    // TODO(ezexff): Mb rework (do without engine services)?
    UI_State->TranArena = &TranState->TranArena;
    UI_State->Assets = TranState->Assets;
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
    //UI_State->Root->Style.Flags = UI_NodeStyleFlag_DrawBackground;
    //UI_State->Root->Style.BackgroundColor = V4(0.5f, 0, 0.5f, 1);
    
    UI_State->Root->Rect = {V2(0, 0), V2((r32)UI_State->Frame->Dim.x, (r32)UI_State->Frame->Dim.y)};
    UI_State->Root->ChildLayoutAxis = Axis2_Y;
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
    renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
    if(IsSet(Style, UI_NodeStyleFlag_DrawBackground))
    {
        //rectangle2 Rect = {Node->FixedP, Node->FixedP + Node->FixedSize};
        PushRectOnScreen(&Renderer->PushBufferUI, Node->Rect.Min, Node->Rect.Max, Style->BackgroundColor, 100);
    }
    
    if(IsSet(Style, UI_NodeStyleFlag_DrawText))
    {
        DrawLabel(Node);
    }
    
    // NOTE(ezexff): Process childs
    if(Node->First)
    {
        //r32 AdvancedChildY = Node->FixedP.y + Node->InnerSumY;
        r32 AtX = 0;
        r32 AtY = 0;
        for(ui_node *ChildNode = Node->First;
            ChildNode != 0;
            ChildNode = ChildNode->Next)
        {
            UI_DrawNodeTree(ChildNode);
            /* 
                        if(Node->ChildLayoutAxis == Axis2_Y)
                        {
                            AtY -= ChildNode->FixedSize.y;
                        }
             */
            //Node->InnerSumY += ChildNode->FixedSize.y;
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
        UI_BeginFrame(TranState, Frame, Input);
        
        if(UI_IsClicked(UI_Button("Foo")))
        {
            Log->Add("[ui] Button Foo was clicked\n");
        }
        
        if(UI_IsClicked(UI_Button("Bar")))
        {
            Log->Add("[ui] Button Bar was clicked\n");
        }
        
        if(UI_IsClicked(UI_Button("Baz")))
        {
            Log->Add("[ui] Button Baz was clicked\n");
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