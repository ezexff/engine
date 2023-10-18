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
inline void *Copy(memory_index Size, void *SourceInit, void *DestInit)
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
#define ZeroArray(Count, Pointer) ZeroSize(Count * sizeof((Pointer)[0]), Pointer)
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

//
// NOTE(me): String
//
struct buffer
{
    umm Count;
    u8 *Data;
};
typedef buffer string;

inline u32 StringLength(char *String)
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

inline char *PushStringZ(memory_arena *Arena, char *Source)
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

internal string PushString(memory_arena *Arena, char *Source)
{
    string Result;
    Result.Count = StringLength(Source) + 1;
    Source += '\0';
    Result.Data = (u8 *)PushCopy(Arena, Result.Count, Source);

    return (Result);
}

inline b32 operator==(string A, string B)
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

inline b32 operator==(string A, char *B)
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