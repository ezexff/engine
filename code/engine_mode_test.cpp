temporary_memory UI_TempMemory;
ui_rect *HotWidget;
memory_arena *UI_TranArena;
game_input *UI_Input;
ui_layout *LastLayout;
ui_rect *LastWidget;
ui_rect UI_RectSetinel;
renderer_push_buffer *UI_PushBuffer;


ui_rect *UI_WidgetMake(u32 Flags, char *String)
{
    ui_rect *Result = PushStruct(UI_TranArena, ui_rect);
    Result->Flags = Flags;
    Result->String = PushString(UI_TranArena, String);
    
    LastWidget = Result;
    return(Result);
}

ui_comm UI_CommFromWidget(ui_rect *Element)
{
    ui_comm Result = {};
    
    Element->Rect = {V2(0, 0), V2(100, 100)};
    
    if(IsSet(Element, UI_WidgetFlag_DrawBackground))
    {
        Element->BackgroundColor = V4(0, 0, 0, 1);
    }
    
    if(IsSet(Element, UI_WidgetFlag_Clickable))
    {
        v2 MouseP = V2((r32)UI_Input->MouseP.x, (r32)UI_Input->MouseP.y);
        if(IsInRectangle(Element->Rect, MouseP))
        {
            Element->BackgroundColor = V4(0, 0, 1, 1);
            if(WasPressed(UI_Input->MouseButtons[PlatformMouseButton_Left]))
            {
                Result.Clicked = true;
            }
            if(IsDown(UI_Input->MouseButtons[PlatformMouseButton_Left]))
            {
                Result.Pressed = true;
            }
        }
    }
    
    Result.Element = Element;
    
    PushRectOnScreen(UI_PushBuffer, Element->Rect.Min, Element->Rect.Max, Element->BackgroundColor, 100);
    
    return(Result);
}

ui_comm UI_Button(char *String)
{
    ui_rect *Widget = UI_WidgetMake(UI_WidgetFlag_Clickable|
                                    UI_WidgetFlag_DrawBorder|
                                    UI_WidgetFlag_DrawText|
                                    UI_WidgetFlag_DrawBackground|
                                    UI_WidgetFlag_HotAnimation|
                                    UI_WidgetFlag_ActiveAnimation,
                                    String);
    ui_comm Comm = UI_CommFromWidget(Widget);
    return(Comm);
}

/* 
void UI_DrawRect(ui_rect *Element)
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
    
    UI_TranArena = &TranState->TranArena;
    UI_Input = Input;
    UI_PushBuffer = &Renderer->PushBufferUI;
    UI_RectSetinel.Rect = {V2(0, 0), V2((r32)Frame->Dim.x, (r32)Frame->Dim.y)};
    UI_RectSetinel.BackgroundColor = V4(0, 1, 0, 1);
    
    //InvalidCodePath;
    // NOTE(ezexff): UI
    {
        UI_TempMemory = BeginTemporaryMemory(UI_TranArena);
        
        //ui_layout Layout = UI_MakeLayout(V2(100, 100), V2(200, 200));
        //UI_PushLayout(&Layout);
        if(UI_Button("Foo").Pressed)
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
        
        EndTemporaryMemory(UI_TempMemory);
    }
    
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