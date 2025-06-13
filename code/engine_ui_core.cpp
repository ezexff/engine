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
UI_GetCachedNode(u32 Key)
{
    ui_node *CachedNode = 0;
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

ui_node *
UI_AddRootNode(ui_widget_link *WidgetArray, char *String, u32 StyleTemplateIndex, u32 Flags)
{
    if(!String){Assert("String can't be null");}
    
    rectangle2 Rect = {};
    ui_style_template Template = UI_State->StyleTemplateArray[StyleTemplateIndex];
    ui_size Size[Axis2_Count] = {};
    Size[Axis2_X] = Template.Size[Axis2_X];
    Size[Axis2_Y] = Template.Size[Axis2_Y];
    
    // NOTE(ezexff): calc size
    //v2 LabelSize = UI_CalcTextSizeInPixels(UI_State->Frame, UI_State->Assets, UI_State->FontID, String);
    
    for(u32 Index = 0;
        Index < Axis2_Count;
        ++Index)
    {
        switch(Size[Index].Type)
        {
            case UI_SizeKind_Pixels:
            {
                Rect.Max.E[Index] = Rect.Min.E[Index] + Size[Index].Value;
            } break;
            
            case UI_SizeKind_TextContent:
            {
                InvalidCodePath;
                //Rect.Max.E[Index] = Rect.Min.E[Index] + LabelSize.E[Index];
            } break;
            
            case UI_SizeKind_ParentPercent:
            {
                InvalidCodePath;
                //Rect.Max.E[Index] = Rect.Min.E[Index] + Size[Index].Value * (Parent->Rect.Max.E[Index] - Parent->Rect.Min.E[Index]);
            } break;
            
            case UI_SizeKind_ChildrenSum:
            {
            } break;
            
            InvalidDefaultCase;
        }
        
        /* 
                if((Parent->Size[Index].Type == UI_SizeKind_ChildrenSum) && !(Flags & UI_NodeFlag_Floating))
                {
                    Parent->Rect.Max.E[Index] += Rect.Max.E[Index] - Rect.Min.E[Index];
                }
         */
    }
    
    // NOTE(ezexff): get or create cache
    u32 Key = UI_GetHashValue(String);
    ui_node *CachedNode = UI_GetCachedNode(Key);
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
        
        CachedNode->Key = Key;
        CachedNode->Flags = Flags;
        CachedNode->BackgroundColor = Template.BackgroundColor;
        CachedNode->StyleTemplateIndex = StyleTemplateIndex;
    }
    
    // NOTE(ezexff): persist P and Size
    Rect.Min += CachedNode->P;
    Rect.Max += CachedNode->P;
    Rect.Max += CachedNode->Size;
    
    // NOTE(ezexff): new widget
    ui_node *Widget = PushStruct(UI_State->TranArena, ui_node);
    Widget->Cache = CachedNode;
    Widget->LayoutAxis = Axis2_Y;
    Widget->Key = CachedNode->Key;
    Widget->Rect = Rect;
    Widget->ID = PushString(UI_State->TranArena, String);
    CachedNode->ID = Widget->ID;
    
    if(!WidgetArray->First)
    {
        WidgetArray->First = Widget;
    }
    else
    {
        Widget->Prev = WidgetArray->Last;
        WidgetArray->Last->Next = Widget;
    }
    WidgetArray->Last = Widget;
    //Parent->ChildCount++;
    
    UI_State->NodeCount++;
    
    return(Widget);
}

ui_node *
UI_AddNode(ui_node *Root, ui_node *Parent, char *ID, u32 StyleTemplateIndex, u32 Flags, char *Text = 0)
{
    if(!Root){InvalidCodePath;}
    if(!Parent){InvalidCodePath;}
    if(!ID){Assert(!"ID can't be null");}
    if((StyleTemplateIndex == 0) || (StyleTemplateIndex >= UI_StyleTemplate_Count)){InvalidCodePath};
    /* 
    v2 P = {};
    v2 Dim = {};
 */
    rectangle2 Rect = {};
    Rect.Min = Parent->Rect.Min;
    //P = Parent->P;
    
    ui_style_template Template = UI_State->StyleTemplateArray[StyleTemplateIndex];
    ui_style_template ParentTemplate = UI_State->StyleTemplateArray[Parent->Cache->StyleTemplateIndex];
    ui_size Size[Axis2_Count] = {};
    Size[Axis2_X] = Template.Size[Axis2_X];
    Size[Axis2_Y] = Template.Size[Axis2_Y];
    
    // NOTE(ezexff): calc pos after prev node and only if that node have parent
    if(Parent->Last)
    {
        if(!(Flags & UI_NodeFlag_Floating))
        {
            if(Parent->LayoutAxis == Axis2_X)
            {
                Rect.Min.x = Parent->Last->Rect.Max.x;
                //if(UI_State->WindowArray.Last->Body)
                {
                    //if(Root->Body->IsOpened)
                    {
                        //Rect.Min.x -= Root->Body->ViewP.x;
                    }
                }
            }
            else if(Parent->LayoutAxis == Axis2_Y)
            {
                Rect.Min.y = Parent->Last->Rect.Max.y;
                //if(UI_State->WindowArray.Last->Body)
                {
                    //if(Root->Body->IsOpened)
                    {
                        //Rect.Min.y -= Root->Body->ViewP.y;
                    }
                }
            }
            else
            {
                InvalidCodePath;
            }
        }
    }
    
    // NOTE(ezexff): calc size
    v2 LabelSize = {};
    if(Text)
    {
        LabelSize = UI_CalcTextSizeInPixels(UI_State->Frame, UI_State->Assets, UI_State->FontID, Text);
    }
    
    for(u32 Index = 0;
        Index < Axis2_Count;
        ++Index)
    {
        switch(Size[Index].Type)
        {
            case UI_SizeKind_Pixels:
            {
                //Dim.E[Index] = Size[Index].Value;
                Rect.Max.E[Index] = Rect.Min.E[Index] + Size[Index].Value;
            } break;
            
            case UI_SizeKind_TextContent:
            {
                //Dim.E[Index] = LabelSize.E[Index];
                if(Text)
                {
                    Rect.Max.E[Index] = Rect.Min.E[Index] + LabelSize.E[Index];
                }
                else
                {
                    InvalidCodePath;
                }
            } break;
            
            case UI_SizeKind_ParentPercent:
            {
                //Dim.E[Index] = Size[Index].Value * (Parent->Rect.Max.E[Index] - Parent->Rect.Min.E[Index]);
                Rect.Max.E[Index] = Rect.Min.E[Index] + Size[Index].Value * (Parent->Rect.Max.E[Index] - Parent->Rect.Min.E[Index]);
            } break;
            
            case UI_SizeKind_ChildrenSum:
            {
                // TODO(ezexff): Need test
                //Parent->Rect.Max.E[Index] += Size[Index].Value;
            } break;
            
            InvalidDefaultCase;
        }
        
        //if((Parent->Size[Index].Type == UI_SizeKind_ChildrenSum) && !(Flags & UI_NodeFlag_Floating))
        if((ParentTemplate.Size[Index].Type == UI_SizeKind_ChildrenSum) && !(Flags & UI_NodeFlag_Floating))
        {
            //Parent->Rect.Max.E[Index] += Dim.E[Index];
            Parent->Rect.Max.E[Index] += Rect.Max.E[Index] - Rect.Min.E[Index];
        }
    }
    
    // NOTE(ezexff): view rect in window
    ui_node *Node = 0;
    //if((Parent == UI_State->Root) || UI_State->OpenWindow)
    {
        v2 StartTextOffset = {};
#if 1
        renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
        if(Root->Body && !(Flags & UI_NodeFlag_Floating))
        {
            ui_node *Body = Root->Body;
            ui_node *BodyCache = Root->Body->Cache;
            
            // NOTE(ezexff): content max
            //Body->MaxNodeDim.x = Maximum(Body->MaxNodeDim.x, Rect.Max.x - Body->Rect.Min.x);
            //Body->MaxNodeDim.y = Maximum(Body->MaxNodeDim.y, Rect.Max.y - Body->Rect.Min.y);
            
            /* 
                        if(Parent->LayoutAxis == Axis2_X)
                        {
                            Body->ContentDimSum.x += Rect.Max.x - Rect.Min.x;
                        }
                        if(Parent->LayoutAxis == Axis2_Y)
                        {
                            Body->ContentDimSum.y += Rect.Max.y - Rect.Min.y;
                        }
             */
            // NOTE(ezexff): calc window content dim
            v2 NodeDim = GetDim(Rect);
            v2 BodyDim = GetDim(Body->Rect);
            if(Parent->LayoutAxis == Axis2_X)
            {
                Body->ContentDim.x += NodeDim.x;
                if(NodeDim.y > BodyDim.y)
                {
                    Body->ContentDim.y = NodeDim.y;
                }
            }
            else if(Parent->LayoutAxis == Axis2_Y)
            {
                Body->ContentDim.y += NodeDim.y;
                if(NodeDim.x > BodyDim.x)
                {
                    Body->ContentDim.x = NodeDim.x;
                }
            }
            else
            {
                InvalidCodePath;
            }
            
            if(BodyCache)
            {
                // NOTE(ezexff): смещаем первый элемент в зависимости от позиции области просмотра
                // NOTE(ezexff): позиции последующих звеньев благодаря автовыравниванию будут тоже смещены
                if(!Parent->First)
                {
                    Rect = {Rect.Min + BodyCache->ViewP, Rect.Max + BodyCache->ViewP};
                }
                else
                {
                    Rect = {Rect.Min, Rect.Max};
                }
                
                //rectangle2 ViewRect = {Body->Rect.Min + BodyCache->ViewP, Body->Rect.Max + BodyCache->ViewP};
                //PushRectOutlineOnScreen(&Renderer->PushBufferUI, ViewRect, 1,  V4(0, 0, 1, 1), 101);
            }
            else
            {
                InvalidCodePath;
            }
        }
        
        
        /*         
                // NOTE(ezexff): Clip nodes that outside window body
                if(Root->Body && !(Flags & UI_NodeFlag_Floating))
                {
                    ui_node *Body = Root->Body;
                    
                                // NOTE(ezexff): clip x
                                if(Rect.Min.x < Body->Rect.Min.x)
                                {
                                    StartTextOffset.x = Body->Rect.Min.x - Rect.Min.x;
                                    Rect.Min.x = Body->Rect.Min.x;
                                }
                                if(Rect.Max.x > Body->Rect.Max.x){Rect.Max.x = Body->Rect.Max.x;}
                                if(Rect.Max.x < Body->Rect.Min.x){Rect.Max.x = Body->Rect.Min.x;}
                                if(Rect.Min.x > Body->Rect.Max.x){Rect.Min.x = Body->Rect.Max.x;}
                                
                                // NOTE(ezexff): clip y
                                if(Rect.Min.y < Body->Rect.Min.y)
                                {
                                    StartTextOffset.y = Body->Rect.Min.y - Rect.Min.y;
                                    Rect.Min.y = Body->Rect.Min.y;
                                }
                                if(Rect.Max.y > Body->Rect.Max.y){Rect.Max.y = Body->Rect.Max.y;}
                                if(Rect.Max.y < Body->Rect.Min.y){Rect.Max.y = Body->Rect.Min.y;}
                                if(Rect.Min.y > Body->Rect.Max.y){Rect.Min.y = Body->Rect.Max.y;}
                }
         */
#endif
        
        // NOTE(ezexff): get or create cached node
        u32 Key = UI_GetHashValue(ID);
        if(Key == 172056332){InvalidCodePath;};
        ui_node *CachedNode = UI_GetCachedNode(Key);
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
            CachedNode->CacheNext = 0;
            
            CachedNode->Key = Key;
            CachedNode->Flags = Flags;
            
            CachedNode->Rect = Rect;
            CachedNode->StyleTemplateIndex = StyleTemplateIndex;
        }
        
        // NOTE(ezexff): persist P and Size
        Rect.Min += CachedNode->P;
        Rect.Max += CachedNode->P;
        Rect.Max += CachedNode->Size;
        
        // NOTE(ezexff): init per frame node
        Node = PushStruct(UI_State->TranArena, ui_node);
        Node->Cache = CachedNode;
        Node->LayoutAxis = Axis2_Invalid;
        Node->Key = CachedNode->Key;
        Node->Rect = Rect;
        //Node->StartTextOffset = StartTextOffset;
        Node->Parent = Parent;
        Node->Root = Root;
        Node->ID = PushString(UI_State->TranArena, ID);
        CachedNode->ID = Node->ID;
        if(Text)
        {
            Node->Text = PushString(UI_State->TranArena, Text);
        }
        //Node->Size[Axis2_X] = Size[Axis2_X];
        //Child->Size[Axis2_Y] = Size[Axis2_Y];
        //Child->ViewP = CachedNode->ViewP;
        if(!Parent->First)
        {
            Parent->First = Node;
        }
        else
        {
            Node->Prev = Parent->Last;
            Parent->Last->Next = Node;
        }
        Parent->Last = Node;
        Parent->ChildCount++;
        
        UI_State->NodeCount++;
        
    }
    
    if(!Node){InvalidCodePath;}
    return(Node);
}

internal void
UI_BeginInteract()
{
    //ui_node *CachedNode = UI_GetCachedNode(UI_State->HotInteraction->String);
    //if(!CachedNode){InvalidCodePath;}
    ui_node *CachedNode = UI_State->HotInteraction->Cache;
    if(CachedNode->Flags & UI_NodeFlag_Clickable)
    {
        Log->Add("Interact (left pressed) = %s\n", UI_State->HotInteraction->ID);
        
        CachedNode->Flags |= UI_NodeFlag_Pressed;
        // TODO(ezexff): Test in node mouse p
        {
            //CachedNode->PressMouseP = UI_State->Input->MouseP;
            CachedNode->PressMouseP.x = UI_State->Input->MouseP.x - UI_State->HotInteraction->Rect.Min.x;
            CachedNode->PressMouseP.y = UI_State->Frame->Dim.y - UI_State->Input->MouseP.y - UI_State->HotInteraction->Rect.Min.y;
            Log->Add("CachedNode->PressMouseP.y = %f\n", CachedNode->PressMouseP.y);
            //r32 TestY = UI_State->HotInteraction->Rect.Min.y - UI_State->Input->MouseP.y;
            
            UI_State->HotInteraction->Cache->LastFrameTouchedIndex = UI_State->FrameCount;
            if(UI_State->HotInteraction->Root)
            {
                UI_State->HotInteraction->Root->Cache->LastFrameTouchedIndex = UI_State->FrameCount;
                Log->Add("%s %llu\n", UI_State->HotInteraction->ID, CachedNode->LastFrameTouchedIndex);
            }
        }
        if(!UI_State->HotInteraction->Key){InvalidCodePath;}
        UI_State->Interaction = UI_State->HotInteraction;
        UI_State->HotInteraction = 0;
    }
}

internal void
UI_EndInteract()
{
    //ui_node *CachedNode = UI_GetCachedNode(UI_State->Interaction->String);
    //if(!CachedNode){InvalidCodePath;}
    ui_node *Node = UI_State->Interaction;
    ui_node *CachedNode = Node->Cache;
    
    Log->Add("Interact (left released) = %s\n", Node->ID);
    
    CachedNode->Flags |= UI_NodeFlag_Released;
    CachedNode->PressMouseP = V2(0.0f, 0.0f);
    
    UI_State->Interaction = 0;
}

internal void
UI_Interact()
{
    if(UI_State->Interaction)
    {
        if(!UI_State->Interaction->Key){InvalidCodePath;}
        // NOTE(ezexff): button pressed and mouse move interaction
        //ui_node *CachedNode = UI_GetCachedNode(UI_State->Interaction->String);
        //if(!CachedNode){InvalidCodePath;}
        ui_node *CachedNode = UI_State->Interaction->Cache;
        CachedNode->Flags |= UI_NodeFlag_Dragging;
        
        // NOTE(ezexff): click interaction
        for(u32 TransitionIndex = UI_State->Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
            TransitionIndex > 1;
            --TransitionIndex)
        {
            Log->Add("Interact (1) = %s\n", UI_State->Interaction->ID);
            //UI_BeginInteract();
            //UI_EndInteract();
        }
        
        if(!UI_State->Input->MouseButtons[PlatformMouseButton_Left].EndedDown)
        {
            Log->Add("Interact (left clicked) = %s\n", UI_State->Interaction->ID);
            CachedNode->Flags &= ~UI_NodeFlag_Dragging;
            CachedNode->Flags &= ~UI_NodeFlag_Clicked;
            UI_EndInteract();
            //UI_State->Interaction = 0;
        }
    }
    else
    {
        UI_State->HotInteraction = UI_State->NextHotInteraction;
        UI_State->NextHotInteraction = 0;
        
        // NOTE(ezexff): hover
        if(UI_State->HotInteraction)
        {
            if(!UI_State->HotInteraction->Key){InvalidCodePath;}
            //ui_node *CachedNode = UI_GetCachedNode(UI_State->HotInteraction->String);
            //if(!CachedNode){InvalidCodePath;}
            ui_node *CachedNode = UI_State->HotInteraction->Cache;
            if(CachedNode->Flags & UI_NodeFlag_Clickable)
            {
                //Log->Add("Interact (hover) = %s\n", UI_State->HotInteraction->String);
                CachedNode->Flags |= UI_NodeFlag_Hovering;
            }
            //CachedNode->BackgroundColor = UI_State->StyleTemplateArray[UI_State->HotInteraction->StyleTemplateIndex].HoveringColor;
            
            // NOTE(ezexff): click interaction
            for(u32 TransitionIndex = UI_State->Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount;
                TransitionIndex > 1;
                --TransitionIndex)
            {
                Log->Add("Interact (2) = %s\n", UI_State->HotInteraction->ID);
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

internal void
UI_DrawLabel(ui_node *Node, rectangle2 ClippedRect)
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
        
        //s32 LeftPadding = 0;
        s32 AtX = (s32)Node->Rect.Min.x;
        r32 HalfRectY = (Node->Rect.Max.y - Node->Rect.Min.y) / 2;
        s32 HalfFontY = 10 / 2;
        //s32 AtY = (s32)Node->Rect.Min.y + (s32)(HalfRectY) + HalfFontY;
        s32 AtY = (s32)Node->Rect.Min.y + 15;
        
        //r32 RectWidth = Node->Rect.Max.x - Node->Rect.Min.x;
        v2 RectDim = GetDim(Node->Rect);
        
        for(char *At = Node->Text;
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
#if 1
                v2 GlyphDim = V2((r32)GlyphInfo->Dim[0], (r32)GlyphInfo->Dim[1]);
                v2 GlyphPos = V2((r32)(AtX + XOffset), (r32)(AtY + YOffset));
                rectangle2 Rect = {GlyphPos, GlyphPos + GlyphDim};
                
                if(RectanglesIntersect(Rect, ClippedRect))
                {
                    r32 MinClipX = 0;
                    r32 MinClipUVX = 0;
                    if(Rect.Min.x < ClippedRect.Min.x)
                    {
                        MinClipX = ClippedRect.Min.x - Rect.Min.x;
                        MinClipUVX = MinClipX / GlyphDim.x;
                    }
                    r32 MaxClipX = 0;
                    r32 MaxClipUVX = 1;
                    if(Rect.Max.x > ClippedRect.Max.x)
                    {
                        MaxClipX = Rect.Max.x - ClippedRect.Max.x;
                        MaxClipUVX = 1 - MaxClipX / GlyphDim.x;
                    }
                    
                    Rect.Min.x += MinClipX;
                    Rect.Max.x -= MaxClipX;
                    
                    // NOTE(ezexff): clip y
                    r32 MinClipY = 0;
                    r32 MinClipUVY = 0;
                    if(Rect.Min.y < ClippedRect.Min.y)
                    {
                        MinClipY = ClippedRect.Min.y - Rect.Min.y;
                        MinClipUVY = MinClipY / GlyphDim.y;
                    }
                    r32 MaxClipY = 0;
                    r32 MaxClipUVY = 1;
                    if(Rect.Max.y > ClippedRect.Max.y)
                    {
                        MaxClipY = Rect.Max.y - ClippedRect.Max.y;
                        MaxClipUVY = 1 - MaxClipY / GlyphDim.y;
                    }
                    
                    Rect.Min.y += MinClipY;
                    Rect.Max.y -= MaxClipY;
                    
                    r32 TexCoords[8] = 
                    {MinClipUVX, MinClipUVY,
                        MaxClipUVX, MinClipUVY,
                        MaxClipUVX, MaxClipUVY,
                        MinClipUVX, MaxClipUVY,
                    };
                    
                    renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
                    PushBitmapOnScreen(&Renderer->PushBufferUI, Assets, BitmapID, Rect, 100, TexCoords);
                }
#else
                v2 GlyphDim;
                GlyphDim.x = (r32)GlyphInfo->Dim[0];
                GlyphDim.y = (r32)GlyphInfo->Dim[1];
                v2 Pos = V2(0, 0);
                Pos.x = (r32)(AtX + XOffset - Node->StartTextOffset.x);
                Pos.y = (r32)(AtY + YOffset - Node->StartTextOffset.y);
                rectangle2 Rect = {Pos, Pos + GlyphDim};
                // NOTE(ezexff): skip glyph that outside node->rect
                if(((Rect.Max.x > Node->Rect.Min.x) && (Rect.Min.x < Node->Rect.Max.x)) &&
                   ((Rect.Max.y > Node->Rect.Min.y) && (Rect.Min.y < Node->Rect.Max.y)))
                {
                    // NOTE(ezexff): clip when glyph on node->rect border
                    // NOTE(ezexff): clip x
                    r32 MinClipX = 0;
                    r32 MinClipUVX = 0;
                    if(Rect.Min.x < Node->Rect.Min.x)
                    {
                        MinClipX = Node->Rect.Min.x - Rect.Min.x;
                        MinClipUVX = MinClipX / GlyphDim.x;
                    }
                    r32 MaxClipX = 0;
                    r32 MaxClipUVX = 1;
                    if(Rect.Max.x > Node->Rect.Max.x)
                    {
                        MaxClipX = Rect.Max.x - Node->Rect.Max.x;
                        MaxClipUVX = 1 - MaxClipX / GlyphDim.x;
                    }
                    
                    Rect.Min.x += MinClipX;
                    Rect.Max.x -= MaxClipX;
                    
                    // NOTE(ezexff): clip y
                    r32 MinClipY = 0;
                    r32 MinClipUVY = 0;
                    if(Rect.Min.y < Node->Rect.Min.y)
                    {
                        MinClipY = Node->Rect.Min.y - Rect.Min.y;
                        MinClipUVY = MinClipY / GlyphDim.y;
                    }
                    r32 MaxClipY = 0;
                    r32 MaxClipUVY = 1;
                    if(Rect.Max.y > Node->Rect.Max.y)
                    {
                        MaxClipY = Rect.Max.y - Node->Rect.Max.y;
                        MaxClipUVY = 1 - MaxClipY / GlyphDim.y;
                    }
                    
                    Rect.Min.y += MinClipY;
                    Rect.Max.y -= MaxClipY;
                    
                    r32 TexCoords[8] = 
                    {MinClipUVX, MinClipUVY,
                        MaxClipUVX, MinClipUVY,
                        MaxClipUVX, MaxClipUVY,
                        MinClipUVX, MaxClipUVY,
                    };
                    renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
                    PushBitmapOnScreen(&Renderer->PushBufferUI, Assets, BitmapID, Rect, 100, TexCoords);
                }
#endif
                /* 
                v2 TestMax = Max - Node->Rect.Min;
                if(TestMax.x > RectDim.x)
                {
                    break;
                }
                
                if(TestMax.y > RectDim.y)
                {
                    break;
                }
    */
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

u32 UI_GetNodeState(ui_node *Node)
{
    u32 Result = Node->Cache->Flags;
    return(Result);
}

internal void
UI_Init(memory_arena *ConstArena, memory_arena *TranArena)
{
    UI_State = PushStruct(TranArena, ui_state);
    UI_State->FrameCount = 0;
    
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
            
            case UI_StyleTemplate_Window1:
            {
                StyleTemplate->BackgroundColor = V4(0, 0, 0, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                //StyleTemplate->Size[Axis2_Y].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ChildrenSum;
                
                //StyleTemplate->OffsetP = V2(200, 100);
                //StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                //StyleTemplate->Size[Axis2_Y].Value = 350.0f;
                
            } break;
            
            case UI_StyleTemplate_Window2:
            {
                StyleTemplate->BackgroundColor = V4(0, 1, 0, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                //StyleTemplate->Size[Axis2_Y].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ChildrenSum;
            } break;
            
            case UI_StyleTemplate_Window3:
            {
                StyleTemplate->BackgroundColor = V4(1, 0, 0, 1);
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                //StyleTemplate->Size[Axis2_Y].Value = 400.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_ChildrenSum;
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
                StyleTemplate->Size[Axis2_X].Value = 10.0f;
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
                /* 
StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_ParentPercent;
StyleTemplate->Size[Axis2_X].Value = 1.0f;
StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
StyleTemplate->Size[Axis2_Y].Value = 10.0f;
*/
                StyleTemplate->Size[Axis2_X].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_X].Value = 0.0f;
                StyleTemplate->Size[Axis2_Y].Type = UI_SizeKind_Pixels;
                StyleTemplate->Size[Axis2_Y].Value = 0.0f;
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
    /* 
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
     */
    
    // NOTE(ezexff): keys history
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][0] = UI_State->PressKeyHistory[PlatformMouseButton_Count][1];
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][0] = UI_State->PressKeyHistory[PlatformMouseButton_Count][1];
    //UI_State->PressKeyHistory[PlatformMouseButton_Count][1] = UI_State->Input->MouseButtons[PlatformMouseButton_Count];
    
    //UI_State->MousePHistory[0] = UI_State->MousePHistory[1];
    //UI_State->MousePHistory[0] = UI_State->MousePHistory[1];
    //UI_State->MousePHistory[1] = UI_State->Input->MouseP;
    
    // NOTE(ezexff): Root cached node
    /* 
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
 */
}

internal void
UI_ProcessNodeTree(ui_node *Node)
{
    if(!Node->Key){InvalidCodePath;}
    
    // TODO(ezexff): Test interaction
    rectangle2 ClippedRect = Node->Rect;
    {
        if(!(Node->Cache->Flags & UI_NodeFlag_Floating))
        {
            if(Node->Root)
            {
                if(Node->Root->Body == Node->Parent)
                {
                    if(RectanglesIntersect(Node->Rect, Node->Root->Body->Rect))
                    {
                        // NOTE(ezexff): clip left
                        if(Node->Rect.Min.x < Node->Root->Body->Rect.Min.x)
                        {
                            ClippedRect.Min.x = Node->Root->Body->Rect.Min.x;
                        }
                        
                        // NOTE(ezexff): clip right
                        if(Node->Rect.Max.x > Node->Root->Body->Rect.Max.x)
                        {
                            ClippedRect.Max.x = Node->Root->Body->Rect.Max.x;
                        }
                        
                        // NOTE(ezexff): clip top
                        if(Node->Rect.Min.y < Node->Root->Body->Rect.Min.y)
                        {
                            ClippedRect.Min.y = Node->Root->Body->Rect.Min.y;
                        }
                        
                        // NOTE(ezexff): clip bot
                        if(Node->Rect.Max.y > Node->Root->Body->Rect.Max.y)
                        {
                            ClippedRect.Max.y = Node->Root->Body->Rect.Max.y;
                        }
                    }
                    else
                    {
                        // NOTE(ezexff): skip node that outside body
                        return;
                    }
                }
            }
        }
        
        rectangle2 TestRect = ClippedRect;
        TestRect.Max.y = UI_State->Frame->Dim.y - ClippedRect.Min.y;
        TestRect.Min.y = UI_State->Frame->Dim.y - ClippedRect.Max.y;
        /* 
                TestRect.Max.y = UI_State->Frame->Dim.y - Node->Rect.Min.y;
                TestRect.Min.y = UI_State->Frame->Dim.y - Node->Rect.Max.y;
         */
        v2 MouseP = V2((r32)UI_State->Input->MouseP.x, (r32)UI_State->Input->MouseP.y);
        if(IsInRectangle(TestRect, MouseP))
        {
            if(UI_State->Frame->Dim.x == 0 || UI_State->Frame->Dim.y == 0)
            {
                Log->Add("FrameDim = 0\n");
            }
            
            if(Node->Cache->Flags & UI_NodeFlag_Clickable)
            {
                UI_State->NextHotInteraction = Node;
            }
        }
        
        //if(Node != UI_State->Root)
        {
            renderer *Renderer = (renderer *)UI_State->Frame->Renderer;
            if(Node->Cache->Flags & UI_NodeFlag_DrawBackground)
            {
                if(Node->Cache->Flags & UI_NodeFlag_Hovering)
                {
                    PushRectOnScreen(&Renderer->PushBufferUI, ClippedRect.Min, ClippedRect.Max, V4(0, 0, 1, 1), 100);
                }
                else
                {
                    PushRectOnScreen(&Renderer->PushBufferUI, ClippedRect.Min, ClippedRect.Max, Node->Cache->BackgroundColor, 100);
                }
            }
            
            if(Node->Cache->Flags & UI_NodeFlag_DrawText)
            {
                UI_DrawLabel(Node, ClippedRect);
            }
            
            if(Node->Cache->Flags & UI_NodeFlag_DrawBorder)
            {
                PushRectOutlineOnScreen(&Renderer->PushBufferUI, ClippedRect, 1, V4(1, 0, 0, 1), 100);
            }
        }
        
        // TODO(ezexff): Test per frame clear flags
        Node->Cache->Flags &= ~UI_NodeFlag_Pressed;
        Node->Cache->Flags &= ~UI_NodeFlag_Hovering;
    }
    
    // NOTE(ezexff): Process childs
    if(Node->First)
    {
        for(ui_node *ChildNode = Node->First;
            ChildNode != 0;
            ChildNode = ChildNode->Next)
        {
            UI_ProcessNodeTree(ChildNode);
        }
    }
}

struct ui_node_sort_entry
{
    ui_node *Node;
};

internal void
UI_EndFrame()
{
    //if(UI_State->OpenWindow)
    {
        //InvalidCodePath;
    }
    
    //UI_DrawNodeTree(UI_State->Root);
    
    // NOTE(ezexff): sort windows by last frame touch
    Assert(UI_State->WindowCount > 0);
    u32 Count = UI_State->WindowCount;
    ui_node_sort_entry *SortWindowArray = PushArray(UI_State->TranArena, Count, ui_node_sort_entry);
    u32 SortIndex = 0;
    for(ui_node *Node = UI_State->WindowArray.First;
        Node != 0;
        Node = Node->Next)
    {
        ui_node_sort_entry *ArrayLink = SortWindowArray + SortIndex;
        ArrayLink->Node = Node;
        SortIndex++;
    }
    
    for(u32 Outer = 0;
        Outer < Count;
        ++Outer)
    {
        b32 ListIsSorted = true;
        for(u32 Inner = 0;
            Inner < (Count - 1);
            ++Inner)
        {
            ui_node_sort_entry *EntryA = SortWindowArray + Inner;
            ui_node_sort_entry *EntryB = EntryA + 1;
            
            if(EntryA->Node->Cache->LastFrameTouchedIndex > EntryB->Node->Cache->LastFrameTouchedIndex)
            {
                ui_node *Swap = EntryB->Node;
                EntryB->Node = EntryA->Node;
                EntryA->Node = Swap;
                ListIsSorted = false;
            }
        }
        
        if(ListIsSorted)
        {
            break;
        }
    }
    
    // NOTE(ezexff): find interaction and draw nodes
    /* 
    for(ui_node *Node = UI_State->WindowArray.First;
        Node != 0;
        Node = Node->Next)
     */
    for(u32 Index = 0;
        Index < Count;
        ++Index)
    {
        ui_node_sort_entry *ArrayLink = SortWindowArray + Index;
        UI_ProcessNodeTree(ArrayLink->Node);
    }
    
    UI_Interact();
    
    EndTemporaryMemory(UI_State->FrameMemory);
    
    
    // NOTE(ezexff): clear per build vars
    UI_State->NodeCount = 0;
    UI_State->WindowCount = 0;
    
    UI_State->FrameCount++;
}