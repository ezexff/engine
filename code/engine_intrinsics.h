//#include "math.h"

/*inline r32
Sin(r32 Angle)
{
    // NOTE(ezexff): SVML Sine version
    r32 Result = _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(Angle)));
    
    // NOTE(ezexff): CRT Sine version
    //r32 Result = sinf(Angle);
    
    // NOTE(ezexff): For _mm_sin_ps needed Short Vector Math Library (SVML) function
    //r32 Result[4];
    //Result[0] = 0.0f;
    //__m128 Angle_4x = _mm_set1_ps(Angle);
//__m128 Angle_4x = _mm_set_ps(Angle3, Angle2, Angle1, Angle0);
    //__m128 Result_4x = _mm_sin_ps(Angle_4x);
    //_mm_store_ps (Result, Result_4x);
    //return(Result[0]);
    
#if 0
    u64 Start1 = __rdtsc();
    r32 Result2 = sinf(Angle);
    u64 End1 = __rdtsc();
    
    u64 Start2 = __rdtsc();
    r32 Result3 = _mm_cvtss_f32(_mm_sin_ps(_mm_set_ss(Angle)));
    u64 End2 = __rdtsc();
    
    u64 CRTSinCycleCount = End1 - Start1;
    u64 SVMLSinCycleCount = End2 - Start2;
    
#endif
    return(Result);
}*/

#include "math.h"

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