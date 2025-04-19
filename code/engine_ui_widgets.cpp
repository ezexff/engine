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
        Title->WidgetRoot = Window; // TODO(ezexff): test
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
        ui_node *CachedWindow = UI_GetCachedNode(Window->String);
        if(!CachedWindow){InvalidCodePath;}
        ui_node *CachedBody = UI_GetCachedNode(Body->String);
        if(!CachedBody){InvalidCodePath;}
        
        v2 ResizeButtonDim = V2(10.0f, 10.0f); // TODO(ezexff): only for test, need replace
        v2 WindowDim = GetDim(Window->Rect); // TODO(ezexff): use only body dim?
        v2 BodyDim = GetDim(Body->Rect);
        v2 ContentDim = Body->MaxChildNodeDim;
        ui_node *TestCachedHSBCursor = 0;
        
        // NOTE(ezexff): horizontal scroll bar
        if(ContentDim.x > WindowDim.x)
        {
            ui_node *HSBCursor = UI_AddNodeVer3(Body,
                                                UI_NodeFlag_Floating|
                                                UI_NodeFlag_Clickable|
                                                UI_NodeFlag_DrawBorder|
                                                UI_NodeFlag_DrawBackground,
                                                UI_StyleTemplate_WindowScrollBar,
                                                Concat(UI_State->TranArena, "HorizontalScrollBarCursor#", Window->String));
            u32 HSBCursorState = UI_GetNodeState(HSBCursor);
            
            r32 CursorWidth = WindowDim.x / (ContentDim.x + ResizeButtonDim.x) * WindowDim.x;
            if(CursorWidth < 10.0f)
            {
                InvalidCodePath;
            }
            
            
            ui_node *CachedHSBCursor = UI_GetCachedNode(HSBCursor->String);
            TestCachedHSBCursor = CachedHSBCursor;
            CachedHSBCursor->OffsetP.y = BodyDim.y - ResizeButtonDim.y;
            CachedHSBCursor->OffsetSize.y = 10.0f;
            
            if(UI_IsDragging(HSBCursorState))
            {
#if 1
                r32 MinCursorP = 0.0f;
                r32 MaxCursorP = WindowDim.x - CursorWidth - ResizeButtonDim.x;
                
                r32 MinViewP = (-1) * (ContentDim.x - WindowDim.x);
                r32 MaxViewP = 0.0f;
                r32 ScrollMultiplier = (-1) * (MinViewP / MaxCursorP);
                
                r32 MinDragHSBCursorP = Window->Rect.Min.x + CachedHSBCursor->PressMouseP.x;
                r32 MaxDragHSBCursorP = Window->Rect.Max.x - CursorWidth + CachedHSBCursor->PressMouseP.x - ResizeButtonDim.x;
                if(UI_State->Input->MouseP.x < MinDragHSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    CachedHSBCursor->OffsetP.x = MinCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    CachedBody->ViewP.x = MaxViewP;
                }
                else if(UI_State->Input->MouseP.x > MaxDragHSBCursorP)
                {
                    // NOTE(ezexff): move scrollbar cursor
                    CachedHSBCursor->OffsetP.x = MaxCursorP;
                    
                    // NOTE(ezexff): move body view pos
                    CachedBody->ViewP.x = MinViewP;
                }
                else
                {
                    // NOTE(ezexff): move scrollbar cursor
                    CachedHSBCursor->OffsetP.x = (r32)RoundR32ToS32(UI_State->Input->MouseP.x - Window->Rect.Min.x - CachedHSBCursor->PressMouseP.x);
                    CachedHSBCursor->OffsetP.x = Clamp(MinCursorP, CachedHSBCursor->OffsetP.x, MaxCursorP);
                    
                    // NOTE(ezexff): move body view pos
                    //CachedBody->ViewP.x = CachedBody->ViewP.x - ScrollMultiplier * UI_State->Input->dMouseP.x;
                    CachedBody->ViewP.x -= (r32)RoundR32ToS32(ScrollMultiplier * UI_State->Input->dMouseP.x);
                    CachedBody->ViewP.x = Clamp(MinViewP, CachedBody->ViewP.x, MaxViewP);
                }
#else
                r32 MinDragHSBCursorP = Window->Rect.Min.x + CachedHSBCursor->PressMouseP.x;
                r32 MaxDragHSBCursorP = Window->Rect.Max.x - CursorWidth + CachedHSBCursor->PressMouseP.x;
                if((UI_State->Input->MouseP.x > MinDragHSBCursorP) &&
                   (UI_State->Input->MouseP.x < MaxDragHSBCursorP))
                {
                    // NOTE(ezexff): move scrollbar cursor
                    r32 MinCursorP = 0.0f;
                    r32 MaxCursorP = WindowDim.x - CursorWidth - ResizeButtonDim.x;
                    r32 NewCursorP = CachedHSBCursor->OffsetP.x + UI_State->Input->dMouseP.x;
                    NewCursorP = Clamp(MinCursorP, NewCursorP, MaxCursorP);
                    
                    //r32 TestMousePX = UI_State->Input->MouseP.x - HSBCursor->Rect.Min.x - UI_State->Input->dMouseP.x;
                    {
                        /* 
                                                if(CachedHSBCursor->OffsetP.x == MinCursorP)
                                                {
                                                    // || !(CachedHSBCursor->OffsetP.x == MaxCursorP)
                                                    Log->Add("TestMousePX = %f\n", TestMousePX);
                                                    Log->Add("PressMouseP.x = %f\n", CachedHSBCursor->PressMouseP.x);
                                                    Log->Add("OffsetP.x = %f\n", CachedHSBCursor->OffsetP.x);
                                                }
                         */
                        
                        CachedHSBCursor->OffsetP.x = NewCursorP;
                        
                        // NOTE(ezexff): move view pos
                        r32 MinViewP = (-1) * (ContentDim.x - WindowDim.x);
                        r32 MaxViewP = 0.0f;
                        r32 ScrollMultiplier = (-1) * (MinViewP / MaxCursorP);
                        //r32 ScrollMultiplier = WindowDim.x / MaxNewCursorP;
                        r32 NewViewP = CachedBody->ViewP.x - ScrollMultiplier * UI_State->Input->dMouseP.x;
                        CachedBody->ViewP.x = Clamp(MinViewP, NewViewP, MaxViewP);
                    }
                }
#endif
            }
            
            CachedHSBCursor->OffsetSize.x = CursorWidth;
            
            /* 
                        r32 MaxCursorP = WindowDim.x - CursorWidth - ResizeButtonDim.x;
                        if(CachedHSBCursor->OffsetP.x > MaxCursorP)
                        {
                            r32 Diff = CachedHSBCursor->OffsetP.x - MaxCursorP;
                            CachedHSBCursor->OffsetP.x -= Diff;
                            
                        }
             */
            
            /* 
                        r32 Test = CachedHSBCursor->OffsetP.x + CursorWidth;
                        if(Test > Window->Rect.Max.x)
                        {
                            r32 Diff = Test - Window->Rect.Max.x;
                            CachedHSBCursor->OffsetP.x -= Diff;
                        }
             */
            //if(CachedHSBCursor->OffsetP.x)
            /* 
                        r32 Test = ContentDim.x + CachedBody->ViewP.x - WindowDim.x;
                        if(Test < 0)
                        {
                            CachedHSBCursor->OffsetP.x += Test;
                            
                            CachedBody->ViewP.x -= Test;
                        }
             */
            
            /* 
                        r32 TestMax = Window->Rect.Max.x - 10.0f;
                        if(HorizontalScrollBarCursor->Rect.Max.x > TestMax)
                        {
                            r32 Diff = HorizontalScrollBarCursor->Rect.Max.x - TestMax;
                            CachedHorizontalScrollBarCursor->OffsetP.x -= Diff;
                            
                            CachedBody->ViewP.x += Multiplier2 * Diff;
                        }
             */
        }
        
        // NOTE(ezexff): vertical scroll bar
        /* 
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
         */
        
        // NOTE(ezexff): window resize button
        {
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
                    
                    // TODO(ezexff): fix body view pos
                    if(TestCachedHSBCursor)
                    {
                        /* 
                                                r32 CursorWidth = WindowDim.x / (ContentDim.x + ResizeButtonDim.x) * WindowDim.x;
                                                if(CursorWidth < 10.0f) {InvalidCodePath;}
                                                TestCachedHSBCursor->OffsetSize.x = CursorWidth;
                         */
                        
                        r32 NewCursorWidth = NewWindowDim.x / (ContentDim.x  + ResizeButtonDim.x) * NewWindowDim.x;
                        if(NewCursorWidth < 10.0f) {InvalidCodePath;}
                        
                        if(NewWindowDim.x != WindowDim.x)
                        {
                            r32 CursorWidth = WindowDim.x / (ContentDim.x + ResizeButtonDim.x) * WindowDim.x;
                            if(CursorWidth < 10.0f) {InvalidCodePath;}
                            r32 MaxCursorP = WindowDim.x - CursorWidth - ResizeButtonDim.x;
                            r32 Percent = TestCachedHSBCursor->OffsetP.x / MaxCursorP;
                            
                            r32 MinCursorP = 0.0f;
                            r32 NewMaxCursorP = NewWindowDim.x - NewCursorWidth - ResizeButtonDim.x;
                            TestCachedHSBCursor->OffsetP.x = (r32)RoundR32ToS32(Percent * NewMaxCursorP);
                            TestCachedHSBCursor->OffsetP.x = Clamp(MinCursorP, TestCachedHSBCursor->OffsetP.x, NewMaxCursorP);
                            
                            r32 VisibleContent = ContentDim.x + CachedBody->ViewP.x;
                            if((NewWindowDim.x > WindowDim.x) && (NewWindowDim.x >= VisibleContent))
                            {
                                r32 NewMinViewP = (-1) * (ContentDim.x - NewWindowDim.x);
                                r32 ScrollMultiplier = (-1) * (NewMinViewP / NewMaxCursorP);
                                CachedBody->ViewP.x += (r32)RoundR32ToS32(ScrollMultiplier * UI_State->Input->dMouseP.x);
                                
                                CachedBody->ViewP.x = Clamp(NewMinViewP, CachedBody->ViewP.x, 0);
                            }
                        }
                        
                        TestCachedHSBCursor->OffsetSize.x = NewCursorWidth;
                        
                    }
                    
                    /* 
                                        r32 MinViewP = (-1) * (ContentDim.x - WindowDim.x);
                                        r32 ViewP = CachedBody->ViewP.x;
                     */
                    /* 
                                        if(TestCachedHSBCursor)
                                        {
                                            r32 NewCursorWidth = NewWindowDim.x / ContentDim.x * NewWindowDim.x;
                                            if(NewWindowDim.x < 10.0f)
                                            {
                                                InvalidCodePath;
                                            }
                                            
                                            r32 MaxNewCursorP = NewWindowDim.x - NewCursorWidth - ResizeButtonDim.x;
                                            
                                            r32 MinNewViewP = (-1) * (ContentDim.x - NewWindowDim.x);
                                            r32 ScrollMultiplier = (-1) * (MinNewViewP / MaxNewCursorP);
                                            //r32 ScrollMultiplier = ContentDim.x / NewWindowDim.x;
                                            
                                            
                                            
                                            r32 NewCursorP = TestCachedHSBCursor->OffsetP.x + UI_State->Input->dMouseP.x / ScrollMultiplier;
                                            TestCachedHSBCursor->OffsetP.x = Clamp(0, NewCursorP, MaxNewCursorP);
                                            
                                            
                                            if(NewSize.x > 0)
                                            {
                                                if(TestCachedHSBCursor->OffsetP.x == MaxNewCursorP)
                                                {
                                                    r32 NewViewP = CachedBody->ViewP.x - ScrollMultiplier * UI_State->Input->dMouseP.x;
                                                    CachedBody->ViewP.x = Clamp(MinNewViewP, NewViewP, 0);
                                                }
                                            }
                                            
                                            TestCachedHSBCursor->OffsetSize.x = NewCursorWidth;
                                        }
                     */
                }
                if(NewWindowDim.x > MinBodyHeight)
                {
                    //CachedBody->OffsetSize.y += NewSize.y;
                    UI_State->StyleTemplateArray[UI_StyleTemplate_WindowBody].Size[Axis2_Y].Value += NewSize.y;
                    //CachedWindow->OffsetSize.y += NewSize.y;
                }
            }
        }
    }
    
    
    
    UI_State->OpenWindow = 0;
    UI_State->OpenWindowBody = 0;
}