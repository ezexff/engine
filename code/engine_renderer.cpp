#define PushRenderElement(Frame, type) (type *)PushRenderElement_(Frame, sizeof(type), RendererEntryType_##type)
void *
PushRenderElement_(renderer_frame *Frame, u32 Size, renderer_entry_type Type)
{
    void *Result = 0;
    
    Size += sizeof(renderer_entry_header);
    
    if((Frame->PushBufferSize + Size) < Frame->MaxPushBufferSize)
    {
        renderer_entry_header *Header = (renderer_entry_header *)(Frame->PushBufferBase + Frame->PushBufferSize);
        Header->Type = Type;
        Result = (u8 *)Header + sizeof(*Header);
        Frame->PushBufferSize += Size;
    }
    else
    {
        InvalidCodePath;
    }
    
    return(Result);
}

void
PushRectOnGround(renderer_frame *Frame, v2 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    //v3 P = (Offset - V3(0.5f * Dim, 0));
    v2 P = Offset - 0.5f * Dim;
    //P += Group->Transform.OffsetP;
    
    renderer_entry_rect_on_ground *Entry = PushRenderElement(Frame, renderer_entry_rect_on_ground);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
    }
}

void
PushRectOnScreen(renderer_frame *Frame, v2 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v2 P = Offset;
    //P += Group->Transform.OffsetP;
    
    renderer_entry_rect_on_screen *Entry = PushRenderElement(Frame, renderer_entry_rect_on_screen);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
    }
}

void
PushRectOutlineOnGround(renderer_frame *Frame, v2 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), r32 LineWidth = 1)
{
    //v3 P = (Offset - V3(0.5f * Dim, 0));
    v2 P = Offset - 0.5f * Dim;
    //P += Group->Transform.OffsetP;
    
    renderer_entry_rect_outline_on_ground *Entry = PushRenderElement(Frame, renderer_entry_rect_outline_on_ground);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
        Entry->LineWidth = LineWidth;
    }
}

void
PushBitmapOnGround(renderer_frame *Frame, game_assets *Assets, bitmap_id ID, v2 Offset, v2 Dim, r32 Repeat = 1)
{
    loaded_bitmap *Bitmap = GetBitmap(Assets, ID, true);
    if(Bitmap)
    {
        v2 P = (Offset - 0.5f * Dim);
        
        renderer_entry_bitmap_on_ground *Entry = PushRenderElement(Frame, renderer_entry_bitmap_on_ground);
        if(Entry)
        {
            Entry->Bitmap = Bitmap;
            Entry->P = P;
            Entry->Dim = Dim;
            Entry->Repeat = Repeat;
        }
    }
    else
    {
        LoadBitmap(Assets, ID, true);
        //++Frame->MissingResourceCount;
    }
}

void
PushBitmapOnScreen(renderer_frame *Frame, game_assets *Assets, bitmap_id ID, v2 Offset, v2 Dim, r32 Repeat = 1)
{
    loaded_bitmap *Bitmap = GetBitmap(Assets, ID, true);
    if(Bitmap)
    {
        v2 P = Offset;
        
        renderer_entry_bitmap_on_screen *Entry = PushRenderElement(Frame, renderer_entry_bitmap_on_screen);
        if(Entry)
        {
            Entry->Bitmap = Bitmap;
            Entry->P = P;
            Entry->Dim = Dim;
            Entry->Repeat = Repeat;
        }
    }
    else
    {
        LoadBitmap(Assets, ID, true);
        //++Frame->MissingResourceCount;
    }
}

// TODO(ezexff): Need rework!
inline loaded_font *
PushFont(renderer_frame *Frame, game_assets *Assets, font_id ID)
{
    loaded_font *Font = GetFont(Assets, ID, true);    
    if(Font)
    {
        // NOTE(casey): Nothing to do
    }
    else
    {
        //Assert(!Group->RendersInBackground);
        LoadFont(Assets, ID, true);
        //++Group->MissingResourceCount;
    }
    
    return(Font);
}

void
PushCube(renderer_frame *Frame, v3 Offset, v3 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v3 P = Offset - 0.5f * Dim;
    //P.z = Offset.z;
    
    renderer_entry_cube *Entry = PushRenderElement(Frame, renderer_entry_cube);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
    }
}

void
PushCubeOutline(renderer_frame *Frame, v3 Offset, v3 Dim, v4 Color = V4(1, 1, 1, 1), r32 LineWidth = 1)
{
    v3 P = Offset - 0.5f * Dim;
    //P.z = Offset.z;
    
    renderer_entry_cube_outline *Entry = PushRenderElement(Frame, renderer_entry_cube_outline);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
        Entry->LineWidth = LineWidth;
    }
}

void
PushLine(renderer_frame *Frame, v3 P1, v3 P2, v4 Color = V4(0.5f, 0.0f, 0.5f, 1.0f), r32 LineWidth = 1)
{
    renderer_entry_line *Entry = PushRenderElement(Frame, renderer_entry_line);
    if(Entry)
    {
        Entry->P1 = P1;
        Entry->P2 = P2;
        Entry->Color = Color;
        Entry->LineWidth = LineWidth;
    }
}

void
PushWater(renderer_frame *Frame, v3 Offset, v2 Dim)
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    
    renderer_entry_water *Entry = 0;
    u32 Size = sizeof(renderer_entry_water);
    if((Frame->WaterPushBufferSize + Size) < Frame->WaterMaxPushBufferSize)
    {
        Entry = (renderer_entry_water *)(Frame->WaterPushBufferBase + Frame->WaterPushBufferSize);
        Frame->WaterPushBufferSize += Size;
    }
    else
    {
        InvalidCodePath;
    }
    if(Entry)
    {
        Entry->P = P;
        Entry->Dim = Dim;
    }
}

// TODO(ezexff): Test
#define PushOrthoRenderElement(PushBuffer, type, SortKey) (type *)PushRenderElement_(PushBuffer, sizeof(type), RendererOrthoEntryType_##type, SortKey)
void *
PushRenderElement_(renderer_push_buffer *PushBuffer, u32 Size, renderer_ortho_entry_type Type, r32 SortKey)
{
    void *Result = 0;
    
    Size += sizeof(renderer_entry_header);
    
    if((PushBuffer->Size + Size) < sizeof(PushBuffer->Memory))
    {
        renderer_entry_header *Header = (renderer_entry_header *)(PushBuffer->Base + PushBuffer->Size);
        Header->OrthoType = Type;
        Result = (u8 *)Header + sizeof(*Header);
        
        if(PushBuffer->ElementCount < PushBuffer->ElementCountMax)
        {
            PushBuffer->SortEntryArray[PushBuffer->ElementCount].Offset = PushBuffer->Size;
            PushBuffer->SortEntryArray[PushBuffer->ElementCount].SortKey = SortKey;
            PushBuffer->ElementCount++;
        }
        else
        {
            InvalidCodePath;
        }
        
        PushBuffer->Size += Size;
    }
    else
    {
        InvalidCodePath;
    }
    
    return(Result);
}

void
PushRectOnScreen(renderer_push_buffer *PushBuffer, v2 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), r32 SortKey = 0.0f)
{
    v2 P = Offset;
    
    renderer_ortho_entry_rect *Entry = PushOrthoRenderElement(PushBuffer, renderer_ortho_entry_rect, SortKey);
    if(Entry)
    {
        Entry->Color = Color;
        Entry->P = P;
        Entry->Dim = Dim;
    }
}

void
PushRectOutlineOnScreen(renderer_push_buffer *PushBuffer, rectangle2 Rect, r32 LineWidth, v4 Color = V4(1, 1, 1, 1), r32 SortKey = 0.0f)
{
    renderer_ortho_entry_rect_outline *Entry = PushOrthoRenderElement(PushBuffer, renderer_ortho_entry_rect_outline, SortKey);
    if(Entry)
    {
        Entry->Rect = Rect;
        Entry->LineWidth = LineWidth;
        Entry->Color = Color;
    }
}

void
PushBitmapOnScreen(renderer_push_buffer *PushBuffer, game_assets *Assets, bitmap_id ID, v2 Offset, v2 Dim,
                   r32 SortKey = 0.0f, r32 Repeat = 1)
{
    loaded_bitmap *Bitmap = GetBitmap(Assets, ID, true);
    if(Bitmap)
    {
        v2 P = Offset;
        
        renderer_ortho_entry_bitmap *Entry = PushOrthoRenderElement(PushBuffer, renderer_ortho_entry_bitmap, SortKey);
        if(Entry)
        {
            Entry->Bitmap = Bitmap;
            Entry->P = P;
            Entry->Dim = Dim;
            Entry->Repeat = Repeat;
        }
    }
    else
    {
        LoadBitmap(Assets, ID, true);
        //++Frame->MissingResourceCount;
    }
}