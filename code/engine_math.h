inline r32 DegToRad(r32 Value)
{
    r32 Result = Value * Pi32 / 180;
    
    return(Result);
}

//~ NOTE(ezexff): Scalar operations
inline r32 Square(r32 A)
{
    r32 Result = A * A;
    
    return(Result);
}

inline r32 Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t) * A + t * B;
    
    return(Result);
}

inline r32 Clamp(r32 Min, r32 Value, r32 Max)
{
    r32 Result = Value;
    
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }
    
    return(Result);
}

inline r32 Clamp01(r32 Value)
{
    r32 Result = Clamp(0.0f, Value, 1.0f);
    
    return(Result);
}

inline r32 SafeRatioN(r32 Numerator, r32 Divisor, r32 N)
{
    r32 Result = N;
    
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }
    
    return(Result);
}

inline r32 SafeRatio0(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
    
    return(Result);
}

inline r32 SafeRatio1(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    
    return(Result);
}

//~ NOTE(ezexff): v2 init
inline v2 V2i(s32 X, s32 Y)
{
    v2 Result = {(r32)X, (r32)Y};
    
    return(Result);
}

inline v2 V2i(u32 X, u32 Y)
{
    v2 Result = {(r32)X, (r32)Y};
    
    return(Result);
}

inline v2 V2(r32 X, r32 Y)
{
    v2 Result;
    
    Result.x = X;
    Result.y = Y;
    
    return(Result);
}

// NOTE(ezexff): v2 operations
inline v2 Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}

inline v2 operator*(r32 A, v2 B)
{
    v2 Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    
    return(Result);
}

inline v2 operator*(v2 B, r32 A)
{
    v2 Result = A * B;
    
    return(Result);
}

inline v2 &operator*=(v2 &B, r32 A)
{
    B = A * B;
    
    return (B);
}

inline v2 operator-(v2 A)
{
    v2 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    
    return(Result);
}

inline v2 operator+(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    
    return(Result);
}

inline v2 &operator+=(v2 &A, v2 B)
{
    A = A + B;
    
    return (A);
}

inline v2 operator-(v2 A, v2 B)
{
    v2 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    
    return(Result);
}

inline v2 Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x * B.x, A.y * B.y};
    
    return(Result);
}

inline r32 Inner(v2 A, v2 B)
{
    r32 Result = A.x * B.x + A.y * B.y;
    
    return(Result);
}

inline r32 LengthSq(v2 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32 Length(v2 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2 Clamp01(v2 Value)
{
    v2 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    
    return(Result);
}

//~ NOTE(ezexff): v3 init
inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return(Result);
}

inline v3
V3(v2 XY, r32 Z)
{
    v3 Result;
    
    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;
    
    return(Result);
}

inline v3
ToV3(v2 XY, r32 Z)
{
    v3 Result;
    
    Result.xy = XY;
    Result.z = Z;
    
    return(Result);
}

// NOTE(ezexff): v3 operations
inline v3 operator*(r32 A, v3 B)
{
    v3 Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    
    return(Result);
}

inline v3 operator*(v3 B, r32 A)
{
    v3 Result = A * B;
    
    return(Result);
}

inline v3 &operator*=(v3 &B, r32 A)
{
    B = A * B;
    
    return (B);
}

inline v3 operator-(v3 A)
{
    v3 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    
    return(Result);
}

inline v3 operator+(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3 &operator+=(v3 &A, v3 B)
{
    A = A + B;
    
    return (A);
}

inline v3 operator-(v3 A, v3 B)
{
    v3 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}

inline v3 Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x * B.x, A.y * B.y, A.z * B.z};
    
    return(Result);
}

inline r32 Inner(v3 A, v3 B)
{
    r32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    
    return(Result);
}

inline r32 LengthSq(v3 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32 Length(v3 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v3 Normalize(v3 A)
{
    v3 Result = A * (1.0f / Length(A));
    
    return(Result);
}

inline v3 Clamp01(v3 Value)
{
    v3 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    
    return(Result);
}

inline v3 Lerp(v3 A, r32 t, v3 B)
{
    v3 Result = (1.0f - t) * A + t * B;
    
    return(Result);
}

//~ NOTE(ezexff): v4 init
inline v4 V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    
    return(Result);
}

inline v4 V4(v3 XYZ, r32 W)
{
    v4 Result;
    
    Result.xyz = XYZ;
    Result.w = W;
    
    return(Result);
}

// NOTE(ezexff): v4 operations
inline v4 operator*(r32 A, v4 B)
{
    v4 Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    Result.w = A * B.w;
    
    return(Result);
}

inline v4 operator*(v4 B, r32 A)
{
    v4 Result = A * B;
    
    return(Result);
}

inline v4 &operator*=(v4 &B, r32 A)
{
    B = A * B;
    
    return (B);
}

inline v4 operator-(v4 A)
{
    v4 Result;
    
    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;
    
    return(Result);
}

inline v4 operator+(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

inline v4 &operator+=(v4 &A, v4 B)
{
    A = A + B;
    
    return (A);
}

inline v4 operator-(v4 A, v4 B)
{
    v4 Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;
    
    return(Result);
}

inline v4 Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
    
    return(Result);
}

inline r32 Inner(v4 A, v4 B)
{
    r32 Result = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
    
    return(Result);
}

inline r32 LengthSq(v4 A)
{
    r32 Result = Inner(A, A);
    
    return(Result);
}

inline r32 Length(v4 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v4 Normalize(v4 A)
{
    v4 Result = A * (1.0f / Length(A));
    
    return(Result);
}

inline v4 Clamp01(v4 Value)
{
    v4 Result;
    
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);
    
    return(Result);
}

inline v4 Lerp(v4 A, r32 t, v4 B)
{
    v4 Result = (1.0f - t) * A + t * B;
    
    return(Result);
}

inline v4 AiNormalize(v4 A)
{
    // compute the magnitude and divide through it
    r32 Mag = SquareRoot(A.x * A.x + A.y * A.y + A.z * A.z + A.w * A.w);
    if(Mag)
    {
        r32 InvMag = (r32)(1.0) / Mag;
        A.x *= InvMag;
        A.y *= InvMag;
        A.z *= InvMag;
        A.w *= InvMag;
    }
    
    return (A);
}

inline v4 AiLerp(v4 A, r32 t, v4 B)
{
#if 0
    r32 Cosom = A.x * B.x + A.y * B.y + A.z * B.z + A.w * B.w;
    
    // adjust signs (if necessary)
    v4 End = B;
    if(Cosom < (r32)(0.0))
    {
        Cosom = -Cosom;
        End.x = -End.x; // Reverse all signs
        End.y = -End.y;
        End.z = -End.z;
        End.w = -End.w;
    }
    
    // Calculate coefficients
    r32 Sclp, Sclq;
    if(((r32)(1.0) - Cosom) > (r32)(0.0001)) // 0.0001 -> some epsillon
    {
        // Standard case (slerp)
        r32 Omega, Sinom;
        Omega = (r32)acos(Cosom); // extract theta from dot product's cos theta
        Sinom = (r32)sin(Omega);
        Sclp = (r32)sin(((r32)(1.0) - t) * Omega) / Sinom;
        Sclq = (r32)sin(t * Omega) / Sinom;
    }
    else
    {
        // Very close, do linear interp (because it's faster)
        Sclp = (r32)(1.0) - t;
        Sclq = t;
    }
    
    v4 Result = V4(0, 0, 0, 0);
    Result.x = Sclp * A.x + Sclq * End.x;
    Result.y = Sclp * A.y + Sclq * End.y;
    Result.z = Sclp * A.z + Sclq * End.z;
    Result.w = Sclp * A.w + Sclq * End.w;
    
    return(Result);
    
    // calc cosine theta
    /*TReal cosom = pStart.x * pEnd.x + pStart.y * pEnd.y + pStart.z * pEnd.z + pStart.w * pEnd.w;

    // adjust signs (if necessary)
    aiQuaterniont end = pEnd;
    if(cosom < static_cast<TReal>(0.0))
    {
        cosom = -cosom;
        end.x = -end.x; // Reverse all signs
        end.y = -end.y;
        end.z = -end.z;
        end.w = -end.w;
    }

    // Calculate coefficients
    TReal sclp, sclq;
    if((static_cast<TReal>(1.0) - cosom) > static_cast<TReal>(0.0001)) // 0.0001 -> some epsillon
    {
        // Standard case (slerp)
        TReal omega, sinom;
        omega = std::acos(cosom); // extract theta from dot product's cos theta
        sinom = std::sin(omega);
        sclp = std::sin((static_cast<TReal>(1.0) - pFactor) * omega) / sinom;
        sclq = std::sin(pFactor * omega) / sinom;
    }
    else
    {
        // Very close, do linear interp (because it's faster)
        sclp = static_cast<TReal>(1.0) - pFactor;
        sclq = pFactor;
    }

    pOut.x = sclp * pStart.x + sclq * end.x;
    pOut.y = sclp * pStart.y + sclq * end.y;
    pOut.z = sclp * pStart.z + sclq * end.z;
    pOut.w = sclp * pStart.w + sclq * end.w;*/
#endif
}

//~ NOTE(ezexff): m4x4 init
inline m4x4 Identity(void)
{
    m4x4 R = 
    {
        {
            {1, 0, 0, 0},
            {0, 1, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        },
    };
    
    return (R);
}

// NOTE(ezexff): m4x4 operations
inline m4x4 Scale(r32 P)
{
    m4x4 R = 
    {
        {
            {1 * P, 0, 0, 0},
            {0, 1 * P, 0, 0},
            {0, 0, 1 * P, 0},
            {0, 0, 0, 1}
        },
    };
    
    return (R);
}

m4x4 Transpose(m4x4 A)
{
    m4x4 R;
    
    for(u32 i = 0; i < 4; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            R.E[i][j] = A.E[j][i];
        }
    }
    
    return (R);
}

inline m4x4 XRotation(r32 Angle)
{
    Angle = Angle * Pi32 / 180;
    r32 c = Cos(Angle);
    r32 s = Sin(Angle);
    
    m4x4 R = 
    {
        {
            {1, 0, 0, 0},
            {0, c, -s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}
        }
    };
    
    return (R);
}

inline m4x4 Translate(v3 T)
{
    m4x4 R = 
    {
        {
            {1, 0, 0, T.x},
            {0, 1, 0, T.y},
            {0, 0, 1, T.z},
            {0, 0, 0, 1}
        },
    };
    
    return (R);
}

internal m4x4 operator*(m4x4 A, m4x4 B)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    
    m4x4 R = {};
    
    for(int r = 0; r <= 3; ++r) // NOTE(casey): Rows (of A)
    {
        for(int c = 0; c <= 3; ++c) // NOTE(casey): Column (of B)
        {
            for(int i = 0; i <= 3; ++i) // NOTE(casey): Columns of A, rows of B!
            {
                R.E[r][c] += A.E[r][i] * B.E[i][c];
            }
        }
    }
    
    return (R);
}

//~ NOTE(ezexff): Rectangle init


// NOTE(ezexff): Rectangle operations
inline v2 GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2 GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2 GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f * (Rect.Min + Rect.Max);
    return(Result);
}

inline v2 GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline rectangle2 RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle2 RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle2 RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle2 AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;
    
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle2 RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f * Dim);
    
    return(Result);
}

inline b32 IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                  (Test.y >= Rectangle.Min.y) &&
                  (Test.x < Rectangle.Max.x) &&
                  (Test.y < Rectangle.Max.y));
    
    return(Result);
}

inline v2 GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    
    return(Result);
}

// NOTE(me): rectangle3 operations
inline v3 GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3 GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3 GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f * (Rect.Min + Rect.Max);
    return(Result);
}

inline v3 GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline rectangle3 RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Max;
    
    return(Result);
}

inline rectangle3 RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;
    
    Result.Min = Min;
    Result.Max = Min + Dim;
    
    return(Result);
}

inline rectangle3 RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;
    
    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;
    
    return(Result);
}

inline rectangle3 AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;
    
    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;
    
    return(Result);
}

inline rectangle3 Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;
    
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;
    
    return(Result);
}

inline rectangle3 RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f * Dim);
    
    return(Result);
}

inline b32 IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) && //
                  (Test.y >= Rectangle.Min.y) && //
                  (Test.z >= Rectangle.Min.z) && //
                  (Test.x < Rectangle.Max.x) &&  //
                  (Test.y < Rectangle.Max.y) &&  //
                  (Test.z < Rectangle.Max.z));
    
    return(Result);
}

inline b32 RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) || (B.Min.x >= A.Max.x) || (B.Max.y <= A.Min.y) || (B.Min.y >= A.Max.y) ||
                   (B.Max.z <= A.Min.z) || (B.Min.z >= A.Max.z));
    return(Result);
}

inline v3 GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;
    
    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);
    
    return(Result);
}

inline rectangle2 ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;
    
    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;
    
    return(Result);
}