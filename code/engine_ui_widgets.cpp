internal u32
UI_Button(char *String)
{
    ui_node *Parent = UI_State->OpenWindowBody;
    if(Parent)
    {
        //UI_UseStyleTemplate();
        ui_node *Node = UI_AddNodeVer3(Parent,
                                       UI_NodeFlag_Clickable|
                                       UI_NodeFlag_DrawBorder|
                                       UI_NodeFlag_DrawText|
                                       UI_NodeFlag_DrawBackground|
                                       UI_NodeFlag_HotAnimation|
                                       UI_NodeFlag_ActiveAnimation,
                                       UI_StyleTemplate_Button,
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
        //UI_UseStyleTemplate(UI_StyleTemplate_Label);
        ui_node *Node = UI_AddNodeVer3(Parent,
                                       //UI_NodeFlag_DrawBackground|
                                       UI_NodeFlag_DrawBorder|
                                       UI_NodeFlag_DrawText,
                                       UI_StyleTemplate_Label,
                                       String);
    }
}

internal void
UI_Checkbox(char *String, b32 *Value)
{
    ui_node *Parent = UI_State->OpenWindowBody;
    if(Parent)
    {
        //UI_UseStyleTemplate(UI_StyleTemplate_Checkbox);
        ui_node *Node = UI_AddNodeVer3(Parent,
                                       UI_NodeFlag_Clickable|
                                       UI_NodeFlag_DrawBackground,
                                       UI_StyleTemplate_Checkbox,
                                       String);
        u32 State = UI_GetNodeState(Node);
        if(UI_IsPressed(State))
        {
            *Value = !*Value;
        }
        
        if(*Value)
        {
            //UI_UseStyleTemplate(UI_StyleTemplate_CheckboxMark);
            ui_node *NewNode = UI_AddNodeVer3(Node, UI_NodeFlag_DrawBackground, UI_StyleTemplate_CheckboxMark, 0);
            u32 State2 = UI_GetNodeState(NewNode);
        }
    }
}

internal void
UI_BeginWindow(char *String, b32 *Value)
{
    if(!UI_State->OpenWindow)
    {
        //UI_UseStyleTemplate(UI_StyleTemplate_Window);
        char *WindowString = Concat(UI_State->TranArena, "Window#", String);
        ui_node *Window = UI_AddNodeVer3(UI_State->Root,
                                         UI_NodeFlag_DrawBackground|
                                         UI_NodeFlag_Floating,
                                         UI_StyleTemplate_Window, WindowString);
        Window->LayoutAxis = Axis2_Y;
        UI_State->OpenWindow = Window;
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
        //UI_UseStyleTemplate(UI_StyleTemplate_WindowTitle);
        ui_node *Title = UI_AddNodeVer3(Window,
                                        UI_NodeFlag_Clickable|
                                        UI_NodeFlag_DrawBorder|
                                        UI_NodeFlag_DrawBackground,
                                        UI_StyleTemplate_WindowTitle,
                                        Concat(UI_State->TranArena, "Title#", String));
        u32 TitleState = UI_GetNodeState(Title);
        Title->LayoutAxis = Axis2_X;
        Title->Padding = 5.0f;
        Title->InteractionType = UI_Interaction_Move;
        if(UI_IsDragging(TitleState))
        {
            CachedWindow->OffsetP.x += UI_State->Input->dMouseP.x;
            CachedWindow->OffsetP.y -= UI_State->Input->dMouseP.y;
        }
        
        // NOTE(ezexff): Title content
        {
            // NOTE(ezexff): Expand button
            char *ExpandString = ">";
            if(WindowState & UI_NodeFlag_Expanded)
            {
                ExpandString = "v";
            }
            //UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleExitButton);
            ui_node *ExpandButton = UI_AddNodeVer3(Title,
                                                   UI_NodeFlag_Clickable|
                                                   UI_NodeFlag_DrawBorder|
                                                   UI_NodeFlag_DrawText|
                                                   UI_NodeFlag_DrawBackground|
                                                   UI_NodeFlag_HotAnimation|
                                                   UI_NodeFlag_ActiveAnimation,
                                                   UI_StyleTemplate_WindowTitleExitButton,
                                                   Concat(UI_State->TranArena, 
                                                          Concat(UI_State->TranArena, ExpandString, "#"), String));
            u32 ExpandButtonState = UI_GetNodeState(ExpandButton);
            if(UI_IsPressed(ExpandButtonState))
            {
                CachedWindow->Flags = CachedWindow->Flags ^ UI_NodeFlag_Expanded;
            }
            
            // NOTE(ezexff): Label
            //UI_UseStyleTemplate(UI_StyleTemplate_Label);
            ui_node *TitleLabel = UI_AddNodeVer3(Title,
                                                 //UI_NodeFlag_DrawBackground|
                                                 UI_NodeFlag_DrawBorder|
                                                 UI_NodeFlag_DrawText,
                                                 UI_StyleTemplate_Label,
                                                 Concat(UI_State->TranArena, 
                                                        Concat(UI_State->TranArena, String, "#"), String));
            // NOTE(ezexff): Empty space
            //UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleEmptySpace);
            ui_node *TitleEmptySpace = UI_AddNodeVer3(Title,
                                                      //UI_NodeFlag_Clickable|
                                                      UI_NodeFlag_DrawBorder,
                                                      UI_StyleTemplate_WindowTitleEmptySpace,
                                                      //UI_NodeFlag_DrawBackground,
                                                      Concat(UI_State->TranArena, "EmptySpace#", String));
            //TitleEmptySpace->StyleTemplateIndex = UI_StyleTemplate_WindowTitleEmptySpace;
            
            
            // NOTE(ezexff): Exit button
            //UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleExitButton);
            ui_node *ExitButton = UI_AddNodeVer3(Title,
                                                 UI_NodeFlag_Clickable|
                                                 UI_NodeFlag_DrawBorder|
                                                 UI_NodeFlag_DrawText|
                                                 UI_NodeFlag_DrawBackground|
                                                 UI_NodeFlag_HotAnimation|
                                                 UI_NodeFlag_ActiveAnimation,
                                                 UI_StyleTemplate_WindowTitleExitButton,
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
            
            /* 
                        UI_UseStyleTemplate(UI_StyleTemplate_WindowTitleEmptySpace);
                        u32 TitleEmptySpaceState = UI_GetNodeState(TitleEmptySpace);
                        if(UI_IsDragging(TitleEmptySpaceState))
                        {
                            CachedWindow->OffsetP.x += UI_State->Input->dMouseP.x;
                            CachedWindow->OffsetP.y -= UI_State->Input->dMouseP.y;
                        }
             */
        }
        
        if(WindowState & UI_NodeFlag_Expanded)
        {
            //UI_UseStyleTemplate(UI_StyleTemplate_WindowBody);
            ui_node *Body = UI_AddNodeVer3(Window,
                                           UI_NodeFlag_Clickable|
                                           UI_NodeFlag_DrawBorder|
                                           UI_NodeFlag_DrawBackground,
                                           UI_StyleTemplate_WindowBody,
                                           Concat(UI_State->TranArena, "WindowBody#", String));
            Body->InteractionType = UI_Interaction_Move;
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
            if(UI_IsDragging(BodyState))
            {
                CachedWindow->OffsetP.x += UI_State->Input->dMouseP.x;
                CachedWindow->OffsetP.y -= UI_State->Input->dMouseP.y;
            }
            
            //if(UI_State->OpenWindow && UI_State->OpenWindowBody)
            {
                ui_node *CachedWindow = UI_GetCachedNode(Window->String);
                if(!CachedWindow){InvalidCodePath;}
                
                ui_node *CachedBody = UI_GetCachedNode(Body->String);
                if(!CachedBody){InvalidCodePath;}
                
                // NOTE(ezexff): window resize button
                //UI_UseStyleTemplate(UI_StyleTemplate_WindowResizeButton);
                ui_node *ResizeButton = UI_AddNodeVer3(Window,
                                                       UI_NodeFlag_Floating|
                                                       UI_NodeFlag_Clickable|
                                                       UI_NodeFlag_DrawBorder|
                                                       UI_NodeFlag_DrawBackground,
                                                       UI_StyleTemplate_WindowResizeButton,
                                                       Concat(UI_State->TranArena, "ResizeButton#", Window->String));
                //v2 WindowRightBottomCorner = V2(Window->Rect.Max.x, Window->Rect.Max.y + Title->Rect.Max.y - Title->Rect.Min.y);
                v2 BodyRightBottomCorner = V2(Body->Rect.Max.x, Body->Rect.Max.y);
                ResizeButton->Rect.Min = V2(BodyRightBottomCorner.x - 10.0f, BodyRightBottomCorner.y - 10.0f);
                ResizeButton->Rect.Max = BodyRightBottomCorner;
                u32 ResizeButtonState = UI_GetNodeState(ResizeButton);
                ResizeButton->InteractionType = UI_Interaction_Move;
                if(UI_IsDragging(ResizeButtonState))
                {
                    v2 NewSize = {};
                    NewSize.x += UI_State->Input->dMouseP.x;
                    NewSize.y -= UI_State->Input->dMouseP.y;
                    
                    v2 NewWindowDim = GetDim(Window->Rect) + NewSize;
                    r32 MinWindowWidth = 100;
                    r32 MinBodyHeight = 100;
                    if(NewWindowDim.x > MinWindowWidth)
                    {
                        CachedWindow->OffsetSize.x += NewSize.x;
                    }
                    if(NewWindowDim.x > MinBodyHeight)
                    {
                        //CachedBody->OffsetSize.y += NewSize.y;
                        UI_State->StyleTemplateArray[UI_StyleTemplate_WindowBody].Size[Axis2_Y].Value += NewSize.y;
                        //CachedWindow->OffsetSize.y += NewSize.y;
                    }
                }
            }
            
            UI_State->OpenWindowBody = Body;
            
            // TODO(ezexff): test buttons
            /* 
                        {
                            ui_node *CachedBody = UI_GetCachedNode(Body->String);
                            if(!CachedBody){InvalidCodePath;}
                            
                            if(UI_IsPressed(UI_Button("HBar+")))
                            {
                                CachedBody->ViewP.x += 10;
                                Log->Add("HorBar+ action\n");
                            }
                            
                            if(UI_IsPressed(UI_Button("HBar-")))
                            {
                                CachedBody->ViewP.x -= 10;
                                Log->Add("HorBar- action\n");
                            }
                        }
             */
        }
    }
    else
    {
        InvalidCodePath;
    }
}

internal void
UI_EndWindow()
{
    if(!UI_State->OpenWindow){InvalidCodePath;}
    
    if(UI_State->OpenWindowBody)
    {
        ui_node *Window = UI_State->OpenWindow;
        ui_node *Body = UI_State->OpenWindowBody;
        ui_node *CachedBody1 = UI_GetCachedNode(Body->String);
        if(!CachedBody1){InvalidCodePath;}
        
        // NOTE(ezexff): horizontal scroll bar
        v2 BodyDim = Body->Rect.Max - Body->Rect.Min;
        if(Body->MaxChildNodeDim.x > BodyDim.x)
        {
            ui_node *HorizontalScrollBar = UI_AddNodeVer3(Body,
                                                          UI_NodeFlag_Floating|
                                                          UI_NodeFlag_Clickable|
                                                          UI_NodeFlag_DrawBorder|
                                                          UI_NodeFlag_DrawBackground,
                                                          UI_StyleTemplate_WindowScrollBar,
                                                          Concat(UI_State->TranArena, "HorizontalScrollBar#", Window->String));
            HorizontalScrollBar->Rect.Min = V2(Body->Rect.Min.x, Body->Rect.Max.y - 10.0f);
            HorizontalScrollBar->Rect.Max = V2(Body->Rect.Max.x - 10.0f, Body->Rect.Max.y);
            u32 HorizontalScrollBarState = UI_GetNodeState(HorizontalScrollBar);
            
            if(UI_IsDragging(HorizontalScrollBarState))
            {
                //if(CachedBody1->ViewP.x < Body->MaxChildNodeDim.x)
                r32 NewViewPX = CachedBody1->ViewP.x - UI_State->Input->dMouseP.x;
                // NewViewPX < Body->MaxChildNodeDim.x)
                r32 BodyDimX = Body->Rect.Max.x - Body->Rect.Min.x;
                //r32 TestMaxX = (-1)*(Body->MaxChildNodeDim.x + CachedBody1->ViewP.x);
                r32 TestMaxX = (-1)*(Body->MaxChildNodeDim.x - BodyDimX);
                // && (NewViewPX > TestMaxX)
                if(NewViewPX < 0)
                {
                    if(NewViewPX > TestMaxX)
                    {
                        CachedBody1->ViewP.x = NewViewPX;
                    }
                    //CachedBody1->ViewP.x += UI_State->Input->dMouseP.x;
                }
            }
        }
        
        // NOTE(ezexff): vertical scroll bar
        if(UI_State->OpenWindowBody->MaxChildNodeDim.y > UI_State->OpenWindowBody->Rect.Max.y)
        {
            ui_node *VerticalScrollBar = UI_AddNodeVer3(Body,
                                                        UI_NodeFlag_Floating|
                                                        UI_NodeFlag_Clickable|
                                                        UI_NodeFlag_DrawBorder|
                                                        UI_NodeFlag_DrawBackground,
                                                        UI_StyleTemplate_WindowScrollBar,
                                                        Concat(UI_State->TranArena, "VerticalScrollBar#", Window->String));
            VerticalScrollBar->Rect.Min = V2(Body->Rect.Max.x - 10.0f, Body->Rect.Min.y);
            VerticalScrollBar->Rect.Max = V2(Body->Rect.Max.x, Body->Rect.Max.y - 10.0f);
            u32 VerticalScrollBarState = UI_GetNodeState(VerticalScrollBar);
            
            if(UI_IsDragging(VerticalScrollBarState))
            {
                CachedBody1->ViewP.y -= UI_State->Input->dMouseP.y;
            }
        }
    }
    
    UI_State->OpenWindow = 0;
    UI_State->OpenWindowBody = 0;
}