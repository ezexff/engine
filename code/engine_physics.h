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
    b32 IsInitialized;
    s32 Type;
    v4 Color;
    v4 OutlineColor;
    
    // NOTE(ezexff): move spec
    b32 IsStatic;
    r32 ForceMagnitude;
    v2 Force;
    v2 P;
    v2 dP;
    v2 ddP;
    r32 Density;
    r32 Mass;
    r32 InvMass;
    r32 Restitution;
    rectangle2 AABB;
    
    //~ NOTE(ezexff): entity types
    
    // NOTE(ezexff): rect
    v2 Dim;
    r32 Size;
    r32 Angle;
    u32 VertexCount;
    v2 VertexArray[4];
    v2 TransformedVertexArray[4];
    
    // NOTE(ezexff): circle
    r32 Radius;
};