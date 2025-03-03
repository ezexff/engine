// NOTE(ezexff): 
/*
--- String after # hashing but not processing and not drawing
*/

enum ui_node_flags
{
    UI_NodeFlag_Clickable       = (1 << 0),
    UI_NodeFlag_ViewScroll      = (1 << 1),
    UI_NodeFlag_DrawText        = (1 << 2),
    UI_NodeFlag_DrawBorder      = (1 << 3),
    UI_NodeFlag_DrawBackground  = (1 << 4),
    UI_NodeFlag_DrawDropShadow  = (1 << 5),
    UI_NodeFlag_Clip            = (1 << 6),
    UI_NodeFlag_HotAnimation    = (1 << 7),
    UI_NodeFlag_ActiveAnimation = (1 << 8),
    
    UI_NodeFlag_Floating        = (1 << 9),
    
    UI_NodeFlag_Clicked         = (1 << 10),
    UI_NodeFlag_DoubleClicked   = (1 << 11),
    UI_NodeFlag_RightClicked    = (1 << 12),
    UI_NodeFlag_Pressed         = (1 << 13),
    UI_NodeFlag_Released        = (1 << 14),
    UI_NodeFlag_Dragging        = (1 << 15),
    UI_NodeFlag_Hovering        = (1 << 16),
    
    UI_NodeFlag_Expanded        = (1 << 17),
};

#define UI_IsPressed(Node) (Node & UI_NodeFlag_Pressed)
#define UI_IsDragging(Node) (Node & UI_NodeFlag_Dragging)
#define UI_IsExpanded(Node) (Node & UI_NodeFlag_Expanded)

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
    //U64 last_frame_touched_index;
    
    // NOTE(ezexff): child per-frame links
    ui_node *First;
    ui_node *Last;
    ui_node *Next;
    ui_node *Prev;
    ui_node *Parent;
    u32 ChildCount;
    
    char *String;
    u32 Flags;
    v4 BackgroundColor;
    
    // per-frame info provided by builders
    rectangle2 Rect; // calculated rect in screen space coordiantes
    v2 OffsetP;
    v2 OffsetSize;
    
    // NOTE(ezexff): used in autolayout algorithm (this affect on child elements)
    axis2 LayoutAxis; // align elements by axis
    ui_size Size[Axis2_Count];
    r32 Spacing; // distance between child nodes in axis
    r32 Padding;
    
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
    r32 Padding;
    
    v2 OffsetP;
};

struct ui_state
{
    ui_node *Root;
    
    u32 SelectedTemplateIndex;
    ui_style_template StyleTemplateArray[UI_StyleTemplate_Count];
    
    ui_node *CacheFirst;
    ui_node *CacheLast;
    
    ui_node *OpenWindow;
    ui_node *OpenWindowBody;
    
    ui_node *Interaction;
    ui_node *HotInteraction;
    ui_node *NextHotInteraction;
    
    game_button_state PressKeyHistory[PlatformMouseButton_Count][2];
    v2 MousePHistory[2];
    //v3s MouseDeltaHistory[PlatformMouseButton_Count][3];
    
    font_id FontID;
    temporary_memory FrameMemory;
    
    // NOTE(ezexff): pointers to external services
    memory_arena *ConstArena;
    memory_arena *TranArena;
    renderer_frame *Frame;
    game_input *Input;
    game_assets *Assets;
};

ui_state *UI_State;