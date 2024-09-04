struct debug_frame
{
    // IMPORTANT(casey): This actually gets freed as a set in FreeFrame!
    
    union
    {
        debug_frame *Next;
        debug_frame *NextFree;
    };
    
    u64 BeginClock;
    u64 EndClock;
    r32 WallSecondsElapsed;
    
    r32 FrameBarScale;
    
    u32 FrameIndex;
};

struct debug_block
{
    debug_event *OpeningEvent;
    debug_block *Next;
    debug_block *Prev;
};


struct debug_state
{
    b32 Initialized;
    
    platform_work_queue *HighPriorityQueue;
    
    
    memory_arena DebugArena;
    temporary_memory DebugMemory;
    
    debug_frame *CollationFrame;
    
    u32 FrameCount;
    
    //debug_block *ParentDebugBlock;
    debug_block *CurrentDebugBlock;
    
    /*debug_tree TreeSentinel;
    
    debug_record *ScopeToRecord;
    
    // NOTE(casey): Collation
    memory_arena CollateArena;
    temporary_memory CollateTemp;
    
    u32 CollationArrayIndex;
    debug_frame *CollationFrame;
    u32 FrameBarLaneCount;
    r32 FrameBarScale;
    b32 Paused;
    
    debug_frame *Frames;
    debug_thread *FirstThread;
    open_debug_block *FirstFreeBlock;
 */
};

struct debug_stored_event
{
    union
    {
        debug_stored_event *Next;
        debug_stored_event *NextFree;
    };
    
    u32 FrameIndex;
    debug_event Event;
};

struct debug_element
{
    char *GUID;
    debug_element *NextInHash;
    
    debug_stored_event *OldestEvent;
    debug_stored_event *MostRecentEvent;
};

struct open_debug_block
{
    union
    {
        open_debug_block *Parent;
        open_debug_block *NextFree;
    };
    
    u32 StartingFrameIndex;
    debug_event *OpeningEvent;
    //debug_element *Element;
    
    // NOTE(casey): Only for data blocks?  Probably!
    //debug_variable_group *Group;    
};