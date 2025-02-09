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
//#define UI_IsClicked(Node) (Node & UI_NodeStateFlag_Clicked)
#define UI_IsPressed(Node) (Node & UI_NodeStateFlag_Pressed)
#define UI_IsDragging(Node) (Node & UI_NodeStateFlag_Dragging)

struct ui_node_style
{
    u32 Flags;
    v4 BackgroundColor;
};

enum ui_size_type
{
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_ParentPercent,
    UI_SizeKind_ChildrenSum,
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
    
    Axis2_Count,
};

struct ui_node
{
    // NOTE(ezexff): persistent links
    ui_node *CacheNext;
    ui_node *CachePrev;
    u32 Key;
    
    // NOTE(ezexff): Child per-frame links
    ui_node *First;
    ui_node *Last;
    ui_node *Next;
    ui_node *Prev;
    ui_node *Parent;
    u32 ChildCount;
    
    // key+generation info
    /* 
        UI_Key key;
        U64 last_frame_touched_index;
     */
    
    // per-frame info provided by builders
    u32 StateFlags;
    char *String;
    ui_node_style Style;
    rectangle2 Rect; // calculated rect in screen space coordiantes
    
    // NOTE(ezexff): used in autolayout algorithm (this affect on child elements)
    axis2 LayoutAxis; // align elements by axis
    ui_size Size[Axis2_Count];
    r32 Spacing; // distance between nodes in axis
    v4 Padding;
    
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
    UI_StyleTemplate_Label,
    UI_StyleTemplate_Checkbox,
    UI_StyleTemplate_CheckboxMark,
    UI_StyleTemplate_Window,
    UI_StyleTemplate_WindowTitle,
    UI_StyleTemplate_WindowTitleEmptySpace,
    UI_StyleTemplate_WindowTitleExitButton,
    UI_StyleTemplate_WindowBody,
    UI_StyleTemplate_WindowResizeButton,
    
    UI_StyleTemplate_Count,
};

struct ui_style_template
{
    v4 BackgroundColor;
    v4 HoveringColor;
    v4 ClickedColor;
    v4 PressedColor;
    
    ui_size Size[Axis2_Count];
    v4 Padding;
};

struct ui_state
{
    ui_node *Root;
    
    u32 SelectedTemplateIndex;
    ui_style_template StyleTemplateArray[UI_StyleTemplate_Count];
    
    font_id FontID;
    
    // cache
    ui_node *CacheFirst;
    ui_node *CacheLast;
    /* 
        u32 CacheIndex;
        u32 CacheTableSize;
        ui_node *CacheTable;
     */
    /* 
        ui_node *CacheTableFirst;
        ui_node *CacheTableLast;
     */
    
    
    /* 
        ui_node *TooltipRoot;
        ui_node *ContextMenuRoot;
     */
    
    /* 
        ui_node *HotWidget;
        ui_node *LastWidget;
        ui_node UI_RectSetinel;
         */
    
    game_button_state PressKeyHistory[PlatformMouseButton_Count][2];
    v2s MousePHistory[2];
    //v3s MouseDeltaHistory[PlatformMouseButton_Count][3];
    
    temporary_memory FrameMemory;
    
    // NOTE(ezexff): pointers to external services
    memory_arena *ConstArena;
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