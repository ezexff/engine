internal u32
UI_Button(char *String)
{
    ui_node *Parent = UI_State->OpenWindowBody;
    if(Parent)
    {
        UI_UseStyleTemplate(UI_StyleTemplate_Button);
        ui_node *Node = UI_AddNodeVer2(Parent,
                                       UI_NodeFlag_Clickable|
                                       UI_NodeFlag_DrawBorder|
                                       UI_NodeFlag_DrawText|
                                       UI_NodeFlag_DrawBackground|
                                       UI_NodeFlag_HotAnimation|
                                       UI_NodeFlag_ActiveAnimation,
                                       String);
        u32 State = UI_GetNodeState(Node);
        return(State);
    }
    return(0);
}

internal void
UI_Label(char *String)
{
    ui_node *Parent = UI_State->OpenWindowBody;
    if(Parent)
    {
        UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *Node = UI_AddNodeVer2(Parent,
                                       //UI_NodeFlag_DrawBackground|
                                       UI_NodeFlag_DrawBorder|
                                       UI_NodeFlag_DrawText,
                                       String);
    }
}

internal void
UI_Checkbox(char *String, b32 *Value)
{
    ui_node *Parent = UI_State->OpenWindowBody;
    if(Parent)
    {
        UI_UseStyleTemplate(UI_StyleTemplate_Checkbox);
        ui_node *Node = UI_AddNodeVer2(Parent,
                                       UI_NodeFlag_Clickable|
                                       UI_NodeFlag_DrawBackground,
                                       String);
        u32 State = UI_GetNodeState(Node);
        if(UI_IsPressed(State))
        {
            *Value = !*Value;
        }
        
        if(*Value)
        {
            UI_UseStyleTemplate(UI_StyleTemplate_CheckboxMark);
            ui_node *NewNode = UI_AddNodeVer2(Node, UI_NodeFlag_DrawBackground, 0);
            u32 State2 = UI_GetNodeState(NewNode);
        }
    }
}

internal void
UI_BeginWindow(char *String, b32 *Value)
{
    if(!UI_State->OpenWindow)
    {
        UI_UseStyleTemplate(UI_StyleTemplate_Window);
        char *WindowString = Concat(UI_State->TranArena, "Window#", String);
        ui_node *Window = UI_AddNodeVer2(UI_State->Root, UI_NodeFlag_Floating, WindowString);
        Window->LayoutAxis = Axis2_Y;
        /* 
                Window->Rect.Min = Window->Rect.Min + P;
                Window->Rect.Max = Window->Rect.Max + P;
                Window->Rect.Max.x = Window->Rect.Max.x + Size.x;
                r32 WindowWidth = Window->Rect.Max.x - Window->Rect.Min.x;
                if(WindowWidth < MinWindowWidth)
                {
                    Window->Rect.Max.x = Window->Rect.Min.x + MinWindowWidth;
                }
         */
        u32 WindowState = UI_GetNodeState(Window);
        
        ui_node *CachedWindow = UI_GetCachedNode(WindowString);
        if(!CachedWindow){InvalidCodePath;}
        
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
            if(WindowState & UI_NodeFlag_Expanded)
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
                CachedWindow->Flags = CachedWindow->Flags ^ UI_NodeFlag_Expanded;
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
            r32 TitleEmptySpaceWidth = Title->Rect.Max.x - TitleLabel->Rect.Max.x - ExitButtonWidth - Title->Padding;
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
                CachedWindow->OffsetP.x += UI_State->Input->dMouseP.x;
                CachedWindow->OffsetP.y -= UI_State->Input->dMouseP.y;
            }
        }
        
        if(WindowState & UI_NodeFlag_Expanded)
        {
            UI_UseStyleTemplate(UI_StyleTemplate_WindowBody);
            ui_node *Body = UI_AddNodeVer2(Window,
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawBackground,
                                           Concat(UI_State->TranArena, "WindowBody#", String));
            /* 
                        Body->Rect.Max.y = Body->Rect.Max.y + Size.y;
                        r32 BodyHeight = Body->Rect.Max.y - Body->Rect.Min.y;
                        if(BodyHeight < MinBodyHeight)
                        {
                            Body->Rect.Max.y = Body->Rect.Min.y + MinBodyHeight;
                        }
             */
            Body->Spacing = 5.0f;
            Body->Padding = 5.0f;
            Body->LayoutAxis = Axis2_Y;
            u32 BodyState = UI_GetNodeState(Body);
            UI_State->OpenWindowBody = Body;
        }
        
        UI_State->OpenWindow = Window;
    }
    else
    {
        InvalidCodePath;
    }
}

internal void
UI_EndWindow()
{
    if(UI_State->OpenWindow)
    {
        /* 
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
                    NewSize.x += UI_State->Input->MouseDelta.x;
                    NewSize.y -= UI_State->Input->MouseDelta.y;
                    
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
         */
        
        UI_State->OpenWindow = 0;
        UI_State->OpenWindowBody = 0;
    }
    else
    {
        InvalidCodePath;
    }
}