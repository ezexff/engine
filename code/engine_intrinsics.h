//
// TODO(casey): Convert all of these to platform-efficient versions
// and remove math.h
//

#include "math.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline s32 RoundReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)roundf(Real32);
    return (Result);
}

inline u32 RoundReal32ToUInt32(r32 Real32)
{
    u32 Result = (u32)roundf(Real32);
    return (Result);
}

inline r32 AbsoluteValue(r32 Real32)
{
    r32 Result = (r32)fabs(Real32);
    return (Result);
}

inline r32 SquareRoot(r32 Real32)
{
    r32 Result = sqrtf(Real32);
    return (Result);
}

inline r32 Cos(r32 Angle)
{
    r32 Result = cosf(Angle);
    return (Result);
}

inline r32 Sin(r32 Angle)
{
    r32 Result = sinf(Angle);
    return (Result);
}

inline s32 FloorReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    return (Result);
}

internal r32 DebugGetRandomNumberR32(r32 Min, r32 Max, u32 Precision)
{
    r32 Result;

    // получить случайное число как целое число с порядком precision
    Result = (r32)(rand() % (int)pow(10, Precision));

    // получить вещественное число
    Result = (r32)(Min + (Result / pow(10, Precision)) * (Max - Min));

    return (Result);
}

/*
#if COMPILER_MSVC
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline u32 AtomicCompareExchangeUInt32(u32 volatile *Value, u32 New, u32 Expected)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);

    return(Result);
}
#elif COMPILER_LLVM
// TODO(casey): Does LLVM have real read-specific barriers yet?
#define CompletePreviousReadsBeforeFutureReads asm volatile("" ::: "memory")
#define CompletePreviousWritesBeforeFutureWrites asm volatile("" ::: "memory")
inline u32 AtomicCompareExchangeUInt32(u32 volatile *Value, u32 New, u32 Expected)
{
    u32 Result = __sync_val_compare_and_swap(Value, Expected, New);

    return(Result);
}
#else
// TODO(casey): Other compilers/platforms??
#endif

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline r32
SignOf(r32 Value)
{
    r32 Result = (Value >= 0) ? 1.0f : -1.0f;
    return(Result);
}

inline r32
SquareRoot(r32 Real32)
{
    r32 Result = sqrtf(Real32);
    return(Result);
}

inline r32
AbsoluteValue(r32 Real32)
{
    r32 Result = (r32)fabs(Real32);
    return(Result);
}

inline u32
RotateLeft(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
    u32 Result = _rotl(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    u32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(Result);
}

inline u32
RotateRight(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
    u32 Result = _rotr(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    u32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}




inline s32
CeilReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)ceilf(Real32);
    return(Result);
}

inline s32
TruncateReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)Real32;
    return(Result);
}

inline r32
ATan2(r32 Y, r32 X)
{
    r32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}
*/