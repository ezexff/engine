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
    
    UI_NodeFlag_Floating        = (1 << 9),  // NOTE(ezexff): without autolayout
    
    UI_NodeFlag_Clicked         = (1 << 10), // TODO(ezexff): node was previously pressed & user released, in bounds???
    UI_NodeFlag_DoubleClicked   = (1 << 11),
    UI_NodeFlag_RightClicked    = (1 << 12),
    UI_NodeFlag_Pressed         = (1 << 13), // TODO(ezexff): mouse press -> node was pressed while hovering
    UI_NodeFlag_Released        = (1 << 14), // TODO(ezexff): released -> node was previously pressed & user released, in or out of bounds
    UI_NodeFlag_Dragging        = (1 << 15), // TODO(ezexff): node was previously pressed, user is still holding button
    UI_NodeFlag_Hovering        = (1 << 16), 
    
    UI_NodeFlag_Expanded        = (1 << 17), // NOTE(ezexff): show window body
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
    // NOTE(ezexff): persistent vars
    u32 Key;
    ui_node *Cache;
    ui_node *CacheNext;
    ui_node *CachePrev;
    v2 P;
    v2 Size;
    u64 LastFrameTouchedIndex;
    u32 StyleTemplateIndex;
    
    // NOTE(ezexff): per-build vars
    ui_node *First;
    ui_node *Last;
    ui_node *Next;
    ui_node *Prev;
    ui_node *Parent;
    //ui_node *WidgetRoot;
    u32 ChildCount;
    
    char *String;
    u32 Flags;
    v4 BackgroundColor;
    
    axis2 LayoutAxis; // align elements by axis
    
    // per-frame info provided by builders
    
    
    // TODO(ezexff): new positioning
    /* 
        v2 P;
        v2 Dim;
     */
    v2 StartTextOffset;
    v2 MaxChildNodeDim;
    //ui_size Size[Axis2_Count];
    rectangle2 Rect; // calculated rect in screen space coordiantes
    v2 ViewP; // TODO(ezexff): temp for scrollbars in windows only
    v2 PressMouseP;
    
    
    // NOTE(ezexff): used in autolayout algorithm (this affect on child elements)
    //r32 Spacing; // distance between child nodes in axis
    //r32 Padding;
    
    u32 InteractionType;
    
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
    //UI_StyleTemplate_Window,
    UI_StyleTemplate_Window1,
    UI_StyleTemplate_Window2,
    UI_StyleTemplate_Window3,
    UI_StyleTemplate_WindowTitle,
    UI_StyleTemplate_WindowTitleEmptySpace,
    UI_StyleTemplate_WindowTitleExitButton,
    UI_StyleTemplate_WindowBody,
    UI_StyleTemplate_WindowResizeButton,
    UI_StyleTemplate_WindowScrollBar,
    
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

struct ui_widget_link
{
    ui_node *First;
    ui_node *Last;
};

struct ui_state
{
    //ui_node *Root;
    ui_widget_link WindowArray;
    u32 WindowCount;
    
    u64 FrameCount;
    u64 NodeCount;
    
    //u32 SelectedTemplateIndex;
    ui_style_template StyleTemplateArray[UI_StyleTemplate_Count];
    
    ui_node *CacheFirst;
    ui_node *CacheLast;
    
    /* 
        ui_node *OpenWindow;
        ui_node *OpenWindowBody;
     */
    
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

enum ui_interaction_type
{
    UI_Interaction_None,
    
    UI_Interaction_NOP,
    
    UI_Interaction_AutoModifyVariable,
    
    UI_Interaction_ToggleValue,
    UI_Interaction_DragValue,
    UI_Interaction_TearValue,
    
    UI_Interaction_Resize,
    UI_Interaction_Move,
    
    UI_Interaction_Select,
    
    UI_Interaction_ToggleExpansion,
    
    UI_Interaction_SetUInt32,
    UI_Interaction_SetPointer,
};

/* 
enum ui_widget_type
{
    UI_WidgetType_None;
    
    UI_WidgetType_Window;
    UI_WidgetType_None;
};
 */

ui_state *UI_State;