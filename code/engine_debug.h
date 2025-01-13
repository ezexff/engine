#define DEBUG_FRAME_COUNT 256

struct debug_stored_block
{
    debug_stored_block *FirstChild;
    debug_stored_block *NextChild;
    debug_stored_block *LastChild;
    
    //debug_stored_block *Parent;
    
    //debug_stored_block *NextChild;
    
    u64 Clock;
    char *GUID;
    u16 ThreadID;
    u16 CoreIndex;
    u8 Type;
    
    u32 FrameIndex;
};

struct debug_frame
{
    debug_stored_block RootStoredBlock;
    /* 
        debug_stored_block *FirstStoredBlock;
        debug_stored_block *LastStoredBlock;
     */
    r32 ClockInMs;
    r64 ClockInCycles;
};

struct debug_statistic
{
    r64 Min;
    r64 Max;
    r64 Sum;
    r64 Avg;
    u32 Count;
};

struct debug_state
{
    b32 Initialized;
    b32 Paused;
    
    memory_arena DebugArena;
    //memory_arena EmptyDebugArena;
    memory_arena PerFrameArena;
    
    temporary_memory FramesTempMemory;
    
    u32 TotalFrameCount;
    
    u32 ViewingFrameOrdinal;
    u32 MostRecentFrameOrdinal;
    /*u32 CollationFrameOrdinal;
    u32 OldestFrameOrdinal;
 */
    
    
    u32 StoredBlockCount;
    //b32 PrevBlockUnclosed;
    
    
    u32 DebugFrameIndex;
    debug_frame DebugFrameArray[DEBUG_FRAME_COUNT];
    
    //u32 BlockStatIndex;
    debug_statistic BlockStatArray[64];
    //debug_stored_block *PrevBeginEvent;
    
    //debug_frame Frames[DEBUG_FRAME_COUNT];
    
    //debug_stored_frame *StoredFrameList;
    /* 
        
        debug_stored_block *OpenedStoredBlock;
        
        u32 OpenedBlockCount; // NOTE(ezexff): Blocks that have Start and End
        u32 BlockCount;
     */
    //debug_stored_block *UnclosedBlockList[255]; // NOTE(ezexff): Begin event without End event
    
    //debug_stored_block *StoredBlockRoot;
    //debug_stored_block *CurrentBlockInHierarchy;
    u32 OpenBlockIndex;
    debug_stored_block *OpenBlockArray[10];
    
    u32 TmpBlockCount;
};

inline b32
DebugIDsAreEqual(debug_id A, debug_id B)
{
    b32 Result = ((A.Value[0] == B.Value[0]) &&
                  (A.Value[1] == B.Value[1]));
    
    return(Result);
}
enum debug_element_add_op
{
    DebugElement_AddToGroup = 0x1,
    DebugElement_CreateHierarchy = 0x2,
};

struct debug_parsed_name
{
    u32 HashValue;
    u32 FileNameCount;
    u32 NameStartsAt;
    u32 LineNumber;
    
    u32 NameLength;
    char *Name;
};
inline debug_parsed_name
DebugParseName(char *GUID)
{
    debug_parsed_name Result = {};
    
    u32 PipeCount = 0;
    char *Scan = GUID;
    for(;
        *Scan;
        ++Scan)
    {
        if(*Scan == '|')
        {
            if(PipeCount == 0)
            {
                Result.FileNameCount = (u32)(Scan - GUID);
                Result.LineNumber = atoi(Scan + 1);
            }
            else if(PipeCount == 1)
            {
                
            }
            else
            {
                Result.NameStartsAt = (u32)(Scan - GUID + 1);
            }
            
            ++PipeCount;
        }
        
        // TODO(casey): Better hash function
        Result.HashValue = 65599*Result.HashValue + *Scan;
    }
    
    Result.NameLength = (u32)(Scan - GUID) - Result.NameStartsAt;
    Result.Name = GUID + Result.NameStartsAt;
    
    return(Result);
}