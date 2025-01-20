struct mode_test
{
    b32 IsInitialized;
    
    /* 
        v4 ClearColor;
        
        opengl_program FrameProgram;
        opengl_shader FrameVert;
        opengl_shader FrameFrag;
     */
};

enum ui_rect_flags
{
    UI_WidgetFlag_Clickable       = (1 << 0),
    UI_WidgetFlag_ViewScroll      = (1 << 1),
    UI_WidgetFlag_DrawText        = (1 << 2),
    UI_WidgetFlag_DrawBorder      = (1 << 3),
    UI_WidgetFlag_DrawBackground  = (1 << 4),
    UI_WidgetFlag_DrawDropShadow  = (1 << 5),
    UI_WidgetFlag_Clip            = (1 << 6),
    UI_WidgetFlag_HotAnimation    = (1 << 7),
    UI_WidgetFlag_ActiveAnimation = (1 << 8),
};

struct ui_rect
{
    // per-build links/data
    ui_rect *First;
    ui_rect *Last;
    ui_rect *Next;
    ui_rect *Prev;
    ui_rect *Parent;
    u32 ChildCount;
    
    // persistent links
    ui_rect *HashNext;
    ui_rect *HashPrev;
    
    // key+generation info
    /* 
        UI_Key key;
        U64 last_frame_touched_index;
     */
    
    // per-frame info provided by builders
    u32 Flags;
    char *String;
    
    // TODO(ezexff): ui_palette
    v4 BackgroundColor;
    /* 
        string string;
        UI_Size semantic_size[Axis2_COUNT];
     */
    
    // computed every frame
    /* 
        F32 computed_rel_position[Axis2_COUNT];
        F32 computed_size[Axis2_COUNT];
        Rng2F32 rect;
         */
    v2 RelPos;
    rectangle2 Rect;
    
    // persistent data
    r32 hot_t;
    r32 active_t;
};

struct ui_comm
{
    ui_rect *Element;
    v2 MouseP;
    v2 DragDelta;
    b8 Clicked;
    b8 DoubleClicked;
    b8 RightClicked;
    b8 Pressed;
    b8 Released;
    b8 Dragging;
    b8 Hovering;
};

struct ui_layout
{
    v2 Pos;
    v2 Dim;
};

inline b32 IsSet(ui_rect *Widget, u32 Flag)
{
    b32 Result = Widget->Flags & Flag;
    return(Result);
}

struct ui_state
{
    ui_rect *Root;
    ui_rect *TooltipRoot;
    ui_rect *CtxMenuRoot;
    // TODO(ezexff): 
};