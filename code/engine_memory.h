struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
    
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    memory_index Used;
};

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize(Count*sizeof((Pointer)[0]), Pointer)
inline void
ZeroSize(memory_index Size, void *Ptr)
{
    // TODO(casey): Check this guy for performance
    u8 *Byte = (u8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

inline void
InitializeArena(memory_arena *Arena, memory_index Size, void *Base)
{
    Arena->Size = Size;
    Arena->Base = (u8 *)Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

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
    
    return(AlignmentOffset);
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = 0x1,
};
struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params
DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline arena_push_params
AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
Align(u32 Alignment, b32 Clear)
{
    arena_push_params Params = DefaultArenaParams();
    if(Clear)
    {
        Params.Flags |= ArenaFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~ArenaFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return(Params);
}

inline memory_index
GetArenaSizeRemaining(memory_arena *Arena, arena_push_params Params = DefaultArenaParams())
{
    memory_index Result = Arena->Size - (Arena->Used + GetAlignmentOffset(Arena, Params.Alignment));
    
    return(Result);
}

inline void *
Copy(memory_index Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}
    
    return(DestInit);
}

// TODO(casey): Optional "clear" parameter!!!!
#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ## __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ## __VA_ARGS__))
inline memory_index
GetEffectiveSizeFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = SizeInit;
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;
    
    return(Size);
}

inline b32
ArenaHasRoomFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    b32 Result = ((Arena->Used + Size) <= Arena->Size);
    return(Result);
}

inline void *
PushSize_(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    
    Assert((Arena->Used + Size) <= Arena->Size);
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;
    
    Assert(Size >= SizeInit);
    
    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }
    
    return(Result);
}

// NOTE(casey): This is generally not for production use, this is probably
// only really something we need during testing, but who knows
inline char *
PushString(memory_arena *Arena, char *Source)
{
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }
    
    char *Dest = (char *)PushSize_(Arena, Size, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    
    return(Dest);
}

inline char *
PushAndNullTerminate(memory_arena *Arena, u32 Length, char *Source)
{
    char *Dest = (char *)PushSize_(Arena, Length + 1, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Length;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    Dest[Length] = 0;
    
    return(Dest);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    
    Result.Arena = Arena;
    Result.Used = Arena->Used;
    
    ++Arena->TempCount;
    
    return(Result);
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
Clear(memory_arena *Arena)
{
    InitializeArena(Arena, Arena->Size, Arena->Base);
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, memory_index Size, arena_push_params Params = DefaultArenaParams())
{
    Result->Size = Size;
    Result->Base = (u8 *)PushSize_(Arena, Size, Params);
    Result->Used = 0;
    Result->TempCount = 0;
}


#include <stdarg.h>

//~ TODO(ezexff): test functions for string
internal char *
Concat(memory_arena *Arena, char *A, char *B)
{
    char *Result;
    if(A && B)
    {
        u64 ALen = StringLength(A);
        u64 BLen = StringLength(B);
        u64 Size = ALen + BLen + 1;
        //Result = (char *)PushSize_(Arena, Size, NoClear());
        Result = (char *)PushSize_(Arena, Size, NoClear());
        u32 CharIndex = 0;
        for(u32 Index = 0;
            Index < ALen;
            ++Index)
        {
            Result[CharIndex] = A[Index];
            CharIndex++;
        }
        for(u32 Index = 0;
            Index < BLen;
            ++Index)
        {
            Result[CharIndex] = B[Index];
            CharIndex++;
        }
        Result[ALen + BLen] = 0;
    }
    else if(A)
    {
        Result = A;
    }
    else
    {
        Result = B;
    }
    return(Result);
}

internal char *
Concat(memory_arena *Arena, char *A, char *B, char *C)
{
    char *Result = Concat(Arena, A, Concat(Arena, B, C));
    return(Result);
}

internal char *
Concat(memory_arena *Arena, char *A, char *B, char *C, char *D)
{
    char *Result = Concat(Arena, A, B, Concat(Arena, C, D));
    return(Result);
}

u32
DigitsCount(u32 Value)
{
    u32 Result = 0;
    while(Value != 0)
    {
        Value /= 10;
        Result++;
    }
    
    return(Result);
}

internal char *
U32ToString(memory_arena *Arena, u32 Value)
{
    char Buffer[32];
    
    u32 Index = 0;
    
    if(Value == 0)
    {
        Buffer[0] = '0';
        Buffer[1] = '\0';
    }
    else
    {
        while(Value != 0)
        {
            Buffer[Index] = Value % 10 + '0';
            Value = Value / 10;
            Index++;
        }
        
        Buffer[Index] = '\0';
    }
    
    int i = Index;
    for(int t = 0; t < i / 2; t++)
    {
        Buffer[t] ^= Buffer[i - t - 1];
        Buffer[i - t - 1] ^= Buffer[t];
        Buffer[t] ^= Buffer[i - t - 1];
    }
    
    char *Result = PushString(Arena, Buffer);
    return(Result);
}

internal s32
S32FromZ(char *At)
{
    s32 Result = 0;
    
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }
    
    return(Result);
}

struct format_dest
{
    umm Size;
    char *At;
};

inline void
OutChar(format_dest *Dest, char Value)
{
    if(Dest->Size)
    {
        --Dest->Size;
        *Dest->At++ = Value;
    }
}

internal umm
FormatStringList(umm DestSize, char *DestInit, char *Format, va_list ArgList)
{
    format_dest Dest = {DestSize, DestInit};
    if(Dest.Size)
    {
        char *At = Format;
        while(At[0])
        {
            if(At[0] == '%')
            {
                // va_arg();
                ++At;
            }
            else
            {
                OutChar(&Dest, *At++);
            }
        }
        
        if(Dest.Size)
        {
            Dest.At[0] = 0;
        }
        else
        {
            Dest.At[-1] = 0;
        }
    }
    
    umm Result = Dest.At - DestInit;
    return(Result);
}

internal umm
FormatString(umm DestSize, char *Dest, char *Format, ...)
{
    va_list ArgList;
    
    va_start(ArgList, Format);
    umm Result = FormatStringList(DestSize, Dest, Format, ArgList);
    va_end(ArgList);
    
    return(Result);
}