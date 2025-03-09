inline u32
UI_GetHashValue(char *String)
{
    u32 Result = 0;
    char *Scan = String;
    for(;
        *Scan;
        ++Scan)
    {
        Result = 65599 * Result + *Scan;
    }
    return(Result);
}

inline ui_node *
UI_GetCachedNode(char *String)
{
    if(!String){InvalidCodePath;}
    
    ui_node *CachedNode = 0;
    u32 Key = UI_GetHashValue(String);
    for(ui_node *Search = UI_State->CacheFirst;
        Search != 0;
        Search = Search->CacheNext)
    {
        if(Search->Key == Key)
        {
            CachedNode = Search;
            break;
        }
    }
    
    return(CachedNode);
}

internal void
UI_BeginInteract()
{
    ui_node *CachedNode = UI_GetCachedNode(UI_State->HotInteraction->String);
    if(!CachedNode){InvalidCodePath;}
    
    if(CachedNode->Flags & UI_NodeFlag_Clickable)
    {
        Log->Add("Interact (left clicked) = %s\n", UI_State->HotInteraction->String);
        
        CachedNode->Flags |= UI_NodeFlag_Pressed;
        
        UI_State->Interaction = UI_State->HotInteraction;
    }
}

internal void
UI_EndInteract()
{
    ui_node *CachedNode = UI_GetCachedNode(UI_State->Interaction->String);
    if(!CachedNode){InvalidCodePath;}
    
    Log->Add("Interact (left released) = %s\n", UI_State->Interaction->String);
    
    CachedNode->Flags |= UI_NodeFlag_Released;
    
    UI_State->Interaction = 0;
}

internal void
UI_Interact()
{
    if(UI_State->Interaction)
    {
        // NOTE(ezexff): button pressed and mouse move interaction
        ui_node *CachedNode = UI_GetCachedNode(UI_State->Interaction->String);
        if(!CachedNode){InvalidCodePath;}
        CachedNode->Flags |= UI_NodeFlag_Dragging;
        
#if 0        
        switch(UI_State->Interaction->InteractionType)
        {
            /* 
                        case DebugInteraction_DragValue:
                        {
                            debug_event *Event = DebugState->Interaction.Element ? 
                                &DebugState->Interaction.Element->Frames[FrameOrdinal].MostRecentEvent->Event : 0;
                            switch(Event->Type)
                            {
                                case DebugType_r32:
                                {
                                    Event->Value_r32 += 0.1f*dMouseP.y;
                                } break;
                            }
                            DEBUGMarkEditedEvent(DebugState, Event);
                        } break;
             */
            
            /* 
                        case DebugInteraction_Resize:
                        {
                            *P += V2(dMouseP.x, -dMouseP.y);
                            P->x = Maximum(P->x, 10.0f);
                            P->y = Maximum(P->y, 10.0f);
                        } break;
             */
            
            case UI_Interaction_Move:
            {
                //*P += V2(dMouseP.x, dMouseP.y);
                Log->Add("UI_Interaction_Move = %s\n", UI_State->Interaction->String);
                //CachedNode->OffsetP += V2(dMouseP.x, -dMouseP.y);
                CachedNode->OffsetP.x += UI_State->Input->dMouseP.x;
                CachedNode->OffsetP.y -= UI_State->Input->dMouseP.y;
            } break;
            
            default:
            {
                //*P += V2(dMouseP.x, dMouseP.y);
                Log->Add("no interaction = %s\n", UI_State->Interaction->String);
            } break;
        }
#endif
        
        // NOTE(ezexff): click interaction
        for(u32 TransitionIndex = UI_State->Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
            TransitionIndex > 1;
            --TransitionIndex)
        {
            Log->Add("Interact (1) = %s\n", UI_State->Interaction->String);
            //UI_BeginInteract();
            //UI_EndInteract();
        }
        
        if(!UI_State->Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
        {
            CachedNode->Flags &= ~UI_NodeFlag_Dragging;
            UI_EndInteract();
            //UI_State->Interaction = 0;
        }
    }
    else
    {
        UI_State->HotInteraction = UI_State->NextHotInteraction;
        
        // NOTE(ezexff): hover
        if(UI_State->HotInteraction)
        {
            ui_node *CachedNode = UI_GetCachedNode(UI_State->HotInteraction->String);
            if(!CachedNode){InvalidCodePath;}
            if(CachedNode->Flags & UI_NodeFlag_Clickable)
            {
                Log->Add("Interact (hover) = %s\n", UI_State->HotInteraction->String);
                CachedNode->Flags |= UI_NodeFlag_Hovering;
            }
            //CachedNode->BackgroundColor = UI_State->StyleTemplateArray[UI_State->HotInteraction->StyleTemplateIndex].HoveringColor;
            
            // NOTE(ezexff): click interaction
            for(u32 TransitionIndex = UI_State->Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
                TransitionIndex > 1;
                --TransitionIndex)
            {
                Log->Add("Interact (2) = %s\n", UI_State->HotInteraction->String);
                //UI_BeginInteract();
                //UI_EndInteract();
            }
            
            if(UI_State->Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
            {
                UI_BeginInteract();
            }
        }
    }
}

internal v2
UI_CalcTextSizeInPixels(renderer_frame *Frame, game_assets *Assets, font_id FontID, char *String)
{
    v2 Result = {};
    
    if(!String)
    {
        InvalidCodePath;
    }
    
    loaded_font *Font = PushFont(Frame, Assets, FontID);
    if(Font)
    {
        eab_font *FontInfo = GetFontInfo(Assets, FontID);
        Result.y = (FontInfo->Ascent - FontInfo->Descent) * FontInfo->Scale;
        
        s32 AtX = 0;
        for(char *At = String;
            *At;
            )
        {
            u32 CodePoint = *At;
            if(CodePoint == '#')
            {
                break;
            }
            
            s32 AdvanceX = RoundR32ToS32(Font->Advances[CodePoint] * FontInfo->Scale);
            AtX += AdvanceX;
            ++At;
        }
        
        Result.x = (r32)AtX;
    }
    return(Result);
}


internal void
UI_DrawLabel(ui_node *Node)
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
        
        r32 CharScale = FontScale;
        v4 Color = V4(1, 1, 1, 1);
        
        s32 LeftPadding = 0;
        s32 AtX = LeftPadding + (s32)Node->Rect.Min.x;
        r32 HalfRectY = (Node->Rect.Max.y - Node->Rect.Min.y) / 2;
        s32 HalfFontY = 10 / 2;
        s32 AtY = (s32)Node->Rect.Min.y + (s32)(HalfRectY) + HalfFontY;
        
        //r32 RectWidth = Node->Rect.Max.x - Node->Rect.Min.x;
        v2 RectDim = GetDim(Node->Rect);
        
        for(char *At = Node->String;
            *At;
            )
        {
            u32 CodePoint = *At;
            if(CodePoint == '#')
            {
                break;
            }
            
            s32 Ascent = RoundR32ToS32(FontInfo->Ascent * FontScale);
            s32 LSB = Font->LSBs[CodePoint];
            s32 XOffset = s32(Font->GlyphOffsets[CodePoint].x);
            s32 YOffset = s32(Font->GlyphOffsets[CodePoint].y);
            
            //if(CodePoint != ' ')
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
                v2 Min = Pos;
                v2 Max = Pos + GlyphDim;
                
                v2 TestMax = Max - Node->Rect.Min;
                if(TestMax.x > RectDim.x)
                {
                    break;
                }
                
                if(TestMax.y > RectDim.y)
                {
                    break;
                }
                
                renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
                PushBitmapOnScreen(&Renderer->PushBufferUI, Assets, BitmapID, Min, Max, 10000, 1.0f);
            }
            
            s32 AdvanceX = RoundR32ToS32(Font->Advances[CodePoint] * CharScale);
            AtX += AdvanceX;
            ++At;
            
            /* 
                        if(AtX > RectWidth)
                        {
                            InvalidCodePath;
                        }
             */
        }
    }
}


inline ui_style_template
UI_GetSelectedStyleTemplate()
{
    ui_style_template Result = UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex];
    return(Result);
}

ui_node *UI_AddNodeVer2(ui_node *Parent, u32 Flags, char *String)
{
    if(!Parent){InvalidCodePath;}
    rectangle2 Rect = {};
    Rect = {Parent->Rect.Min, Parent->Rect.Min};
    
    // NOTE(ezexff): get data from template
    //r32 Padding = UI_GetSelectedStyleTemplate().Padding;
    ui_size Size[Axis2_Count] = {};
    Size[Axis2_X] = UI_GetSelectedStyleTemplate().Size[Axis2_X];
    Size[Axis2_Y] = UI_GetSelectedStyleTemplate().Size[Axis2_Y];
    v2 OffsetP = UI_GetSelectedStyleTemplate().OffsetP;
    
    // NOTE(ezexff): calc pos after prev node
    if(Parent->Last)
    {
        if(Parent != UI_State->Root) // TODO(ezexff): tmp
        {
            if(!(Flags & UI_NodeFlag_Floating))
            {
                if(Parent->LayoutAxis == Axis2_X)
                {
                    Rect.Min = V2(Parent->Last->Rect.Max.x + Parent->Spacing, Parent->Rect.Min.y);
                }
                else if(Parent->LayoutAxis == Axis2_Y)
                {
                    Rect.Min = V2(Parent->Rect.Min.x, Parent->Last->Rect.Max.y + Parent->Spacing);
                }
                
                Rect.Max = Rect.Min;
            }
        }
    }
    
    // NOTE(ezexff): calc size
    v2 LabelSize = UI_CalcTextSizeInPixels(UI_State->Frame, UI_State->Assets, UI_State->FontID, String);
    
    for(u32 Index = 0;
        Index < Axis2_Count;
        ++Index)
    {
        switch(Size[Index].Type)
        {
            case UI_SizeKind_Pixels:
            {
                Rect.Max.E[Index] += Size[Index].Value;
            } break;
            
            case UI_SizeKind_TextContent:
            {
                Size[Index].Value = LabelSize.E[Index];
                Rect.Max.E[Index] += Size[Index].Value;
            } break;
            
            case UI_SizeKind_ParentPercent:
            {
                // TODO(ezexff): Mb sub size prev nodes?
                Rect.Max.E[Index] += Size[Index].Value * (Parent->Rect.Max.E[Index] - Parent->Rect.Min.E[Index]);
            } break;
            
            case UI_SizeKind_ChildrenSum:
            {
                // TODO(ezexff): Need test
                //Parent->Rect.Max.E[Index] += Size[Index].Value;
            } break;
            
            InvalidDefaultCase;
        }
        
        if(Parent->Size[Index].Type == UI_SizeKind_ChildrenSum)
        {
            Parent->Rect.Max.E[Index] += Size[Index].Value;
        }
    }
    
    // TODO(ezexff): tmp test
    //|| StringsAreEqual(String, "EmptySpace")
    /* 
        if(StringsAreEqual(String, "ResizeButton"))
        {
            Rect = {Parent->Rect.Min, Parent->Rect.Min};
        }
     */
    if(!IsInRectangle(Parent->Rect, Rect.Min))
    {
        return(0);
        //InvalidCodePath;
    }
    
    // NOTE(ezexff): padding
    if(Rect.Min.x == Parent->Rect.Min.x)
    {
        Rect.Min.x += Parent->Padding;
        Rect.Max.x += Parent->Padding;
    }
    
    if(Rect.Min.y == Parent->Rect.Min.y)
    {
        Rect.Min.y += Parent->Padding;
        Rect.Max.y += Parent->Padding;
    }
    
    // NOTE(ezexff): clip
    v2 ParentRectMaxWidthPadding = Parent->Rect.Max;
    ParentRectMaxWidthPadding.x -= Parent->Padding;
    ParentRectMaxWidthPadding.y -= Parent->Padding;
    if(Rect.Max.x > ParentRectMaxWidthPadding.x)
    {
        Rect.Max.x = ParentRectMaxWidthPadding.x;
    }
    if(Rect.Max.y > ParentRectMaxWidthPadding.y)
    {
        Rect.Max.y = ParentRectMaxWidthPadding.y;
    }
    
    // NOTE(ezexff): get or create cached node
    ui_node *CachedNode = UI_GetCachedNode(String);
    if(!CachedNode)
    {
        CachedNode = PushStruct(UI_State->ConstArena, ui_node);
        if(!UI_State->CacheFirst)
        {
            UI_State->CacheFirst = CachedNode;
        }
        else
        {
            CachedNode->CachePrev = UI_State->CacheLast;
            UI_State->CacheLast->CacheNext = CachedNode; 
        }
        
        UI_State->CacheLast = CachedNode;
        
        CachedNode->Key = UI_GetHashValue(String);
        CachedNode->Flags = Flags;
        CachedNode->OffsetP = OffsetP;
    }
    
    // NOTE(ezexff): offset
    Rect.Min += CachedNode->OffsetP;
    Rect.Max += CachedNode->OffsetP;
    Rect.Max += CachedNode->OffsetSize;
    
    // NOTE(ezexff): init per frame node
    ui_node *Child = PushStruct(UI_State->TranArena, ui_node);
    Child->String = PushString(UI_State->TranArena, String);
    Child->Rect = Rect;
    Child->Parent = Parent;
    Child->Size[Axis2_X] = Size[Axis2_X];
    Child->Size[Axis2_Y] = Size[Axis2_Y];
    if(!Parent->First)
    {
        Parent->First = Child;
    }
    else
    {
        Child->Prev = Parent->Last;
        Parent->Last->Next = Child;
    }
    Parent->Last = Child;
    Parent->ChildCount++;
    
#if 0    
    ui_node *Child = PushStruct(UI_State->TranArena, ui_node);
    Child->Padding = Padding;
    Child->Size[Axis2_X] = SizeX;
    Child->Size[Axis2_Y] = SizeY;
    
    Child->String = PushString(UI_State->TranArena, String);
    
    Child->Rect.Min = Parent->Rect.Min;
    Child->Rect.Max = Parent->Rect.Min;
    v2 PrevRectMax = {};
    
    // NOTE(ezexff): add to node per frame links
    Child->Parent = Parent;
    if(!Parent->First)
    {
        Parent->First = Child;
    }
    else
    {
        Child->Prev = Parent->Last;
        Parent->Last->Next = Child;
        
        // NOTE(ezexff): 
        if(Parent != UI_State->Root)
        {
            if(Parent->LayoutAxis == Axis2_X)
            {
                PrevRectMax.x = Child->Prev->Rect.Max.x + Parent->Spacing;
                PrevRectMax.y = Parent->Rect.Min.y;
            }
            else if(Parent->LayoutAxis == Axis2_Y)
            {
                PrevRectMax.x = Parent->Rect.Min.x;
                PrevRectMax.y = Child->Prev->Rect.Max.y + Parent->Spacing;
            }
        }
        
        Child->Rect.Min = PrevRectMax;
        Child->Rect.Max = PrevRectMax;
        
        // TODO(ezexff): clip?
        if(Child->Rect.Max.x > Parent->Rect.Max.x)
        {
            InvalidCodePath;
        }
        if(Child->Rect.Max.y > Parent->Rect.Max.y)
        {
            InvalidCodePath;
        }
    }
    
    Parent->Last = Child;
    
    /*     
        Child->Rect.Min = Parent->Rect.Min;
        Child->Rect.Max = Parent->Rect.Min;
             */
    
    // NOTE(ezexff): padding min
    /* 
        Child->Rect.Min.x += Parent->Padding.left;
        Child->Rect.Min.y += Parent->Padding.bottom;
     */
    
    // NOTE(ezexff): calc size
    for(u32 Index = 0;
        Index < Axis2_Count;
        ++Index)
    {
        switch(Child->Size[Index].Type)
        {
            case UI_SizeKind_Pixels:
            {
                Child->Rect.Max.E[Index] += Child->Size[Index].Value;
            } break;
            
            case UI_SizeKind_TextContent:
            {
                Child->Size[Index].Value = LabelSize.E[Index];
                Child->Rect.Max.E[Index] += Child->Size[Index].Value;
                //Child->Rect.Max.E[Index] = Child->Rect.Min.E[Index] + Child->Size[Index].Value;
            } break;
            
            case UI_SizeKind_ParentPercent:
            {
                // TODO(ezexff): Mb sub size prev nodes?
                //Child->Rect.Min.E[Index] = PrevRectMax.E[Index];
                Child->Rect.Max.E[Index] += Child->Size[Index].Value * (Parent->Rect.Max.E[Index] - Parent->Rect.Min.E[Index]);
                //PrevRectMax.E[Index] = 0;
            } break;
            
            case UI_SizeKind_ChildrenSum:
            {
                // TODO(ezexff): Need test
                Parent->Rect.Max.E[Index] += Child->Size[Index].Value;
            } break;
            
            InvalidDefaultCase;
        }
    }
    
    // NOTE(ezexff): padding max
    /* 
        Child->Rect.Max.x -= Parent->Padding.right;
        Child->Rect.Max.y -= Parent->Padding.top;
     */
    
    // TODO(ezexff): Is spacing advanced twice?
    //Child->Rect.Min += PrevRectMax;
    //Child->Rect.Max += PrevRectMax;
    
    if(!IsInRectangle(Parent->Rect, Child->Rect.Min) || !IsInRectangle(Parent->Rect, Child->Rect.Max))
    {
        if((Parent->Rect.Min.x != Child->Rect.Min.x) && (Parent->Rect.Min.y != Child->Rect.Min.y) &&
           (Parent->Rect.Max.x != Child->Rect.Max.x) && (Parent->Rect.Max.y != Child->Rect.Max.y))
        {
            // Child outside parent space
            InvalidCodePath;
        }
    }
#endif
    
    return(Child);
}

u32 UI_GetNodeState(ui_node *Node)
{
    if(!Node){InvalidCodePath;}
    
    ui_node *CachedNode = UI_GetCachedNode(Node->String);
    if(!CachedNode){InvalidCodePath;}
    
#if 0    
    ui_style_template *Template = &UI_State->StyleTemplateArray[UI_State->SelectedTemplateIndex];
    
    if(CachedNode->Flags & UI_NodeFlag_DrawBackground)
    {
        CachedNode->BackgroundColor = Template->BackgroundColor;
    }
    
    rectangle2 NodeRect = Node->Rect;
    NodeRect.Max.y = UI_State->Frame->Dim.y - Node->Rect.Min.y;
    NodeRect.Min.y = UI_State->Frame->Dim.y - Node->Rect.Max.y;
    if(CachedNode->Flags & UI_NodeFlag_Clickable)
    {
        v2 MouseP = V2((r32)UI_State->Input->MouseP.x, (r32)UI_State->Input->MouseP.y);
        if(IsInRectangle(NodeRect, MouseP))
        {
            // NOTE(ezexff): start hover
            CachedNode->Flags |= UI_NodeFlag_Hovering;
            CachedNode->BackgroundColor = Template->HoveringColor;
            
            // NOTE(ezexff): start dragging
            if(CachedNode->Flags & UI_NodeFlag_Pressed)
            {
                if(IsDown(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
                {
                    CachedNode->Flags |= UI_NodeFlag_Dragging;
                }
            }
            
            // NOTE(ezexff): pressed
            if(WasPressed(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
            {
                CachedNode->Flags |= UI_NodeFlag_Pressed;
            }
            else
            {
                CachedNode->Flags &= ~UI_NodeFlag_Pressed;
            }
        }
        else
        {
            // NOTE(ezexff): end hover
            CachedNode->Flags &= ~UI_NodeFlag_Hovering;
            
            // NOTE(ezexff): while dragging
            if(CachedNode->Flags & UI_NodeFlag_Dragging)
            {
                CachedNode->BackgroundColor = Template->HoveringColor;
            }
        }
        
        // NOTE(ezexff): end dragging
        if(!IsDown(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
        {
            CachedNode->Flags &= ~UI_NodeFlag_Dragging;
        }
        
        /* 
                game_button_state PrevFrameMouseButtonLeftState = UI_State->PressKeyHistory[PlatformMouseButton_Left][0];
                v2 PrevFrameMouseP = V2((r32)UI_State->MousePHistory[0].x ,(r32)UI_State->MousePHistory[0].y);
                if(WasPressed(PrevFrameMouseButtonLeftState) && IsInRectangle(NodeRect, PrevFrameMouseP))
                {
                    if(IsDown(UI_State->Input->MouseButtons[PlatformMouseButton_Left]))
                    {
                        UI_State->TestIsDragging = true;
                    }
                }
         */
    }
    
#endif
    Node->Flags = CachedNode->Flags;
    return(Node->Flags);
}

void UI_UseStyleTemplate(u32 Index)
{
    Assert(Index < UI_StyleTemplate_Count);
    UI_State->SelectedTemplateIndex = Index;
}

internal void
UI_Init(memory_arena *ConstArena, memory_arena *TranArena)
{
    UI_State = PushStruct(TranArena, ui_state);
    //UI_State->TestIsDragging = false;
    
    /* 
        UI_State->CacheTableSize = 4096;
        UI_State->CacheIndex = 0;
        UI_State->CacheTable = PushArray(ConstArena, UI_State->CacheTableSize, ui_node);
     */
    
    
    // NOTE(ezexff): style templates
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
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 100.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 100.0f;
            } break;
            
            case UI_StyleTemplate_Button:
            {
                StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                /* 
                                StyleTemplate.ClickedColor = V4(0, 0, 0, 1);
                                StyleTemplate.PressedColor = V4(0, 0, 0, 1);
                 */
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 70.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 30.0f;
                /* 
                                StyleTemplate->PrefSize[Axis2_X].Type = UI_SizeKind_Pixels;
                                StyleTemplate->PrefSize[Axis2_X].Value = 50;
                                StyleTemplate->PrefSize[Axis2_Y].Type = UI_SizeKind_Pixels;
                                StyleTemplate->PrefSize[Axis2_Y].Value = 70;
                 */
            } break;
            
            case UI_StyleTemplate_Label:
            {
                StyleTemplate->BackgroundColor = V4(1, 0, 0, 1);
                StyleTemplate->HoveringColor = V4(0, 1, 0, 1);
                
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_TextContent;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_TextContent;
            } break;
            
            case UI_StyleTemplate_Checkbox:
            {
                StyleTemplate->BackgroundColor = V4(.125f, .196f, .298f, 1);
                StyleTemplate->HoveringColor = V4(.157f, .286f, .443f, 1);
                
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 30.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 30.0f;
                
                StyleTemplate->Padding = 5.0f;
                /* 
                                StyleTemplate->Padding.left = 5.0f;
                                StyleTemplate->Padding.right = 5.0f;
                                StyleTemplate->Padding.top = 5.0f;
                                StyleTemplate->Padding.bottom = 5.0f;
                 */
            } break;
            
            case UI_StyleTemplate_CheckboxMark:
            {
                StyleTemplate->BackgroundColor = RGBA(66, 150, 250, 1);
                
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_X].Value = 1.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_Y].Value = 1.0f;
                
            } break;
            
            case UI_StyleTemplate_Window:
            {
                //StyleTemplate->BackgroundColor = RGBA(22, 22, 22, 1);
                
                //StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_ChildrenSum;
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ChildrenSum;
                
                StyleTemplate->OffsetP = V2(200, 100);
                //StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                //StyleTemplate->Size[Axis2_Y].Value = 350.0f;
                
            } break;
            
            case UI_StyleTemplate_WindowTitle:
            {
                StyleTemplate->BackgroundColor = RGBA(10, 10, 10, 1);
                
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_X].Value = 1.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 30.0f;
                
            } break;
            
            case UI_StyleTemplate_WindowTitleEmptySpace:
            {
                StyleTemplate->BackgroundColor = RGBA(10, 10, 10, 1);
                StyleTemplate->HoveringColor = V4(1, 0, 0, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 0.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_Y].Value = 1.0f;
            } break;
            
            case UI_StyleTemplate_WindowTitleExitButton:
            {
                StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_TextContent;
                StyleTemplate->Size[Axis2_X].Value = 0.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_TextContent;
                StyleTemplate->Size[Axis2_Y].Value = 0.0f;
            } break;
            
            case UI_StyleTemplate_WindowBody:
            {
                StyleTemplate->BackgroundColor = RGBA(22, 22, 22, 0.9f);
                //StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                //StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_X].Value = 1.0f;
#if 0
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ParentPercent;
                StyleTemplate->Size[Axis2_Y].Value = 1.0f;
#else
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 350.0f;
#endif
            } break;
            
            case UI_StyleTemplate_WindowResizeButton:
            {
                StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 10.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 10.0f;
            } break;
            
            case UI_StyleTemplate_WindowScrollBar:
            {
                StyleTemplate->BackgroundColor = V4(1, 1, 1, 1);
                StyleTemplate->HoveringColor = V4(0, 0, 1, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 10.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 10.0f;
            } break;
            
            InvalidDefaultCase;
        }
    }
}


internal void
UI_BeginFrame(game_state *GameState, tran_state *TranState, renderer_frame *Frame, game_input *Input)
{
    // TODO(ezexff): Mb rework (do without engine services)?
    UI_State->ConstArena = &GameState->ConstArena;
    UI_State->TranArena = &TranState->TranArena;
    UI_State->Assets = TranState->Assets;
    UI_State->Frame = Frame;
    UI_State->Input = Input;
    
    // NOTE(ezexff): font
    asset_vector MatchVector = {};
    asset_vector WeightVector = {};
    MatchVector.E[Tag_FontType] = (r32)FontType_Debug;
    WeightVector.E[Tag_FontType] = 1.0f;
    UI_State->FontID = GetBestMatchFontFrom(UI_State->Assets, Asset_Font, &MatchVector, &WeightVector);
    
    // NOTE(ezexff): frame memory
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
    UI_State->Root->LayoutAxis = Axis2_Y;
    UI_State->Root->Spacing = 1.0f;
    //UI_State->Root->Padding = V4(5, 5, 5, 5);
    
    // NOTE(ezexff): keys history
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][0] = UI_State->PressKeyHistory[PlatformMouseButton_Count][1];
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][0] = UI_State->PressKeyHistory[PlatformMouseButton_Count][1];
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][1] = UI_State->Input->MouseButtons[PlatformMouseButton_Count];
    
    //UI_State->MousePHistory[0] = UI_State->MousePHistory[1];
    //UI_State->MousePHistory[0] = UI_State->MousePHistory[1];
    //UI_State->MousePHistory[1] = UI_State->Input->MouseP;
    
    // NOTE(ezexff): Root cached node
    ui_node *CachedNode = UI_GetCachedNode("Root");
    if(!CachedNode)
    {
        CachedNode = PushStruct(UI_State->ConstArena, ui_node);
        if(!UI_State->CacheFirst)
        {
            UI_State->CacheFirst = CachedNode;
        }
        else
        {
            CachedNode->CachePrev = UI_State->CacheLast;
            UI_State->CacheLast->CacheNext = CachedNode; 
        }
        
        UI_State->CacheLast = CachedNode;
        
        CachedNode->Key = UI_GetHashValue("Root");
        CachedNode->Flags = UI_NodeFlag_Clickable;
    }
}

internal void
UI_DrawNodeTree(ui_node *Node)
{
    // TODO(ezexff): Test interaction
    ui_node *CachedNode = UI_GetCachedNode(Node->String);
    if(!CachedNode){InvalidCodePath;}
    rectangle2 NodeRect = Node->Rect;
    NodeRect.Max.y = UI_State->Frame->Dim.y - Node->Rect.Min.y;
    NodeRect.Min.y = UI_State->Frame->Dim.y - Node->Rect.Max.y;
    v2 MouseP = V2((r32)UI_State->Input->MouseP.x, (r32)UI_State->Input->MouseP.y);
    if(IsInRectangle(NodeRect, MouseP))
    {
        if(UI_State->Frame->Dim.x == 0 || UI_State->Frame->Dim.y == 0)
        {
            Log->Add("FrameDim = 0\n");
        }
        
        if(CachedNode->Flags & UI_NodeFlag_Clickable)
        {
            UI_State->NextHotInteraction = Node;
        }
        else
        {
            //UI_State->NextHotInteraction = 0;
        }
    }
    
    if(Node != UI_State->Root)
    {
        renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
        if(CachedNode->Flags & UI_NodeFlag_DrawBackground)
        {
            //PushRectOnScreen(&Renderer->PushBufferUI, Node->Rect.Min, Node->Rect.Max, CachedNode->BackgroundColor, 100);
            if(CachedNode->Flags & UI_NodeFlag_Hovering)
            {
                PushRectOnScreen(&Renderer->PushBufferUI, Node->Rect.Min, Node->Rect.Max, V4(0, 0, 1, 1), 100);
            }
            else
            {
                PushRectOnScreen(&Renderer->PushBufferUI, Node->Rect.Min, Node->Rect.Max, V4(0, 0, 0, 1), 100);
            }
        }
        
        if(CachedNode->Flags & UI_NodeFlag_DrawText)
        {
            UI_DrawLabel(Node);
        }
        
        if(CachedNode->Flags & UI_NodeFlag_DrawBorder)
        {
            PushRectOutlineOnScreen(&Renderer->PushBufferUI, Node->Rect, 1, V4(1, 0, 0, 1), 100);
        }
        
        // TODO(ezexff): Test per frame clear flags
        CachedNode->Flags &= ~UI_NodeFlag_Pressed;
        CachedNode->Flags &= ~UI_NodeFlag_Hovering;
        //CachedNode->BackgroundColor = UI_State->StyleTemplateArray[Node->StyleTemplateIndex].BackgroundColor;
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
    if(UI_State->OpenWindow)
    {
        InvalidCodePath;
    }
    
    UI_DrawNodeTree(UI_State->Root);
    UI_Interact();
    /* 
        UI_State->Root->First = 0;
        UI_State->Root->Last = 0;
        UI_State->Root->Next = 0;
        UI_State->Root->Prev = 0;
     */
    
    EndTemporaryMemory(UI_State->FrameMemory);
}