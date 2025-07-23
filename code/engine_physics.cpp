internal segment_distance
PointSegmentDistance(v2 P, v2 A, v2 B)
{
    segment_distance Result = {};
    
    v2 AB = B - A;
    v2 AP = P - A;
    
    r32 Projection = Inner(AP, AB);
    r32 ABLengthSquared = LengthSq(AB);
    r32 d = Projection / ABLengthSquared;
    
    if(d <= 0.0f)
    {
        Result.ClosestPoint = A;
    }
    else if(d >= 1.0f)
    {
        Result.ClosestPoint = B;
    }
    else
    {
        Result.ClosestPoint = A + AB * d;
    }
    Result.DistanceSquared = DistanceSq(P, Result.ClosestPoint);
    r32 Test = DistanceSq2(P, Result.ClosestPoint); // TODO(ezexff): temp remove
    Assert(Result.DistanceSquared == Test);
    return(Result);
}

internal contact_points
FindPolygonsContactPoint(v2 CenterA, u32 VertexCountA, v2 *VertexArrayA, v2 CenterB, u32 VertexCountB, v2 *VertexArrayB)
{
    contact_points Result = {};
    r32 MinDistanceSquared = F32Max;
    for(u32 Inner = 0;
        Inner < VertexCountA;
        ++Inner)
    {
        v2 P = VertexArrayA[Inner];
        
        for(u32 Outer = 0;
            Outer < VertexCountB;
            ++Outer)
        {
            v2 VA = VertexArrayB[Outer];
            v2 VB = VertexArrayB[(Outer + 1) % VertexCountB];
            segment_distance SD = PointSegmentDistance(P, VA, VB);
            if(NearlyEqual(PHYSICS_EPSILON, SD.DistanceSquared, MinDistanceSquared))
            {
                if(!NearlyEqual(PHYSICS_EPSILON, SD.ClosestPoint, Result.P1))
                {
                    Result.P2 = SD.ClosestPoint;
                    Result.Count = 2;
                }
            }
            else if(SD.DistanceSquared < MinDistanceSquared)
            {
                MinDistanceSquared = SD.DistanceSquared;
                Result.P1 = SD.ClosestPoint;
                Result.Count = 1;
            }
        }
    }
    
    for(u32 Inner = 0;
        Inner < VertexCountB;
        ++Inner)
    {
        v2 P = VertexArrayB[Inner];
        
        for(u32 Outer = 0;
            Outer < VertexCountA;
            ++Outer)
        {
            v2 VA = VertexArrayA[Outer];
            v2 VB = VertexArrayA[(Outer + 1) % VertexCountA];
            segment_distance SD = PointSegmentDistance(P, VA, VB);
            if(NearlyEqual(PHYSICS_EPSILON, SD.DistanceSquared, MinDistanceSquared))
            {
                if(!NearlyEqual(PHYSICS_EPSILON, SD.ClosestPoint, Result.P1))
                {
                    Result.P2 = SD.ClosestPoint;
                    Result.Count = 2;
                }
            }
            else if(SD.DistanceSquared < MinDistanceSquared)
            {
                MinDistanceSquared = SD.DistanceSquared;
                Result.P1 = SD.ClosestPoint;
                Result.Count = 1;
            }
        }
    }
    return(Result);
}

internal v2
FindCirclePolygonContactPoint(v2 CircleCenter, r32 CircleRadius, v2 PolygonCenter, u32 VertexCount, v2 *VertexArray)
{
    v2 Contact = {};
    r32 MinDistanceSquared = F32Max;
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        v2 VA = VertexArray[Index];
        v2 VB = VertexArray[(Index + 1) % VertexCount];
        segment_distance SD = PointSegmentDistance(CircleCenter, VA, VB);
        
        if(SD.DistanceSquared < MinDistanceSquared)
        {
            MinDistanceSquared = SD.DistanceSquared;
            Contact = SD.ClosestPoint;
        }
    }
    
    return(Contact);
}

internal v2
FindCirclesContactPoint(v2 CenterA, r32 RadiusA, v2 CenterB)
{
    v2 AB = CenterB - CenterA;
    v2 Dir = Normalize(AB);
    v2 Result = CenterA + Dir * RadiusA;
    return(Result);
}

internal contact_points
FindContactPoints(test_entity *BodyA, test_entity *BodyB)
{
    contact_points Result = {};
    
    if(BodyA->Type == TestEntityType_Rect)
    {
        if(BodyB->Type == TestEntityType_Rect)
        {
            Result = FindPolygonsContactPoint(BodyA->P, BodyA->VertexCount, BodyA->TransformedVertexArray,
                                              BodyB->P, BodyB->VertexCount, BodyB->TransformedVertexArray);
        }
        else if(BodyB->Type == TestEntityType_Circle)
        {
            Result.P1 = FindCirclePolygonContactPoint(BodyB->P, BodyB->Radius,
                                                      BodyA->P, BodyA->VertexCount, BodyA->TransformedVertexArray);
            Result.Count = 1;
        }
    }
    else if(BodyA->Type == TestEntityType_Circle)
    {
        if(BodyB->Type == TestEntityType_Rect)
        {
            Result.P1 = FindCirclePolygonContactPoint(BodyA->P, BodyA->Radius,
                                                      BodyB->P, BodyB->VertexCount, BodyB->TransformedVertexArray);
            Result.Count = 1;
        }
        else if(BodyB->Type == TestEntityType_Circle)
        {
            Result.P1 = FindCirclesContactPoint(BodyA->P, BodyA->Radius, BodyB->P);
            Result.Count = 1;
        }
    }
    return(Result);
}

void ResolveCollisionOptimized(test_contact Contact)
{
    test_entity *A = Contact.BodyA;
    test_entity *B = Contact.BodyB;
    v2 Normal = Contact.Normal;
    
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
    r32 Distance1 = Distance2(TestP, P);
    r32 Test = Distance(TestP, P); // TODO(ezexff): temp remove
    Assert(Distance1 == Test);
    r32 Radii = Radius + TestRadius;
    if(Distance1 < Radii)
    {
        Result.IsCollides = true;;
        Result.Normal = Normalize(TestP - P);
        Result.Depth = Radii - Distance1;
        
    }
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
IntersectPolygonsOptimized(v2 CenterA, u32 VertexCountA, v2 *VertexArrayA, v2 CenterB, u32 VertexCountB, v2 *VertexArrayB)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCountA;
        ++Index)
    {
        v2 VA = VertexArrayA[Index];
        v2 VB = VertexArrayA[(Index + 1) % VertexCountA];
        v2 Edge = VB - VA;
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
        v2 VA = VertexArrayB[Index];
        v2 VB = VertexArrayB[(Index + 1) % VertexCountB];
        v2 Edge = VB - VA;
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
    
    v2 Direction = CenterB - CenterA;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    
    Result.IsCollides = true;
    return(Result);
}