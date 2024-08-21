struct debug_counter_state
{
    char *FileName;
    char *BlockName;
    
    u32 LineNumber;
};

enum debug_variable_type
{
    DebugVariableType_Bool32,
    DebugVariableType_Int32,
    DebugVariableType_UInt32,
    DebugVariableType_Real32,
    DebugVariableType_V2,
    DebugVariableType_V3,
    DebugVariableType_V4,
    
    DebugVariableType_CounterThreadList,
    //    DebugVariableType_CounterFunctionList,
    
    DebugVariableType_BitmapDisplay,
    
    DebugVariableType_VarArray,
};

struct debug_profile_settings
{
    int Placeholder;
};

struct debug_bitmap_display
{
    bitmap_id ID;
};

struct debug_variable;

struct debug_variable_array
{
    u32 Count;
    debug_variable *Vars;
};

struct debug_variable
{
    debug_variable_type Type;
    char *Name;
    
    union
    {
        b32 Bool32;
        s32 Int32;
        u32 UInt32;
        r32 Real32;
        v2 Vector2;
        v3 Vector3;
        v4 Vector4;
        debug_profile_settings Profile;
        debug_bitmap_display BitmapDisplay;
        debug_variable_array VarArray;
    };
};

struct debug_tree
{
    v2 UIP;
    debug_variable *Group;
    
    debug_tree *Next;
    debug_tree *Prev;
};

struct debug_frame_region
{
    debug_record *Record;
    u64 CycleCount;
    u16 LaneIndex;
    u16 ColorIndex;
    r32 MinT;
    r32 MaxT;
};

#define MAX_REGIONS_PER_FRAME 2*4096
struct debug_frame
{
    u64 BeginClock;
    u64 EndClock;
    r32 WallSecondsElapsed;
    
    u32 RegionCount;
    debug_frame_region *Regions;
};

struct open_debug_block
{
    u32 StartingFrameIndex;
    debug_record *Source;
    debug_event *OpeningEvent;
    open_debug_block *Parent;
    
    open_debug_block *NextFree;
};

struct debug_thread
{
    u32 ID;
    u32 LaneIndex;
    open_debug_block *FirstOpenBlock;
    debug_thread *Next;
};

struct debug_state
{
    b32 Initialized;
    
    platform_work_queue *HighPriorityQueue;
    
    memory_arena DebugArena;
    
    debug_tree TreeSentinel;
    
    debug_record *ScopeToRecord;
    
    // NOTE(casey): Collation
    memory_arena CollateArena;
    temporary_memory CollateTemp;
    
    u32 CollationArrayIndex;
    debug_frame *CollationFrame;
    u32 FrameBarLaneCount;
    u32 FrameCount;
    r32 FrameBarScale;
    b32 Paused;
    
    debug_frame *Frames;
    debug_thread *FirstThread;
    open_debug_block *FirstFreeBlock;
};