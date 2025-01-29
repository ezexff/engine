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
typedef double r64;

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

//~ NOTE(ezexff): Asset
struct loaded_bitmap
{
    //v2 AlignPercentage;
    //r32 WidthOverHeight;
    
    s32 Width;
    s32 Height;
    s32 BytesPerPixel;
    void *Memory;
    u32 OpenglID;
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