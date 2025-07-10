internal void
UI_Label(char *ID, char *Text)
{
    if(!UI_State->WindowArray.Last){InvalidCodePath};
    ui_node *Window = UI_State->WindowArray.Last;
    if(Window)
    {
        //InvalidCodePath;
        if(Window->Body)
        {
            if(Window->Cache->Flags & UI_NodeFlag_Expanded)
            {
                ui_node *Node = UI_AddNode(Window,
                                           Window->Body,
                                           ID,
                                           UI_StyleTemplate_Label,
                                           //UI_NodeFlag_DrawBackground|
                                           UI_NodeFlag_DrawBorder|UI_NodeFlag_DrawText,
                                           Text);
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
}

internal void
UI_Label(char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    char Temp[1024];
    FormatStringList(sizeof(Temp), Temp, Format, ArgList);
    UI_Label(Format, Temp);
    va_end(ArgList);
}

internal u32
UI_Button(char *ID, char *Text)
{
    u32 State = 0;
    if(!UI_State->WindowArray.Last){InvalidCodePath};
    ui_node *Window = UI_State->WindowArray.Last;
    if(Window)
    {
        //InvalidCodePath;
        if(Window->Body)
        {
            if(Window->Cache->Flags & UI_NodeFlag_Expanded)
            {
                ui_node *Node = UI_AddNode(Window,
                                           Window->Body,
                                           ID,
                                           UI_StyleTemplate_Button,
                                           UI_NodeFlag_Clickable|
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawText|
                                           UI_NodeFlag_DrawBackground|
                                           UI_NodeFlag_HotAnimation|
                                           UI_NodeFlag_ActiveAnimation,
                                           Text);
                State = UI_GetNodeState(Node);
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
    return(State);
}

internal u32
UI_Button(char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    char Temp[1024];
    FormatStringList(sizeof(Temp), Temp, Format, ArgList);
    u32 State = UI_Button(Format, Temp);
    va_end(ArgList);
    return(State);
}

void
UI_CheckBox(char *ID, b32 *Value)
{
    if(!UI_State->WindowArray.Last){InvalidCodePath};
    ui_node *Window = UI_State->WindowArray.Last;
    if(Window)
    {
        //InvalidCodePath;
        if(Window->Body)
        {
            if(Window->Cache->Flags & UI_NodeFlag_Expanded)
            {
                ui_node *Node = UI_AddNode(Window,
                                           Window->Body,
                                           ID,
                                           UI_StyleTemplate_Checkbox,
                                           //UI_NodeFlag_DrawBackground|
                                           UI_NodeFlag_Clickable|
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawBackground);
                Node->LayoutAxis = Axis2_X;
                u32 State = UI_GetNodeState(Node);
                if(UI_IsPressed(State))
                {
                    *Value = !*Value;
                }
                
                // TODO(ezexff): Temp
                if(*Value)
                {
                    Node->Cache->BackgroundColor = V4(.125f, .196f, .298f, 1);
                }
                else
                {
                    Node->Cache->BackgroundColor = RGBA(66, 150, 250, 1);
                }
                /* 
                                if(*Value)
                                {
                                    ui_node *MarkNode = UI_AddNode(Window,
                                                                   Node,
                                                                   Concat(UI_State->TranArena, ID, "Mark"),
                                                                   UI_StyleTemplate_CheckboxMark,
                                                                   UI_NodeFlag_Clickable|
                                                                   UI_NodeFlag_DrawBorder|
                                                                   UI_NodeFlag_DrawBackground);
                                }
                 */
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
}

/* 
internal void
UI_CheckBox(b32 *Value, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    
    char Temp[1024];
    FormatStringList(sizeof(Temp), Temp, Format, ArgList);
    UI_CheckBox(Format, Temp, Value);
    va_end(ArgList);
}
 */

internal void
UI_BeginWindow(char *ID, b32 *ExitButtonValue)
{
    ui_node *Window = UI_AddRootNode(&UI_State->WindowArray,
                                     ID,
                                     UI_StyleTemplate_Window1,
                                     //UI_NodeFlag_DrawBackground|UI_NodeFlag_Floating|UI_NodeFlag_Clickable|UI_NodeFlag_DrawBorder);
                                     UI_NodeFlag_DrawBackground|UI_NodeFlag_Floating);
    UI_State->WindowCount++;
    u32 WindowState = UI_GetNodeState(Window);
    
    // NOTE(ezexff): Title
    ui_node *Title = UI_AddNode(Window,
                                Window,
                                Concat(UI_State->TranArena, "Title#", ID),
                                UI_StyleTemplate_WindowTitle,
                                UI_NodeFlag_Clickable|UI_NodeFlag_DrawBorder|UI_NodeFlag_DrawBackground);
    Title->LayoutAxis = Axis2_X;
    u32 TitleState = UI_GetNodeState(Title);
    
    // NOTE(ezexff): Title content
    {
        ui_node *ExpandButton = UI_AddNode(Window,
                                           Title,
                                           Concat(UI_State->TranArena, "test#", ID),
                                           UI_StyleTemplate_WindowTitleExitButton,
                                           UI_NodeFlag_Clickable|
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawText|
                                           UI_NodeFlag_DrawBackground|
                                           UI_NodeFlag_HotAnimation|
                                           UI_NodeFlag_ActiveAnimation,
                                           (Window->Cache->Flags & UI_NodeFlag_Expanded) ? "v" : ">");
        u32 ExpandButtonState = UI_GetNodeState(ExpandButton);
        if(UI_IsPressed(ExpandButtonState))
        {
            Window->Cache->Flags = Window->Cache->Flags ^ UI_NodeFlag_Expanded;
        }
        
        ui_node *TitleLabel = UI_AddNode(Window,
                                         Title,
                                         Concat(UI_State->TranArena, ID, "#", ID),
                                         UI_StyleTemplate_Label,
                                         //UI_NodeFlag_DrawBackground|
                                         UI_NodeFlag_DrawBorder|
                                         UI_NodeFlag_DrawText,
                                         ID);
        UI_State->WindowArray.Last->Title = Title;
        
        // NOTE(ezexff): Empty space
        ui_node *TitleEmptySpace = UI_AddNode(Window,
                                              Title,
                                              Concat(UI_State->TranArena, "EmptySpace#", ID),
                                              UI_StyleTemplate_WindowTitleEmptySpace,
                                              UI_NodeFlag_DrawBorder);
        
        // NOTE(ezexff): Exit button
        ui_node *ExitButton = UI_AddNode(Window,
                                         Title,
                                         Concat(UI_State->TranArena, "x#", ID),
                                         UI_StyleTemplate_WindowTitleExitButton,
                                         UI_NodeFlag_Clickable|
                                         UI_NodeFlag_DrawBorder|
                                         UI_NodeFlag_DrawText|
                                         UI_NodeFlag_DrawBackground|
                                         UI_NodeFlag_HotAnimation|
                                         UI_NodeFlag_ActiveAnimation,
                                         "x#");
        
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
            //*ExitButtonValue = !*ExitButtonValue;
        }
    }
    
    if(Window->Cache->Flags & UI_NodeFlag_Expanded)
    {
        ui_node *Body = UI_AddNode(Window,
                                   Window,
                                   Concat(UI_State->TranArena, "WindowBody#", ID),
                                   UI_StyleTemplate_WindowBody,
                                   UI_NodeFlag_Clickable|
                                   UI_NodeFlag_DrawBorder|
                                   UI_NodeFlag_DrawBackground);
        Body->LayoutAxis = Axis2_Y;
        UI_State->WindowArray.Last->Body = Body;
        
        u32 BodyState = UI_GetNodeState(Body);
        if(UI_IsDragging(BodyState))
        {
            Window->Cache->P.x += UI_State->Input->dMouseP.x;
            Window->Cache->P.y -= UI_State->Input->dMouseP.y;
        }
    }
    
    if(UI_IsDragging(WindowState) || UI_IsDragging(TitleState))
    {
        Window->Cache->P.x += UI_State->Input->dMouseP.x;
        Window->Cache->P.y -= UI_State->Input->dMouseP.y;
    }
}

internal void
UI_EndWindow()
{
    ui_node *Window = UI_State->WindowArray.Last;
    ui_node *Body = Window->Body;
    
    /* 
    UI_State->WindowArray.Last->VerticalScrollbar = ...;
    UI_State->WindowArray.Last->HorizontalScrollbar = ...;
    UI_State->WindowArray.Last->ResizeButton = ...;
 */
    if(Body)
    {
        v2 ResizeButtonDim = V2(10.0f, 10.0f); // TODO(ezexff): replace with getting dim from node
        v2 BodyDim = GetDim(Body->Rect);
        v2 ContentDim = Body->ContentDim;
        
        // NOTE(ezexff): horizontal scroll bar
        if(ContentDim.x > BodyDim.x)
        {
            ui_node *HScrollBarCursor = UI_AddNode(Window,
                                                   Body,
                                                   Concat(UI_State->TranArena, "HScrollBarCursor#", Window->ID),
                                                   UI_StyleTemplate_WindowScrollBar,
                                                   UI_NodeFlag_Floating|
                                                   UI_NodeFlag_Clickable|
                                                   UI_NodeFlag_DrawBorder|
                                                   UI_NodeFlag_DrawBackground);
            u32 HScrollBarCursorState = UI_GetNodeState(HScrollBarCursor);
            
            r32 CursorWidth = BodyDim.x / (ContentDim.x + ResizeButtonDim.x) * BodyDim.x;
            if(CursorWidth < 10.0f)
            {
                InvalidCodePath;
            }
            
            HScrollBarCursor->Cache->P.y = BodyDim.y - ResizeButtonDim.y;
            HScrollBarCursor->Cache->Size.y = 10.0f;
            
            if(UI_IsDragging(HScrollBarCursorState))
            {
                r32 MinCursorP = 0.0f;
                r32 MaxCursorP = BodyDim.x - CursorWidth - ResizeButtonDim.x;
                
                r32 MinViewP = (-1) * (ContentDim.x - BodyDim.x);
                r32 MaxViewP = 0.0f;
                r32 ScrollMultiplier = (-1) * (MinViewP / MaxCursorP);
                //Log->Add("HScrollBar = %.2f %.2f\n", MinViewP, MaxViewP);
                
                r32 MinDragHSBCursorP = Body->Rect.Min.x + HScrollBarCursor->Cache->PressMouseP.x;
                r32 MaxDragHSBCursorP = Body->Rect.Max.x - CursorWidth + HScrollBarCursor->Cache->PressMouseP.x - ResizeButtonDim.x;
                if(UI_State->Input->MouseP.x < MinDragHSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    HScrollBarCursor->Cache->P.x = MinCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.x = MaxViewP;
                }
                else if(UI_State->Input->MouseP.x > MaxDragHSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    HScrollBarCursor->Cache->P.x = MaxCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.x = MinViewP;
                }
                else
                {
                    // NOTE(ezexff): move scrollbar cursor
                    HScrollBarCursor->Cache->P.x = (r32)RoundR32ToS32(UI_State->Input->MouseP.x - Body->Rect.Min.x - HScrollBarCursor->Cache->PressMouseP.x);
                    HScrollBarCursor->Cache->P.x = Clamp(MinCursorP, HScrollBarCursor->Cache->P.x, MaxCursorP);
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.x -= (r32)RoundR32ToS32(ScrollMultiplier * UI_State->Input->dMouseP.x);
                    Body->Cache->ViewP.x = Clamp(MinViewP, Body->Cache->ViewP.x, MaxViewP);
                }
            }
            
            HScrollBarCursor->Cache->Size.x = CursorWidth;
        }
        
        // NOTE(ezexff): vertical scroll bar
        if(ContentDim.y > BodyDim.y)
        {
            ui_node *VScrollBarCursor = UI_AddNode(Window,
                                                   Body,
                                                   Concat(UI_State->TranArena, "VScrollBarCursor#", Window->ID),
                                                   UI_StyleTemplate_WindowScrollBar,
                                                   UI_NodeFlag_Floating|
                                                   UI_NodeFlag_Clickable|
                                                   UI_NodeFlag_DrawBorder|
                                                   UI_NodeFlag_DrawBackground);
            u32 VScrollBarCursorState = UI_GetNodeState(VScrollBarCursor);
            
            r32 CursorHeight = BodyDim.y / (ContentDim.y + ResizeButtonDim.y) * BodyDim.y;
            if(CursorHeight < 10.0f)
            {
                InvalidCodePath;
            }
            
            VScrollBarCursor->Cache->P.x = BodyDim.x - ResizeButtonDim.x;
            VScrollBarCursor->Cache->Size.x = 10.0f;
            
            if(UI_IsDragging(VScrollBarCursorState))
            {
                r32 MinCursorP = 0.0f;
                r32 MaxCursorP = BodyDim.y - CursorHeight - ResizeButtonDim.y;
                
                r32 MinViewP = (-1) * (ContentDim.y - BodyDim.y);
                r32 MaxViewP = 0.0f;
                r32 ScrollMultiplier = (-1) * (MinViewP / MaxCursorP);
                //Log->Add("VScrollBar = %.2f %.2f\n", MinViewP, MaxViewP);
                
                r32 MinDragVSBCursorP = Body->Rect.Min.y + VScrollBarCursor->Cache->PressMouseP.y;
                r32 MaxDragVSBCursorP = Body->Rect.Max.y - CursorHeight + VScrollBarCursor->Cache->PressMouseP.y - ResizeButtonDim.y;
                r32 MousePY = UI_State->Frame->Dim.y - UI_State->Input->MouseP.y;
                r32 MouseDeltaY = (-1) * UI_State->Input->dMouseP.y;
                if(MousePY < MinDragVSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    VScrollBarCursor->Cache->P.y = MinCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.y = MaxViewP;
                }
                else if(MousePY > MaxDragVSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    VScrollBarCursor->Cache->P.y = MaxCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.y = MinViewP;
                }
                else
                {
                    // NOTE(ezexff): move scrollbar cursor
                    VScrollBarCursor->Cache->P.y = (r32)RoundR32ToS32(MousePY - Body->Rect.Min.y - VScrollBarCursor->Cache->PressMouseP.y);
                    VScrollBarCursor->Cache->P.y = Clamp(MinCursorP, VScrollBarCursor->Cache->P.y, MaxCursorP);
                    
                    // NOTE(ezexff): move body view pos
                    Body->Cache->ViewP.y -= (r32)RoundR32ToS32(ScrollMultiplier * MouseDeltaY);
                    Body->Cache->ViewP.y = Clamp(MinViewP, Body->Cache->ViewP.y, MaxViewP);
                }
            }
            else
            {
                u32 BodyState = UI_GetNodeState(Body);
                if(UI_IsHovering(BodyState))
                {
                    u32 ScrollSensitivity = 20;
                    if(UI_State->Input->dMouseP.z != 0)
                    {
                        r32 MinCursorP = 0.0f;
                        r32 MaxCursorP = BodyDim.y - CursorHeight - ResizeButtonDim.y;
                        
                        r32 MinViewP = (-1) * (ContentDim.y - BodyDim.y);
                        r32 MaxViewP = 0.0f;
                        r32 ScrollMultiplier = (-1) * (MinViewP / MaxCursorP);
                        
                        r32 MouseDeltaZ = UI_State->Input->dMouseP.z;
                        
                        VScrollBarCursor->Cache->P.y -= (r32)RoundR32ToS32(MouseDeltaZ * ScrollSensitivity);
                        VScrollBarCursor->Cache->P.y = Clamp(MinCursorP, VScrollBarCursor->Cache->P.y, MaxCursorP);
                        
                        Body->Cache->ViewP.y += (r32)RoundR32ToS32(ScrollMultiplier * MouseDeltaZ * ScrollSensitivity);
                        Body->Cache->ViewP.y = Clamp(MinViewP, Body->Cache->ViewP.y, MaxViewP);
                    }
                }
            }
            
            VScrollBarCursor->Cache->Size.y = CursorHeight;
            
            /* 
            if(!UI_IsDragging(VScrollBarCursorState))
            {
                if(UI_State->Input->dMouseP.z != 0)
                {
                    // TODO(ezexff): 
                    Log->Add("implement scrolling in window by mouse scroll\n");
                }
            }
 */
        }
        
        ui_node *ResizeButton = UI_AddNode(Window,
                                           Body,
                                           Concat(UI_State->TranArena, "ResizeButton#", Window->ID),
                                           UI_StyleTemplate_WindowResizeButton,
                                           UI_NodeFlag_Floating|
                                           UI_NodeFlag_Clickable|
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawBackground);
        v2 BodyRightBottomCorner = V2(Body->Rect.Max.x, Body->Rect.Max.y);
        ResizeButton->Rect.Min = V2(BodyRightBottomCorner.x - 10.0f, BodyRightBottomCorner.y - 10.0f);
        ResizeButton->Rect.Max = BodyRightBottomCorner;
        u32 ResizeButtonState = UI_GetNodeState(ResizeButton);
        
        // TODO(ezexff):  NEED REWORK WITH SCROLL BAR UPDATE!!!
        if(UI_IsDragging(ResizeButtonState))
        {
            v2 NewSize = {};
            NewSize.x += UI_State->Input->dMouseP.x;
            NewSize.y -= UI_State->Input->dMouseP.y;
            
            r32 MinWindowWidth = 200;
            r32 MinBodyHeight = 100;
            v2 NewDim = GetDim(Window->Rect) + NewSize;
            if(NewDim.x > MinWindowWidth)
            {
                Window->Cache->Size.x += NewSize.x;
            }
            if(NewDim.x > MinBodyHeight)
            {
                Body->Cache->Size.y += NewSize.y;
            }
        }
    }
}