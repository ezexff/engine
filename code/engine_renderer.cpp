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
PushClear(renderer_frame *Frame, v4 Color)
{
    renderer_entry_clear *Entry = PushRenderElement(Frame, renderer_entry_clear);
    if(Entry)
    {
        Entry->Color = Color;
    }
}

void
PushRectOnGround(renderer_frame *Frame, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    //P += Group->Transform.OffsetP;
    
    renderer_entry_rect_on_ground *Entry = PushRenderElement(Frame, renderer_entry_rect_on_ground);
    Entry->Color = Color;
    Entry->P = P.xy;
    Entry->Dim = Dim;
}

void
PushRectOutlineOnGround(renderer_frame *Frame, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), r32 LineWidth = 1)
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    //P += Group->Transform.OffsetP;
    
    renderer_entry_rect_outline_on_ground *Entry = PushRenderElement(Frame, renderer_entry_rect_outline_on_ground);
    Entry->Color = Color;
    Entry->P = P.xy;
    Entry->Dim = Dim;
    Entry->LineWidth = LineWidth;
}
