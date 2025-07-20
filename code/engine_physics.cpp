void ResolveCollision(test_entity *A, test_entity *B, v2 Normal, r32 Depth)
{
    v2 RelVel = B->dP - A->dP;
    
    if(Inner(RelVel, Normal) <= 0.0f)
    {
        r32 e = Minimum(A->Restitution, B->Restitution);
        r32 j = -(1.0f + e) * Inner(RelVel, Normal);
        j /= A->InvMass + B->InvMass;
        
        v2 Impulse = j * Normal;
        A->dP += -(Impulse * A->InvMass);
        B->dP += Impulse * B->InvMass;
    }
}

intersect_result IntersectCircles(v2 P, r32 Radius, v2 TestP, r32 TestRadius)
{
    intersect_result Result = {};
    r32 Distance = Length(TestP - P);
    r32 Radii = Radius + TestRadius;
    if(Distance < Radii)
    {
        Result.IsCollides = true;;
        Result.Normal = Normalize(TestP - P);
        Result.Depth = Radii - Distance;
        
    }
    return(Result);
}

v2 FindSumVector(u32 VertexCount, v2 *VertexArray)
{
    v2 Result = {};
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        Result.x += VertexArray[Index].x;
        Result.y += VertexArray[Index].y;
    }
    
    Result.x = Result.x / (VertexCount);
    Result.y = Result.y / (VertexCount);
    
    return(Result);
}

s32 FindClosestPointOnPolygion(v2 CircleCenter, u32 VertexCount, v2 *VertexArray)
{
    s32 Result = -1;
    r32 MinDistance = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        r32 Distance = Length(VertexArray[Index] - CircleCenter);
        if(Distance < MinDistance)
        {
            MinDistance = Distance;
            Result = Index;
        }
    }
    return(Result);
}

projection
ProjectCircle(v2 Center, r32 Radius, v2 Axis)
{
    projection Result = {};
    
    v2 Direction = Normalize(Axis);
    v2 DirectionAndRadius = Direction * Radius;
    
    v2 P1 = Center - DirectionAndRadius;
    v2 P2 = Center + DirectionAndRadius;
    
    Result.Min = Inner(P1, Axis);
    Result.Max = Inner(P2, Axis);
    
    if(Result.Min > Result.Min)
    {
        r32 Temp = Result.Min;
        Result.Min = Result.Max;
        Result.Max = Temp;
    }
    
    return(Result);
}

projection
ProjectVertices(u32 VertexCount, v2 *VertexArray, v2 Axis)
{
    projection Result = {};
    Result.Min = F32Max;
    Result.Max = F32Min;
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        r32 Projection = Inner(VertexArray[Index], Axis);
        if(Projection < Result.Min){ Result.Min = Projection;}
        if(Projection > Result.Max){ Result.Max = Projection;}
    }
    
    return(Result);
}

intersect_result
IntersectCirclePolygon(v2 CircleCenter, r32 CircleRadius, u32 VertexCount, v2 *VertexArray)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    projection A = {};
    projection B = {};
    v2 Axis = {};
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        v2 *VA = VertexArray + Index;
        v2 *VB = VertexArray + (Index + 1) % VertexCount;
        v2 Edge = *VB - *VA;
        Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        A = ProjectVertices(VertexCount, VertexArray, Axis);
        B = ProjectCircle(CircleCenter, CircleRadius, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    s32 CpIndex = FindClosestPointOnPolygion(CircleCenter, VertexCount, VertexArray);
    v2 Cp = VertexArray[CpIndex];
    Axis = Cp - CircleCenter;
    Axis = Normalize(Axis);
    
    A = ProjectVertices(VertexCount, VertexArray, Axis);
    B = ProjectCircle(CircleCenter, CircleRadius, Axis);
    
    if(A.Min >= B.Max || B.Min >= A.Max)
    {
        Result.IsCollides = false;
        return(Result);
    }
    
    r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
    if(AxisDepth < Result.Depth)
    {
        Result.Depth = AxisDepth;
        Result.Normal = Axis;
    }
    
    Result.IsCollides = true;
    
    v2 PolygonCenter = FindSumVector(VertexCount, VertexArray);
    v2 Direction = PolygonCenter - CircleCenter;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    return(Result);
}

intersect_result
IntersectCirclePolygonOptimized(v2 CircleCenter, r32 CircleRadius, v2 PolygonCenter, u32 VertexCount, v2 *VertexArray)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    projection A = {};
    projection B = {};
    v2 Axis = {};
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        v2 *VA = VertexArray + Index;
        v2 *VB = VertexArray + (Index + 1) % VertexCount;
        v2 Edge = *VB - *VA;
        Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        A = ProjectVertices(VertexCount, VertexArray, Axis);
        B = ProjectCircle(CircleCenter, CircleRadius, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    s32 CpIndex = FindClosestPointOnPolygion(CircleCenter, VertexCount, VertexArray);
    v2 Cp = VertexArray[CpIndex];
    Axis = Cp - CircleCenter;
    Axis = Normalize(Axis);
    
    A = ProjectVertices(VertexCount, VertexArray, Axis);
    B = ProjectCircle(CircleCenter, CircleRadius, Axis);
    
    if(A.Min >= B.Max || B.Min >= A.Max)
    {
        Result.IsCollides = false;
        return(Result);
    }
    
    r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
    if(AxisDepth < Result.Depth)
    {
        Result.Depth = AxisDepth;
        Result.Normal = Axis;
    }
    
    Result.IsCollides = true;
    
    v2 Direction = PolygonCenter - CircleCenter;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    return(Result);
}

intersect_result
IntersectPolygons(u32 VertexCountA, v2 *VertexArrayA, u32 VertexCountB, v2 *VertexArrayB)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCountA;
        ++Index)
    {
        v2 *VA = VertexArrayA + Index;
        v2 *VB = VertexArrayA + (Index + 1) % VertexCountA;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    for(u32 Index = 0;
        Index < VertexCountB;
        ++Index)
    {
        v2 *VA = VertexArrayB + Index;
        v2 *VB = VertexArrayB + (Index + 1) % VertexCountB;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    Result.IsCollides = true;
    
    v2 CenterA = FindSumVector(VertexCountA, VertexArrayA);
    v2 CenterB = FindSumVector(VertexCountB, VertexArrayB);
    v2 Direction = CenterB - CenterA;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    
    return(Result);
}

intersect_result
IntersectPolygonsOptimized(v2 CenterA, u32 VertexCountA, v2 *VertexArrayA, v2 CenterB, u32 VertexCountB, v2 *VertexArrayB)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCountA;
        ++Index)
    {
        v2 *VA = VertexArrayA + Index;
        v2 *VB = VertexArrayA + (Index + 1) % VertexCountA;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    for(u32 Index = 0;
        Index < VertexCountB;
        ++Index)
    {
        v2 *VA = VertexArrayB + Index;
        v2 *VB = VertexArrayB + (Index + 1) % VertexCountB;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    Result.IsCollides = true;
    
    v2 Direction = CenterB - CenterA;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    
    return(Result);
}