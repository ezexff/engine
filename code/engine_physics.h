struct segment_distance
{
    r32 DistanceSquared;
    v2 ClosestPoint;
};

struct projection
{
    r32 Min;
    r32 Max;
};

struct intersect_result
{
    b32 IsCollides;
    v2 Normal;
    r32 Depth;
};

struct controlled_entity
{
    u32 EntityIndex;
    v2 ddP;
};

enum test_entity_type
{
    TestEntityType_Rect,
    TestEntityType_Circle,
    
    TestEntityType_Count,
};

struct test_entity
{
    test_entity_type Type;
    b32 IsStatic;
    b32 IsInitialized;
    rectangle2 AABB; // collision detection optimization with AABB check
    
    //~ NOTE(ezexff): physics
    v2 P;
    v2 dP;
    v2 ddP;
    
    r32 Angle;
    v2 dPAngular;
    
    r32 Density;
    r32 Restitution;
    r32 Mass;
    r32 InvMass;
    r32 Inertia;
    r32 InvInertia;
    
    
    //~ NOTE(ezexff): draw
    v4 Color;
    v4 OutlineColor;
    
    //~ NOTE(ezexff): entity types
    
    // NOTE(ezexff): circle
    r32 Radius;
    
    // NOTE(ezexff): rect
    v2 Dim;
    u32 VertexCount;
    v2 VertexArray[4];
    v2 TransformedVertexArray[4];
    
    //~ NOTE(ezexff): outdated (physics1)
    r32 Size;
    v2 Force;
    r32 ForceMagnitude;
};