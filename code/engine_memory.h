struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;

    int32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

inline void InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

inline memory_index GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
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

inline memory_index GetArenaSizeRemaining(memory_arena *Arena, memory_index Alignment = 4)
{
    memory_index Result = Arena->Size - (Arena->Used + GetAlignmentOffset(Arena, Alignment));

    return (Result);
}

#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ##__VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count) * sizeof(type), ##__VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ##__VA_ARGS__)
inline void *PushSize_(memory_arena *Arena, memory_index SizeInit, memory_index Alignment = 4)
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

#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ##__VA_ARGS__))
inline void * //
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--)
    {
        *Dest++ = *Source++;
    }

    return (DestInit);
}

inline temporary_memory BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return (Result);
}

inline void EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size, memory_index Alignment = 16)
{
    Result->Size = Size;
    Result->Base = (uint8 *)PushSize_(Arena, Size, Alignment);
    Result->Used = 0;
    Result->TempCount = 0;
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
inline void ZeroSize(memory_index Size, void *Ptr)
{
    // TODO(casey): Check this guy for performance
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

/*struct memory_arena
{
    memory_index Size;
    uint8 *Base;
    memory_index Used;

    int32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

internal void //
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (uint8 *)Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count) * sizeof(type))
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ##__VA_ARGS__))
#define PushSize(Arena, Size) PushSize_(Arena, Size)

// #define ConstZ(Z) {sizeof(Z) - 1, (u8 *)(Z)}

#define CopyArray(Count, Source, Dest) Copy((Count) * sizeof(*(Source)), (Source), (Dest))
inline void * //
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--)
    {
        *Dest++ = *Source++;
    }

    return (DestInit);
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
//#define ZeroArray(Count, Pointer) ZeroSize(Count * sizeof((Pointer)[0]), Pointer)
inline void //
ZeroSize(memory_index Size, void *Ptr)
{
    uint8 *Byte = (uint8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

inline void * //
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return (Result);
}

inline temporary_memory //
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return (Result);
}

inline void //
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used);
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void //
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}*/

//
// NOTE(me): String
//
struct buffer
{
    umm Count;
    u8 *Data;
};
typedef buffer string;

inline u32 //
StringLength(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }

    return (Count);
}

inline char * //
PushStringZ(memory_arena *Arena, char *Source)
{
    u32 Size = 1;
    for(char *At = Source; *At; ++At)
    {
        ++Size;
    }

    char *Dest = (char *)PushSize_(Arena, Size);
    for(u32 CharIndex = 0; CharIndex < Size; ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }

    return (Dest);
}

internal string //
PushString(memory_arena *Arena, char *Source)
{
    string Result;
    Result.Count = StringLength(Source) + 1;
    Source += '\0';
    Result.Data = (u8 *)PushCopy(Arena, Result.Count, Source);

    return (Result);
}

inline b32 //
operator==(string A, string B)
{
    b32 Result = false;

    if(A.Count == B.Count)
    {
        for(u32 i = 0; i < A.Count; i++)
        {
            if(A.Data[i] != B.Data[i])
            {
                return (Result);
            }
        }
        Result = true;
    }

    return (Result);
}

inline b32 //
operator==(string A, char *B)
{
    b32 Result = false;

    u64 ACount = A.Count - 1; // без символа конца строки
    u32 BCount = StringLength(B);
    if(ACount == BCount)
    {
        for(u32 i = 0; i < ACount; i++)
        {
            if(A.Data[i] != B[i])
            {
                return (Result);
            }
        }
        Result = true;
    }

    return (Result);
}