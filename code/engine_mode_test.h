enum ui_node_style_flags
{
    UI_NodeStyleFlag_Clickable       = (1 << 0),
    UI_NodeStyleFlag_ViewScroll      = (1 << 1),
    UI_NodeStyleFlag_DrawText        = (1 << 2),
    UI_NodeStyleFlag_DrawBorder      = (1 << 3),
    UI_NodeStyleFlag_DrawBackground  = (1 << 4),
    UI_NodeStyleFlag_DrawDropShadow  = (1 << 5),
    UI_NodeStyleFlag_Clip            = (1 << 6),
    UI_NodeStyleFlag_HotAnimation    = (1 << 7),
    UI_NodeStyleFlag_ActiveAnimation = (1 << 8),
};

enum ui_node_state_flags
{
    UI_NodeStateFlag_Clicked       = (1 << 0),
    UI_NodeStateFlag_DoubleClicked = (1 << 1),
    UI_NodeStateFlag_RightClicked  = (1 << 2),
    UI_NodeStateFlag_Pressed       = (1 << 3),
    UI_NodeStateFlag_Released      = (1 << 4),
    UI_NodeStateFlag_Dragging      = (1 << 5),
    UI_NodeStateFlag_Hovering      = (1 << 6),
};

/* 
#define ui_pressed(s)        !!((s).f&UI_SignalFlag_Pressed)
#define ui_clicked(s)        !!((s).f&UI_SignalFlag_Clicked)
#define ui_released(s)       !!((s).f&UI_SignalFlag_Released)
#define ui_double_clicked(s) !!((s).f&UI_SignalFlag_DoubleClicked)
#define ui_triple_clicked(s) !!((s).f&UI_SignalFlag_TripleClicked)
#define ui_middle_clicked(s) !!((s).f&UI_SignalFlag_MiddleClicked)
#define ui_right_clicked(s)  !!((s).f&UI_SignalFlag_RightClicked)
#define ui_dragging(s)       !!((s).f&UI_SignalFlag_Dragging)
#define ui_hovering(s)       !!((s).f&UI_SignalFlag_Hovering)
#define ui_mouse_over(s)     !!((s).f&UI_SignalFlag_MouseOver)
#define ui_committed(s)      !!((s).f&UI_SignalFlag_Commit)
 */
#define UI_IsClicked(Node) (Node & UI_NodeStateFlag_Clicked)
#define UI_IsPressed(Node) (Node & UI_NodeStateFlag_Pressed)

struct ui_node_style
{
    u32 Flags;
    v4 BackgroundColor;
};

enum ui_size_type
{
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,      // size is computed via a preferred pixel value
    UI_SizeKind_TextContent, // size is computed via the dimensions of box's rendered string
    UI_SizeKind_ParentPct,   // size is computed via a well-determined parent or grandparent size
    UI_SizeKind_ChildrenSum, // size is computed via summing well-determined sizes of children
};

struct ui_size
{
    ui_size_type Type;
    r32 Value;
    r32 Strictness;
};

enum axis2
{
    Axis2_Invalid = -1,
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT,
};

struct ui_node
{
    // NOTE(ezexff): Child per-frame links
    ui_node *First;
    ui_node *Last;
    ui_node *Next;
    ui_node *Prev;
    ui_node *Parent;
    u32 ChildCount;
    
    // persistent links
    ui_node *HashNext;
    ui_node *HashPrev;
    
    // key+generation info
    /* 
        UI_Key key;
        U64 last_frame_touched_index;
     */
    
    // per-frame info provided by builders
    u32 StateFlags;
    //u32 StateFlags;
    char *String;
    
    ui_node_style Style;
    
    // NOTE(ezexff): Size
    //v2 FixedP;
    //v2 FixedSize;
    ui_size PrefSize[Axis2_COUNT];
    axis2 ChildLayoutAxis;
    //r32 InnerSumY;
    rectangle2 Rect;
    //rectangle2 FixedSizeRect;
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
    
    // persistent data
    r32 hot_t;
    r32 active_t;
};

enum ui_style_template_name
{
    UI_StyleTemplate_Default,
    UI_StyleTemplate_Button,
    
    UI_StyleTemplate_Count,
};

struct ui_style_template
{
    v4 BackgroundColor;
    v4 HoveringColor;
    v4 ClickedColor;
    v4 PressedColor;
    
    v2 FixedSize;
};

struct ui_state
{
    ui_node *Root;
    
    u32 SelectedTemplateIndex;
    ui_style_template StyleTemplateArray[UI_StyleTemplate_Count];
    /* 
        ui_node *TooltipRoot;
        ui_node *ContextMenuRoot;
     */
    
    /* 
        ui_node *HotWidget;
        ui_node *LastWidget;
        ui_node UI_RectSetinel;
        
         */
    
    temporary_memory FrameMemory;
    
    // NOTE(ezexff): Pointers to external services
    memory_arena *TranArena;
    renderer_frame *Frame;
    game_input *Input;
    game_assets *Assets;
};

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

ui_state *UI_State;