struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
    
    s32 TempCount;
};

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

//~
// NOTE(ezexff): Push in memory
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count) * sizeof(type), ##__VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ##__VA_ARGS__)

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
    memory_index AlignmentOffset = 0;
    
    memory_index ResultPointer = (memory_index)Arena->Base + Arena->Used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }
    
    return (AlignmentOffset);
}

inline void *
PushSize_(memory_arena *Arena, memory_index SizeInit, memory_index Alignment = 4)
{
    memory_index Size = SizeInit;
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Alignment);
    Size += AlignmentOffset;
    
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;
    
    Assert(Size >= SizeInit);
    
    return (Result);
}

//~
// NOTE(ezexff): Temporary arena
/*struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    
    ++Arena->TempCount;
    
    return (Result);
}

inline void 
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void 
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}*/