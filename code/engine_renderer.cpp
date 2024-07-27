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
Clear(renderer_frame *Frame, v4 Color)
{
    renderer_entry_clear *Entry = PushRenderElement(Frame, renderer_entry_clear);
    if(Entry)
    {
        Entry->Color = Color;
    }
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
        ++Frame->MissingResourceCount;
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
        ++Frame->MissingResourceCount;
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
PushTerrainChunk(renderer_frame *Frame, u32 PositionsCount, v3 *Positions, u32 IndicesCount, u32 *Indices)
{
    renderer_entry_terrain_chunk *Entry = PushRenderElement(Frame, renderer_entry_terrain_chunk);
    if(Entry)
    {
        Entry->PositionsCount = PositionsCount;
        Entry->Positions = Positions;
        Entry->IndicesCount = IndicesCount;
        Entry->Indices = Indices;
    }
}