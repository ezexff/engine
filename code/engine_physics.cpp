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

void ResolveCollisionBasic(test_contact Contact)
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
        
        A->dP += -Impulse * A->InvMass;
        B->dP += Impulse * B->InvMass;
    }
}

internal void
ResolveCollisionWithRotation(test_contact Contact)
{
    test_entity *BodyA = Contact.BodyA;
    test_entity *BodyB = Contact.BodyB;
    v2 Normal = Contact.Normal;
    v2 Contact1 = Contact.ContactPoints.P1;
    v2 Contact2 = Contact.ContactPoints.P2;
    u32 ContactCount = Contact.ContactPoints.Count;
    
    v2 ContactArray[2] = {Contact1, Contact2};
    v2 ImpulseArray[2] = {};
    v2 RAArray[2] = {};
    v2 RBArray[2] = {};
    
    r32 e = Minimum(BodyA->Restitution, BodyB->Restitution);
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 RA = ContactArray[Index] - BodyA->P;
        v2 RB = ContactArray[Index] - BodyB->P;
        
        RAArray[Index] = RA;
        RBArray[Index] = RB;
        
        v2 RAPerp = V2(-RA.y, RA.x);
        v2 RBPerp = V2(-RB.y, RB.x);
        
        v2 AngularLinearVelocityA = RAPerp * BodyA->dPAngular;
        v2 AngularLinearVelocityB = RBPerp * BodyB->dPAngular;
        
        v2 RelativeVelocity =
        (BodyB->dP + AngularLinearVelocityB) -
        (BodyA->dP + AngularLinearVelocityA);
        
        r32 ContactVelocityMagnitude = Inner(RelativeVelocity, Normal);
        
        if(ContactVelocityMagnitude <= 0.0f)
        {
            r32 RAPerpDotN = Inner(RAPerp, Normal);
            r32 RBPerpDotN = Inner(RBPerp, Normal);
            
            r32 Denom = BodyA->InvMass + BodyB->InvMass +
                Square(RAPerpDotN) * BodyA->InvInertia +
                Square(RBPerpDotN) * BodyB->InvInertia;
            
            r32 j = -(1.0f + e) * ContactVelocityMagnitude;
            j /= Denom;
            j /= (r32)ContactCount;
            v2 Impulse = j * Normal;
            ImpulseArray[Index] = Impulse;
        }
    }
    
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 Impulse = ImpulseArray[Index];
        v2 RA = RAArray[Index];
        v2 RB = RBArray[Index];
        
        BodyA->dP += -Impulse * BodyA->InvMass;
        BodyA->dPAngular += -Cross(RA, Impulse) * BodyA->InvInertia;
        BodyB->dP += Impulse * BodyB->InvMass;
        BodyB->dPAngular += Cross(RB, Impulse) * BodyB->InvInertia;
        
    }
}

internal void
ResolveCollisionWithRotationAndFriction(test_contact Contact)
{
    test_entity *BodyA = Contact.BodyA;
    test_entity *BodyB = Contact.BodyB;
    v2 Normal = Contact.Normal;
    v2 Contact1 = Contact.ContactPoints.P1;
    v2 Contact2 = Contact.ContactPoints.P2;
    u32 ContactCount = Contact.ContactPoints.Count;
    
    v2 ContactArray[2] = {Contact1, Contact2};
    v2 ImpulseArray[2] = {};
    v2 RAArray[2] = {};
    v2 RBArray[2] = {};
    v2 FrictionImpulseArray[2] = {};
    r32 jArray[2] = {};
    
    r32 e = Minimum(BodyA->Restitution, BodyB->Restitution);
    
    r32 SF = (BodyA->StaticFriction + BodyB->StaticFriction) * 0.5f;
    r32 DF = (BodyA->DynamicFriction + BodyB->DynamicFriction) * 0.5f;
    
    // NOTE(ezexff): rotation
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 RA = ContactArray[Index] - BodyA->P;
        v2 RB = ContactArray[Index] - BodyB->P;
        
        RAArray[Index] = RA;
        RBArray[Index] = RB;
        
        v2 RAPerp = V2(-RA.y, RA.x);
        v2 RBPerp = V2(-RB.y, RB.x);
        
        v2 AngularLinearVelocityA = RAPerp * BodyA->dPAngular;
        v2 AngularLinearVelocityB = RBPerp * BodyB->dPAngular;
        
        v2 RelativeVelocity =
        (BodyB->dP + AngularLinearVelocityB) -
        (BodyA->dP + AngularLinearVelocityA);
        
        r32 ContactVelocityMagnitude = Inner(RelativeVelocity, Normal);
        
        if(ContactVelocityMagnitude <= 0.0f)
        {
            r32 RAPerpDotN = Inner(RAPerp, Normal);
            r32 RBPerpDotN = Inner(RBPerp, Normal);
            
            r32 Denom = BodyA->InvMass + BodyB->InvMass +
                Square(RAPerpDotN) * BodyA->InvInertia +
                Square(RBPerpDotN) * BodyB->InvInertia;
            
            r32 j = -(1.0f + e) * ContactVelocityMagnitude;
            j /= Denom;
            j /= (r32)ContactCount;
            
            jArray[Index] = j;
            
            v2 Impulse = j * Normal;
            ImpulseArray[Index] = Impulse;
        }
    }
    
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 Impulse = ImpulseArray[Index];
        v2 RA = RAArray[Index];
        v2 RB = RBArray[Index];
        
        BodyA->dP += -Impulse * BodyA->InvMass;
        BodyA->dPAngular += -Cross(RA, Impulse) * BodyA->InvInertia;
        BodyB->dP += Impulse * BodyB->InvMass;
        BodyB->dPAngular += Cross(RB, Impulse) * BodyB->InvInertia;
        
    }
    
    // NOTE(ezexff): friction
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 RA = ContactArray[Index] - BodyA->P;
        v2 RB = ContactArray[Index] - BodyB->P;
        
        RAArray[Index] = RA;
        RBArray[Index] = RB;
        
        v2 RAPerp = V2(-RA.y, RA.x);
        v2 RBPerp = V2(-RB.y, RB.x);
        
        v2 AngularLinearVelocityA = RAPerp * BodyA->dPAngular;
        v2 AngularLinearVelocityB = RBPerp * BodyB->dPAngular;
        
        v2 RelativeVelocity =
        (BodyB->dP + AngularLinearVelocityB) -
        (BodyA->dP + AngularLinearVelocityA);
        
        v2 Tangent = RelativeVelocity - Inner(RelativeVelocity, Normal) * Normal;
        
        if(NearlyEqual(PHYSICS_EPSILON, Tangent, V2(0, 0)))
        {
            continue;
        }
        else
        {
            Tangent = Normalize(Tangent);
        }
        
        r32 RAPerpDotT = Inner(RAPerp, Tangent);
        r32 RBPerpDotT = Inner(RBPerp, Tangent);
        
        r32 Denom = BodyA->InvMass + BodyB->InvMass +
            Square(RAPerpDotT) * BodyA->InvInertia +
            Square(RBPerpDotT) * BodyB->InvInertia;
        
        r32 jt = -Inner(RelativeVelocity, Tangent);
        jt /= Denom;
        jt /= (r32)ContactCount;
        
        v2 FrictionImpulse = {};
        r32 j = jArray[Index];
        if(AbsoluteValue(jt) <= j * SF)
        {
            FrictionImpulse = jt * Tangent;
        }
        else
        {
            FrictionImpulse = -j * Tangent * DF;
        }
        
        FrictionImpulseArray[Index] = FrictionImpulse;
    }
    
    for(u32 Index = 0;
        Index < ContactCount;
        ++Index)
    {
        v2 FrictionImpulse = FrictionImpulseArray[Index];
        v2 RA = RAArray[Index];
        v2 RB = RBArray[Index];
        
        BodyA->dP += -FrictionImpulse * BodyA->InvMass;
        BodyA->dPAngular += -Cross(RA, FrictionImpulse) * BodyA->InvInertia;
        BodyB->dP += FrictionImpulse * BodyB->InvMass;
        BodyB->dPAngular += Cross(RB, FrictionImpulse) * BodyB->InvInertia;
        
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