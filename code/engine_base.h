#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s8 b8;
typedef s32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;
typedef intptr_t smm;
typedef uintptr_t umm;

typedef size_t memory_index;

#if !defined(internal)
#define internal static
#endif
#define local static
#define global static

// NOTE(ezexff): Floating point fix for no CRT release compile (MSVC)
#if !(ENGINE_INTERNAL)
extern "C"
{
    int _fltused;
}
#pragma function(memset)
void *
memset(void *_Dst, int _Val, size_t _Size)
{
    unsigned char Val = *(unsigned char *)&_Val;
    unsigned char *At = (unsigned char *)_Dst;
    while(_Size--)
    {
        *At++ = Val;
    }
    return(_Dst);
}
#endif

typedef float r32;
typedef r32 f32;
typedef double r64;
typedef r64 f64;

#define flag8(type) u8
#define flag16(type) u16
#define flag32(type) u32
#define flag64(type) u64

#define enum8(type) u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

#define U16Max 65535
#define S32Min ((s32)0x80000000)
#define S32Max ((s32)0x7fffffff)
#define U32Min 0
#define U32Max ((u32)-1)
#define U64Max ((u64)-1)
#define F32Max FLT_MAX
#define F32Min -FLT_MAX
#define F64Max DBL_MAX
#define F64Min -DBL_MAX

#if !defined(internal)
#define internal static
#endif

#define Pi32 3.14159265359f
#define Pi32_2 1.57079632679f // pi/2 TODO(me): remove?
#define Pi32_4 0.78539816340f // pi/4 TODO(me): remove?
#define Tau32 6.28318530717958647692f

#if ENGINE_INTERNAL
#define Assert(Expression)                                                                                             \
if(!(Expression))                                                                                                  \
{                                                                                                                  \
*(int *)0 = 0;                                                                                                 \
}
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase                                                                                             \
default: {                                                                                                         \
InvalidCodePath;                                                                                               \
}                                                                                                                  \
break

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(me): swap, min, max (add if need)

#define AlignPow2(Value, Alignment) ((Value + ((Alignment)-1)) & ~((Alignment)-1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

//~ NOTE(ezexff): Vectors
union v2
{
    struct
    {
        r32 x, y;
    };
    r32 E[2];
};

union v2s
{
    struct
    {
        s32 x, y;
    };
    s32 E[2];
};

union v2u
{
    struct
    {
        s32 x, y;
    };
    u32 E[2];
};

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 r, g, b;
    };
    struct
    {
        r32 pitch, yaw, roll;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 yz;
    };
    r32 E[3];
};

union v3s
{
    struct
    {
        s32 x;
        s32 y;
        s32 z;
    };
    s32 E[3];
};

union v4
{
    struct
    {
        union {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };
        
        r32 w;
    };
    struct
    {
        union {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };
        
        r32 a;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
        r32 Ignored1_;
    };
    struct
    {
        r32 Ignored2_;
        v2 yz;
        r32 Ignored3_;
    };
    struct
    {
        r32 Ignored4_;
        r32 Ignored5_;
        v2 zw;
    };
    struct
    {
        r32 left, right, top, bottom;
    };
    r32 E[4];
};

//~ NOTE(ezexff): Rectangles
struct rectangle2
{
    v2 Min;
    v2 Max;
};

struct rectangle3
{
    v3 Min;
    v3 Max;
};

//~ NOTE(ezexff): Matrices
struct m4x4
{
    // NOTE(ezexff): These are stored ROW MAJOR - E[ROW][COLUMN]!!!
    r32 E[4][4];
};

struct m4x4_inv
{
    m4x4 Forward;
    m4x4 Inverse;
};

//~ NOTE(ezexff): String
struct buffer
{
    umm Count;
    u8 *Data; // NOTE(ezexff): wchar_t - 2 bytes per symbol
};
typedef buffer string;

inline u64
StringLength(char *String)
{
    u64 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }
    
    return (Count);
}

inline u64
StringLength(wchar_t *String)
{
    u64 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }
    
    return (Count);
}

inline b32
operator==(string A, string B)
{
    b32 Result = false;
    
    if(A.Count == B.Count)
    {
        for(u64 i = 0; i < A.Count; i++)
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

inline b32
operator==(string A, char *B)
{
    b32 Result = false;
    
    u64 ACount = A.Count - 1; // без символа конца строки
    u64 BCount = StringLength(B);
    if(ACount == BCount)
    {
        for(u64 i = 0; i < ACount; i++)
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

inline b32
operator==(string A, wchar_t *B)
{
    b32 Result = false;
    
    u64 ACount = A.Count - 1; // без символа конца строки
    u64 BCount = StringLength(B);
    if(ACount == BCount)
    {
        u8 *BU8 = (u8 *)B;
        u64 SizeInBytes = sizeof(wchar_t) * ACount;
        for(u64 i = 0; i < SizeInBytes; i++)
        {
            if(A.Data[i] != BU8[i])
            {
                return (Result);
            }
        }
        Result = true;
    }
    
    return (Result);
}

inline b32
StringsAreEqual(char *A, char *B)
{
    b32 Result = (A == B);
    
    if(A && B)
    {
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        
        Result = ((*A == 0) && (*B == 0));
    }
    
    return(Result);
}

inline b32
StringsAreEqual(umm ALength, char *A, char *B)
{
    b32 Result = false;
    
    if(B)
    {
        char *At = B;
        for(umm Index = 0;
            Index < ALength;
            ++Index, ++At)
        {
            if((*At == 0) ||
               (A[Index] != *At))
            {
                return(false);
            }        
        }
        
        Result = (*At == 0);
    }
    else
    {
        Result = (ALength == 0);
    }
    
    return(Result);
}

//~ NOTE(ezexff): from hmh shared file
internal s32
S32FromZInternal(char **AtInit)
{
    s32 Result = 0;
    
    char *At = *AtInit;
    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10;
        Result += (*At - '0');
        ++At;
    }
    
    *AtInit = At;
    
    return(Result);
}

internal s32
S32FromZ(char *At)
{
    char *Ignored = At;
    s32 Result = S32FromZInternal(&At);
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

inline void
OutChars(format_dest *Dest, char *Value)
{
    // NOTE(casey): Not particularly speedy, are we?  :P
    while(*Value)
    {
        OutChar(Dest, *Value++);
    }
}

#define _ReadVarArgUnsignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, u64) : (u64)va_arg(ArgList, u32)
#define _ReadVarArgSignedInteger(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, s64) : (s64)va_arg(ArgList, s32)
#define _ReadVarArgFloat(Length, ArgList) ((Length) == 8) ? va_arg(ArgList, f64) : (f64)va_arg(ArgList, f32)

char DecChars[] = "0123456789";
char LowerHexChars[] = "0123456789abcdef";
char UpperHexChars[] = "0123456789ABCDEF";
internal void
U64ToASCII(format_dest *Dest, u64 Value, u32 Base, char *Digits)
{
    Assert(Base != 0);
    
    char *Start = Dest->At;
    do
    {
        u64 DigitIndex = (Value % Base);
        char Digit = Digits[DigitIndex];
        OutChar(Dest, Digit);
        
        Value /= Base;
    } while(Value != 0);
    char *End = Dest->At;
    
    while(Start < End)
    {
        --End;
        char Temp = *End;
        *End = *Start;
        *Start = Temp;
        ++Start;
    }
}

internal void
F64ToASCII(format_dest *Dest, f64 Value, u32 Precision)
{
    if(Value < 0)
    {
        OutChar(Dest, '-');
        Value = -Value;
    }
    
    u64 IntegerPart = (u64)Value;
    Value -= (f64)IntegerPart;
    U64ToASCII(Dest, IntegerPart, 10, DecChars);
    
    OutChar(Dest, '.');
    
    // TODO(casey): Note that this is NOT an accurate way to do this!
    for(u32 PrecisionIndex = 0;
        PrecisionIndex < Precision;
        ++PrecisionIndex)
    {
        Value *= 10.0f;
        u32 Integer = (u32)Value;
        Value -= (f32)Integer;
        OutChar(Dest, DecChars[Integer]);
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
            if(*At == '%')
            {
                ++At;
                
                b32 ForceSign = false;
                b32 PadWithZeros = false;
                b32 LeftJustify = false;
                b32 PostiveSignIsBlank = false;
                b32 AnnotateIfNotZero = false;
                
                b32 Parsing = true;
                
                //
                // NOTE(casey): Handle the flags
                //
                Parsing = true;
                while(Parsing)
                {
                    switch(*At)
                    {
                        case '+': {ForceSign = true;} break;
                        case '0': {PadWithZeros = true;} break;
                        case '-': {LeftJustify = true;} break;
                        case ' ': {PostiveSignIsBlank = true;} break;
                        case '#': {AnnotateIfNotZero = true;} break;
                        default: {Parsing = false;} break;
                    }
                    
                    if(Parsing)
                    {
                        ++At;
                    }
                }
                
                //
                // NOTE(casey): Handle the width
                //
                b32 WidthSpecified = false;
                s32 Width = 0;
                if(*At == '*')
                {
                    Width = va_arg(ArgList, int);
                    WidthSpecified = true;
                    ++At;
                }
                else if((*At >= '0') && (*At <= '9'))
                {
                    Width = S32FromZInternal(&At);
                    WidthSpecified = true;
                }
                
                //
                // NOTE(casey): Handle the precision
                //
                b32 PrecisionSpecified = false;
                s32 Precision = 0;
                if(*At == '.')
                {
                    ++At;
                    
                    if(*At == '*')
                    {
                        Precision = va_arg(ArgList, int);
                        PrecisionSpecified = true;
                        ++At;
                    }
                    else if((*At >= '0') && (*At <= '9'))
                    {
                        Precision = S32FromZInternal(&At);
                        PrecisionSpecified = true;
                    }   
                    else
                    {
                        Assert(!"Malformed precision specifier!");
                    }
                }
                
                // TODO(casey): Right now our routine doesn't allow non-specified
                // precisions, so we just set non-specified precisions to a specified value
                if(!PrecisionSpecified)
                {
                    Precision = 6;
                }
                
                //
                // NOTE(casey): Handle the length
                //
                u32 IntegerLength = 4;
                u32 FloatLength = 8;
                // TODO(casey): Actually set different values here!
                if((At[0] == 'h') && (At[1] == 'h'))
                {
                    At += 2;
                }
                else if((At[0] == 'l') && (At[1] == 'l'))
                {
                    At += 2;
                }
                else if(*At == 'h')
                {
                    ++At;
                }
                else if(*At == 'l')
                {
                    ++At;
                }
                else if(*At == 'j')
                {
                    ++At;
                }
                else if(*At == 'z')
                {
                    ++At;
                }
                else if(*At == 't')
                {
                    ++At;
                }
                else if(*At == 'L')
                {
                    ++At;
                }
                
                char TempBuffer[64];
                char *Temp = TempBuffer;
                format_dest TempDest = {ArrayCount(TempBuffer), Temp};
                char *Prefix = "";
                b32 IsFloat = false;
                
                switch(*At)
                {
                    case 'd':
                    case 'i':
                    {
                        s64 Value = _ReadVarArgSignedInteger(IntegerLength, ArgList);
                        b32 WasNegative = (Value < 0);
                        if(WasNegative)
                        {
                            Value = -Value;
                        }
                        U64ToASCII(&TempDest, (u64)Value, 10, DecChars);
                        
                        // TODO(casey): Make this a common routine once floating
                        // point is available.
                        if(WasNegative)
                        {
                            Prefix = "-";
                        }
                        else if(ForceSign)
                        {
                            Assert(!PostiveSignIsBlank); // NOTE(casey): Not a problem here, but probably shouldn't be specified?
                            Prefix = "+";
                        }
                        else if(PostiveSignIsBlank)
                        {
                            Prefix = " ";
                        }
                    } break;
                    
                    case 'u':
                    {
                        u64 Value = _ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 10, DecChars);
                    } break;
                    
                    case 'o':
                    {
                        u64 Value = _ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 8, DecChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0";
                        }
                    } break;
                    
                    case 'x':
                    {
                        u64 Value = _ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, LowerHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0x";
                        }
                    } break;
                    
                    case 'X':
                    {
                        u64 Value = _ReadVarArgUnsignedInteger(IntegerLength, ArgList);
                        U64ToASCII(&TempDest, Value, 16, UpperHexChars);
                        if(AnnotateIfNotZero && (Value != 0))
                        {
                            Prefix = "0X";
                        }
                    } break;
                    
                    // TODO(casey): Support other kinds of floating point prints
                    // (right now we only do basic decimal output)
                    case 'f':
                    case 'F':
                    case 'e':
                    case 'E':
                    case 'g':
                    case 'G':
                    case 'a':
                    case 'A':
                    {
                        f64 Value = _ReadVarArgFloat(FloatLength, ArgList);
                        F64ToASCII(&TempDest, Value, Precision);
                        IsFloat = true;
                    } break;
                    
                    case 'c':
                    {
                        int Value = va_arg(ArgList, int);
                        OutChar(&TempDest, (char)Value);
                    } break;
                    
                    case 's':
                    {
                        char *String = va_arg(ArgList, char *);
                        
                        // TODO(casey): Obey precision, width, etc.
                        
                        Temp = String;
                        if(PrecisionSpecified)
                        {
                            TempDest.Size = 0;
                            for(char *Scan = String;
                                *Scan && (TempDest.Size < Precision);
                                ++Scan)
                            {
                                ++TempDest.Size;
                            }
                        }
                        else
                        {
                            TempDest.Size = StringLength(String);
                        }
                        TempDest.At = String + TempDest.Size;
                    } break;
                    
                    case 'p':
                    {
                        void *Value = va_arg(ArgList, void *);
                        U64ToASCII(&TempDest, *(umm *)&Value, 16, LowerHexChars);
                    } break;
                    
                    case 'n':
                    {
                        int *TabDest = va_arg(ArgList, int *);
                        *TabDest = (int)(Dest.At - DestInit);
                    } break;
                    
                    case '%':
                    {
                        OutChar(&Dest, '%');
                    } break;
                    
                    default:
                    {
                        Assert(!"Unrecognized format specifier");
                    } break;
                }
                
                if(TempDest.At - Temp)
                {
                    smm UsePrecision = Precision;
                    if(IsFloat || !PrecisionSpecified)
                    {
                        UsePrecision = (TempDest.At - Temp);
                    }
                    
                    smm PrefixLength = StringLength(Prefix);
                    smm UseWidth = Width;
                    smm ComputedWidth = UsePrecision + PrefixLength;
                    if(UseWidth < ComputedWidth)
                    {
                        UseWidth = ComputedWidth;
                    }
                    
                    if(PadWithZeros)
                    {
                        Assert(!LeftJustify); // NOTE(casey): Not a problem, but no way to do it?
                        LeftJustify = false;
                    }
                    
                    if(!LeftJustify)
                    {
                        while(UseWidth > (UsePrecision + PrefixLength))
                        {
                            OutChar(&Dest, PadWithZeros ? '0' : ' ');
                            --UseWidth;
                        }
                    }
                    
                    for(char *Pre = Prefix;
                        *Pre && UseWidth;
                        ++Pre)
                    {
                        OutChar(&Dest, *Pre);
                        --UseWidth;
                    }
                    
                    if(UsePrecision > UseWidth)
                    {
                        UsePrecision = UseWidth;
                    }
                    while(UsePrecision > (TempDest.At - Temp))
                    {
                        OutChar(&Dest, '0');
                        --UsePrecision;
                        --UseWidth;
                    }
                    while(UsePrecision && (TempDest.At != Temp))
                    {
                        OutChar(&Dest, *Temp++);
                        --UsePrecision;
                        --UseWidth;
                    }
                    
                    if(LeftJustify)
                    {
                        while(UseWidth)
                        {
                            OutChar(&Dest, ' ');
                            --UseWidth;
                        }
                    }
                }
                
                if(*At)
                {
                    ++At;
                }
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

//~ NOTE(ezexff): Asset
struct loaded_bitmap
{
    s32 Width;
    s32 Height;
    s32 BytesPerPixel;
    void *Memory;
    u32 OpenglID;
    //InternalFormat;
};

struct loaded_sound
{
    u32 SampleCount;
    u32 ChannelCount;
    s16 *Samples[2];
    
    void *Free;
};

struct loaded_shader
{
    u32 OpenglID;
    u8 Text[10000];
};

struct shader_program
{
    u32 OpenglID;
};

enum mesh_flags
{
    MeshFlag_HasTexCoords = (1 << 0),
    MeshFlag_HasNormals = (1 << 1),
    MeshFlag_HasTangents = (1 << 2),
    MeshFlag_HasIndices = (1 << 3),
    MeshFlag_HasMaterial = (1 << 4),
    MeshFlag_HasAnimations = (1 << 5),
};

struct mesh
{
    u32 VertCount;
    v3 *Positions;
    v2 *TexCoords;
    v3 *Normals;
    //v3 *Tangents; // Normal mapping
    
    u32 IndicesCount;
    u32 *Indices;
    
    u32 Flags;
};

struct loaded_model
{
    u32 MeshesCount;
    mesh *Meshes;
};

inline v4 RGBA(r32 R, r32 G, r32 B, r32 A)
{
    v4 Result = {};
    Result.r = R / 255.f;
    Result.g = G / 255.f;
    Result.b = B / 255.f;
    Result.a = A;
    return(Result);
}

//~ TODO(ezexff): mb start use this?
/*#define DLIST_INSERT(Sentinel, Element)         \
(Element)->Next = (Sentinel)->Next;         \
(Element)->Prev = (Sentinel);               \
(Element)->Next->Prev = (Element);          \
(Element)->Prev->Next = (Element); 
#define DLIST_INSERT_AS_LAST(Sentinel, Element)         \
(Element)->Next = (Sentinel);               \
(Element)->Prev = (Sentinel)->Prev;         \
(Element)->Next->Prev = (Element);          \
(Element)->Prev->Next = (Element); 

#define DLIST_INIT(Sentinel) \
(Sentinel)->Next = (Sentinel); \
(Sentinel)->Prev = (Sentinel);

#define FREELIST_ALLOCATE(Result, FreeListPointer, AllocationCode)             \
(Result) = (FreeListPointer); \
if(Result) {FreeListPointer = (Result)->NextFree;} else {Result = AllocationCode;}
#define FREELIST_DEALLOCATE(Pointer, FreeListPointer) \
if(Pointer) {(Pointer)->NextFree = (FreeListPointer); (FreeListPointer) = (Pointer);}
 */

