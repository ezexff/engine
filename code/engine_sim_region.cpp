internal sim_entity_hash *
GetHashFromStorageIndex(sim_region *SimRegion, u32 StorageIndex)
{
    Assert(StorageIndex);
    
    sim_entity_hash *Result = 0;
    
    u32 HashValue = StorageIndex;
    for(u32 Offset = 0;                    //
        Offset < ArrayCount(SimRegion->Hash); //
        ++Offset)
    {
        u32 HashMask = (ArrayCount(SimRegion->Hash) - 1);
        u32 HashIndex = ((HashValue + Offset) & HashMask);
        sim_entity_hash *Entry = SimRegion->Hash + HashIndex;
        if((Entry->Index == 0) || (Entry->Index == StorageIndex))
        {
            Result = Entry;
            break;
        }
    }
    
    return (Result);
}

inline sim_entity *
GetEntityByStorageIndex(sim_region *SimRegion, u32 StorageIndex)
{
    sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
    sim_entity *Result = Entry->Ptr;
    return (Result);
}

inline v3 
GetSimSpaceP(sim_region *SimRegion, low_entity *Stored)
{
    // NOTE(casey): Map the entity into camera space
    // TODO(casey): Do we want to set this to signaling NAN in
    // debug mode to make sure nobody ever uses the position
    // of a nonspatial entity?
    v3 Result = InvalidP;
    if(!IsSet(&Stored->Sim, EntityFlag_Nonspatial))
    {
        Result = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
    }
    
    return (Result);
}

internal sim_entity *
AddEntity(mode_world *ModeWorld, sim_region *SimRegion, u32 StorageIndex, low_entity *Source,
          v3 *SimP);
inline void 
LoadEntityReference(mode_world *ModeWorld, sim_region *SimRegion, entity_reference *Ref)
{
    if(Ref->Index)
    {
        sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
        if(Entry->Ptr == 0)
        {
            Entry->Index = Ref->Index;
            low_entity *LowEntity = GetLowEntity(ModeWorld, Ref->Index);
            v3 P = GetSimSpaceP(SimRegion, LowEntity);
            Entry->Ptr = AddEntity(ModeWorld, SimRegion, Ref->Index, LowEntity, &P);
        }
        
        Ref->Ptr = Entry->Ptr;
    }
}

inline void 
StoreEntityReference(entity_reference *Ref)
{
    if(Ref->Ptr != 0)
    {
        Ref->Index = Ref->Ptr->StorageIndex;
    }
}

internal sim_entity *
AddEntityRaw(mode_world *ModeWorld, sim_region *SimRegion, u32 StorageIndex, low_entity *Source)
{
    Assert(StorageIndex);
    sim_entity *Entity = 0;
    
    sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
    if(Entry->Ptr == 0)
    {
        if(SimRegion->EntityCount < SimRegion->MaxEntityCount)
        {
            Entity = SimRegion->Entities + SimRegion->EntityCount++;
            
            Entry->Index = StorageIndex;
            Entry->Ptr = Entity;
            
            if(Source)
            {
                // TODO(casey): This should really be a decompression step, not
                // a copy!
                *Entity = Source->Sim;
                LoadEntityReference(ModeWorld, SimRegion, &Entity->Sword);
                
                Assert(!IsSet(&Source->Sim, EntityFlag_Simming));
                AddFlags(&Source->Sim, EntityFlag_Simming);
            }
            
            Entity->StorageIndex = StorageIndex;
            Entity->Updatable = false;
        }
        else
        {
            InvalidCodePath;
        }
    }
    
    return (Entity);
}

inline b32 
EntityOverlapsRectangle(v3 P, sim_entity_collision_volume Volume, rectangle3 Rect)
{
    rectangle3 Grown = AddRadiusTo(Rect, 0.5f * Volume.Dim);
    b32 Result = IsInRectangle(Grown, P + Volume.OffsetP);
    return (Result);
}

internal sim_entity *
AddEntity(mode_world *ModeWorld, sim_region *SimRegion, u32 StorageIndex, low_entity *Source,
          v3 *SimP)
{
    sim_entity *Dest = AddEntityRaw(ModeWorld, SimRegion, StorageIndex, Source);
    if(Dest)
    {
        if(SimP)
        {
            Dest->P = *SimP;
            Dest->Updatable =
                EntityOverlapsRectangle(Dest->P, Dest->Collision->TotalVolume, SimRegion->UpdatableBounds);
        }
        else
        {
            Dest->P = GetSimSpaceP(SimRegion, Source);
        }
    }
    
    return (Dest);
}

internal sim_region *
BeginSim(mode_world *ModeWorld, rectangle3 Bounds, r32 dt)
{
    world_position Origin = ModeWorld->Camera.P;
    world *World = ModeWorld->World;
    memory_arena *SimArena = ModeWorld->TranArena;
    // TODO(casey): If entities were stored in the world, we wouldn't need the game state here!
    
    sim_region *SimRegion = PushStruct(SimArena, sim_region);
    ZeroStruct(SimRegion->Hash);
    
    // TODO(casey): Try to make these get enforced more rigorously
    // TODO(casey): Perhaps try using a dual system here, where we support
    // entities larger than the max entity radius by adding them multiple times
    // to the spatial partition?
    SimRegion->MaxEntityRadius = 5.0f;
    //SimRegion->MaxEntityVelocity = 30.0f;
    SimRegion->MaxEntityVelocity = 3000.0f;
    r32 UpdateSafetyMargin = SimRegion->MaxEntityRadius + dt * SimRegion->MaxEntityVelocity;
    r32 UpdateSafetyMarginZ = 1.0f;
    
    SimRegion->World = World;
    SimRegion->Origin = Origin;
    SimRegion->UpdatableBounds =
        AddRadiusTo(Bounds, V3(SimRegion->MaxEntityRadius, SimRegion->MaxEntityRadius, SimRegion->MaxEntityRadius));
    SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatableBounds, //
                                    V3(UpdateSafetyMargin, UpdateSafetyMargin, UpdateSafetyMarginZ));
    
    // TODO(casey): Need to be more specific about entity counts
    SimRegion->MaxEntityCount = 4096;
    SimRegion->EntityCount = 0;
    SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);
    
    world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
    world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
    
    for(s32 ChunkZ = MinChunkP.ChunkZ; //
        ChunkZ <= MaxChunkP.ChunkZ;      //
        ++ChunkZ)
    {
        for(s32 ChunkY = MinChunkP.ChunkY; //
            ChunkY <= MaxChunkP.ChunkY;      //
            ++ChunkY)
        {
            for(s32 ChunkX = MinChunkP.ChunkX; //
                ChunkX <= MaxChunkP.ChunkX;      //
                ++ChunkX)
            {
                world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
                if(Chunk) // На чанке есть сущности | Chunk have entities
                {
                    for(world_entity_block *Block = &Chunk->FirstBlock; //
                        Block;                                          //
                        Block = Block->Next)
                    {
                        for(u32 EntityIndexIndex = 0;           //
                            EntityIndexIndex < Block->EntityCount; //
                            ++EntityIndexIndex)
                        {
                            u32 LowEntityIndex = Block->LowEntityIndex[EntityIndexIndex];
                            low_entity *Low = ModeWorld->LowEntities + LowEntityIndex;
                            if(!IsSet(&Low->Sim, EntityFlag_Nonspatial))
                            {
                                v3 SimSpaceP = GetSimSpaceP(SimRegion, Low);
                                if(EntityOverlapsRectangle(SimSpaceP, Low->Sim.Collision->TotalVolume,
                                                           SimRegion->Bounds))
                                {
                                    AddEntity(ModeWorld, SimRegion, LowEntityIndex, Low, &SimSpaceP);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    return (SimRegion);
}

internal void 
EndSim(sim_region *Region, mode_world *ModeWorld)
{
    // TODO(casey): Maybe don't take a game state here, low entities should be stored
    // in the world??
    
    sim_entity *Entity = Region->Entities;
    for(u32 EntityIndex = 0;
        EntityIndex < Region->EntityCount;
        ++EntityIndex, ++Entity)
    {
        low_entity *Stored = ModeWorld->LowEntities + Entity->StorageIndex;
        
        Assert(IsSet(&Stored->Sim, EntityFlag_Simming));
        Stored->Sim = *Entity;
        Assert(!IsSet(&Stored->Sim, EntityFlag_Simming));
        
        StoreEntityReference(&Stored->Sim.Sword);
        
        // TODO(casey): Save state back to the stored entity, once high entities
        // do state decompression, etc.
        
        world_position NewP = IsSet(Entity, EntityFlag_Nonspatial)
            ? NullPosition()
            : MapIntoChunkSpace(ModeWorld->World, Region->Origin, Entity->P);
        ChangeEntityLocation(ModeWorld, Entity->StorageIndex, Stored, NewP);
        
        if(Entity->StorageIndex == ModeWorld->CameraFollowingEntityIndex)
        {
            world_position NewCameraP = ModeWorld->Camera.P;
            
            NewCameraP.ChunkZ = Stored->P.ChunkZ;
            
#if 0
            if(CameraFollowingEntity.High->P.x > (9.0f*World->TileSideInMeters))
            {
                NewCameraP.AbsTileX += 17;
            }
            if(CameraFollowingEntity.High->P.x < -(9.0f*World->TileSideInMeters))
            {
                NewCameraP.AbsTileX -= 17;
            }
            if(CameraFollowingEntity.High->P.y > (5.0f*World->TileSideInMeters))
            {
                NewCameraP.AbsTileY += 9;
            }
            if(CameraFollowingEntity.High->P.y < -(5.0f*World->TileSideInMeters))
            {
                NewCameraP.AbsTileY -= 9;
            }
#else
            r32 CamZOffset = NewCameraP.Offset_.z;
            NewCameraP = Stored->P;
            NewCameraP.Offset_.z = CamZOffset;
#endif
            
            ModeWorld->Camera.P = NewCameraP;
        }
    }
}

internal b32 
CanCollide(mode_world *ModeWorld, sim_entity *A, sim_entity *B)
{
    b32 Result = false;
    
    if(A != B)
    {
        if(A->StorageIndex > B->StorageIndex)
        {
            sim_entity *Temp = A;
            A = B;
            B = Temp;
        }
        
        if(IsSet(A, EntityFlag_Collides) && IsSet(B, EntityFlag_Collides))
        {
            if(!IsSet(A, EntityFlag_Nonspatial) && !IsSet(B, EntityFlag_Nonspatial))
            {
                // TODO(casey): Property-based logic goes here
                Result = true;
            }
            
            // TODO(casey): BETTER HASH FUNCTION
            u32 HashBucket = A->StorageIndex & (ArrayCount(ModeWorld->CollisionRuleHash) - 1);
            for(pairwise_collision_rule *Rule = ModeWorld->CollisionRuleHash[HashBucket];
                Rule;
                Rule = Rule->NextInHash)
            {
                if((Rule->StorageIndexA == A->StorageIndex) && (Rule->StorageIndexB == B->StorageIndex))
                {
                    Result = Rule->CanCollide;
                    break;
                }
            }
        }
    }
    
    return (Result);
}

internal b32
HandleCollision(mode_world *ModeWorld, sim_entity *A, sim_entity *B)
{
    b32 StopsOnCollision = false;
    
    if(A->Type == EntityType_Sword)
    {
        AddCollisionRule(ModeWorld, A->StorageIndex, B->StorageIndex, false);
        StopsOnCollision = false;
    }
    else
    {
        StopsOnCollision = true;
    }
    
    if(A->Type > B->Type)
    {
        sim_entity *Temp = A;
        A = B;
        B = Temp;
    }
    
    if((A->Type == EntityType_Monstar) && (B->Type == EntityType_Sword))
    {
        if(A->HitPointMax > 0)
        {
            --A->HitPointMax;
        }
    }
    
    // TODO(casey): Stairs
    //            Entity->AbsTileZ += HitLow->dAbsTileZ;
    
    return (StopsOnCollision);
}

internal b32 
CanOverlap(mode_world *ModeWorld, sim_entity *Mover, sim_entity *Region)
{
    b32 Result = false;
    
    if(Mover != Region)
    {
        if(Region->Type == EntityType_Stairwell)
        {
            Result = true;
        }
    }
    
    return (Result);
}

internal void 
HandleOverlap(mode_world *ModeWorld, sim_entity *Mover, sim_entity *Region, r32 dt, r32 *Ground)
{
    if(Region->Type == EntityType_Stairwell)
    {
        *Ground = GetStairGround(Region, GetEntityGroundPoint(Mover));
    }
}

internal b32 
SpeculativeCollide(sim_entity *Mover, sim_entity *Region, v3 TestP)
{
    b32 Result = true;
    
    if(Region->Type == EntityType_Stairwell)
    {
        // TODO(casey): Needs work :)
        r32 StepHeight = 0.1f;
#if 0
        Result = ((AbsoluteValue(GetEntityGroundPoint(Mover).z - Ground) > StepHeight) ||
                  ((Bary.y > 0.1f) && (Bary.y < 0.9f)));
#endif
        v3 MoverGroundPoint = GetEntityGroundPoint(Mover, TestP);
        r32 Ground = GetStairGround(Region, MoverGroundPoint);
        Result = (AbsoluteValue(MoverGroundPoint.z - Ground) > StepHeight);
    }
    
    return (Result);
}

internal b32 
EntitiesOverlap(sim_entity *Entity, sim_entity *TestEntity, v3 Epsilon = V3(0, 0, 0))
{
    b32 Result = false;
    
    for(u32 VolumeIndex = 0;                                    //
        !Result && (VolumeIndex < Entity->Collision->VolumeCount); //
        ++VolumeIndex)
    {
        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
        
        for(u32 TestVolumeIndex = 0;                                        //
            !Result && (TestVolumeIndex < TestEntity->Collision->VolumeCount); //
            ++TestVolumeIndex)
        {
            sim_entity_collision_volume *TestVolume = TestEntity->Collision->Volumes + TestVolumeIndex;
            
            rectangle3 EntityRect = RectCenterDim(Entity->P + Volume->OffsetP, Volume->Dim + Epsilon);
            rectangle3 TestEntityRect = RectCenterDim(TestEntity->P + TestVolume->OffsetP, TestVolume->Dim);
            Result = RectanglesIntersect(EntityRect, TestEntityRect);
        }
    }
    
    return (Result);
}

struct test_wall
{
    r32 X;
    r32 RelX;
    r32 RelY;
    r32 DeltaX;
    r32 DeltaY;
    r32 MinY;
    r32 MaxY;
    v3 Normal;
};

internal void
MoveEntity(mode_world *ModeWorld, sim_region *SimRegion, sim_entity *Entity, r32 dt, move_spec *MoveSpec, v3 ddP,
           v3 WishDir, r32 WishSpeed, r32 Accel)
{
    Assert(!IsSet(Entity, EntityFlag_Nonspatial));
    
    world *World = SimRegion->World;
    
#if 0
    if(MoveSpec->UnitMaxAccelVector)
    {
        r32 ddPLength = LengthSq(ddP);
        if(ddPLength > 1.0f)
        {
            ddP *= (1.0f / SquareRoot(ddPLength));
        }
    }
    
    ddP *= MoveSpec->Speed;
    
    // TODO(casey): ODE here!
    v3 Drag = -MoveSpec->Drag * Entity->dP;
    Drag.z = 0.0f;
    ddP += Drag;
    if(!IsSet(Entity, EntityFlag_ZSupported))
    {
        ddP += V3(0, 0, -9.8f); // NOTE(casey): Gravity!
    }
    
    v3 PlayerDelta = (0.5f * ddP * Square(dt) + Entity->dP * dt);
    Entity->dP = ddP * dt + Entity->dP;
#else
    
    if(Entity->Type == EntityType_Hero)
    {
        int f324324 = 0;
    }
    
    // NOTE(ezexff): Accelerate
    Entity->dP.z = 0.0f;
    
    r32 CurrentSpeed = Inner(Entity->dP, WishDir);
    r32 AddSpeed = WishSpeed - CurrentSpeed;
    
    v3 PlayerDelta = {};
    //if(AddSpeed != 0)
    
    r32 SurfaceFriction = 1.0f;
    r32 AccelSpeed = Accel * dt * WishSpeed * SurfaceFriction;
    
    if (AccelSpeed > AddSpeed)
    {
        AccelSpeed = AddSpeed;
    }
    
    Entity->dP += AccelSpeed * WishDir;
    
    Entity->dP.z = 0.0f;
    
    r32 Spd = Length(Entity->dP);
    
    if(Spd < 1.0f)
    {
        //InvalidCodePath;
    }
    
    PlayerDelta = Entity->dP * dt;
    
    
    if(Entity->Type == EntityType_Hero)
    {
        //Log->Add("v = %f %f %f\n", Entity->dP.x, Entity->dP.y, Entity->dP.z);
    }
    
#endif
    
    // TODO(casey): Upgrade physical motion routines to handle capping the
    // maximum velocity?
    Assert(LengthSq(Entity->dP) <= Square(SimRegion->MaxEntityVelocity));
    
    r32 DistanceRemaining = Entity->DistanceLimit;
    if(DistanceRemaining == 0.0f)
    {
        // TODO(casey): Do we want to formalize this number?
        DistanceRemaining = 10000.0f;
    }
    
    for(u32 Iteration = 0;
        Iteration < 4;
        ++Iteration)
    {
        r32 tMin = 1.0f;
        r32 tMax = 0.0f;
        
        r32 PlayerDeltaLength = Length(PlayerDelta);
        // TODO(casey): What do we want to do for epsilons here?
        // Think this through for the final collision code
        if(PlayerDeltaLength > 0.0f)
        {
            if(PlayerDeltaLength > DistanceRemaining)
            {
                tMin = (DistanceRemaining / PlayerDeltaLength);
            }
            
            v3 WallNormalMin = {};
            v3 WallNormalMax = {};
            sim_entity *HitEntityMin = 0;
            sim_entity *HitEntityMax = 0;
            
            v3 DesiredPosition = Entity->P + PlayerDelta;
            
            // NOTE(casey): This is just an optimization to avoid enterring the
            // loop in the case where the test entity is non-spatial!
            if(!IsSet(Entity, EntityFlag_Nonspatial))
            {
                // TODO(casey): Spatial partition here!
                for(u32 TestHighEntityIndex = 0;
                    TestHighEntityIndex < SimRegion->EntityCount;
                    ++TestHighEntityIndex)
                {
                    sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
                    
                    // TODO(casey): Robustness!
                    r32 OverlapEpsilon = 0.001f;
                    
                    if((IsSet(TestEntity, EntityFlag_Traversable) &&
                        EntitiesOverlap(Entity, TestEntity, OverlapEpsilon * V3(1, 1, 1))) ||
                       CanCollide(ModeWorld, Entity, TestEntity))
                    {
                        for(u32 VolumeIndex = 0;
                            VolumeIndex < Entity->Collision->VolumeCount;
                            ++VolumeIndex)
                        {
                            sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                            
                            for(u32 TestVolumeIndex = 0;
                                TestVolumeIndex < TestEntity->Collision->VolumeCount;
                                ++TestVolumeIndex)
                            {
                                sim_entity_collision_volume *TestVolume =
                                    TestEntity->Collision->Volumes + TestVolumeIndex;
                                
                                v3 MinkowskiDiameter = {TestVolume->Dim.x + Volume->Dim.x,
                                    TestVolume->Dim.y + Volume->Dim.y,
                                    TestVolume->Dim.z + Volume->Dim.z};
                                
                                v3 MinCorner = -0.5f * MinkowskiDiameter;
                                v3 MaxCorner = 0.5f * MinkowskiDiameter;
                                
                                v3 Rel = ((Entity->P + Volume->OffsetP) - (TestEntity->P + TestVolume->OffsetP));
                                
                                // TODO(casey): Do we want an open inclusion at the MaxCorner?
                                //if((Rel.z >= MinCorner.z) && (Rel.z < MaxCorner.z))
                                {
                                    test_wall Walls[] = {
                                        {MinCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y,
                                            MaxCorner.y, V3(-1, 0, 0)},
                                        {MaxCorner.x, Rel.x, Rel.y, PlayerDelta.x, PlayerDelta.y, MinCorner.y,
                                            MaxCorner.y, V3(1, 0, 0)},
                                        {MinCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x,
                                            MaxCorner.x, V3(0, -1, 0)},
                                        {MaxCorner.y, Rel.y, Rel.x, PlayerDelta.y, PlayerDelta.x, MinCorner.x,
                                            MaxCorner.x, V3(0, 1, 0)},
                                    };
                                    
                                    if(IsSet(TestEntity, EntityFlag_Traversable))
                                    {
                                        // NOTE(ezexff): Check entity in Space
                                        
                                        r32 tMaxTest = tMax;
                                        b32 HitThis = false;
                                        
                                        v3 TestWallNormal = {};
                                        for(u32 WallIndex = 0;
                                            WallIndex < ArrayCount(Walls);
                                            ++WallIndex)
                                        {
                                            test_wall *Wall = Walls + WallIndex;
                                            
                                            r32 tEpsilon = 0.001f;
                                            if(Wall->DeltaX != 0.0f)
                                            {
                                                r32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
                                                r32 Y = Wall->RelY + tResult * Wall->DeltaY;
                                                if((tResult >= 0.0f) && (tMaxTest < tResult))
                                                {
                                                    if((Y >= Wall->MinY) && (Y <= Wall->MaxY))
                                                    {
                                                        tMaxTest = Maximum(0.0f, tResult - tEpsilon);
                                                        TestWallNormal = Wall->Normal;
                                                        HitThis = true;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        if(HitThis)
                                        {
                                            tMax = tMaxTest;
                                            WallNormalMax = TestWallNormal;
                                            HitEntityMax = TestEntity;
                                        }
                                    }
                                    else
                                    {
                                        // NOTE(ezexff): Check collision with wall/monstar and etc.
                                        
                                        r32 tMinTest = tMin;
                                        b32 HitThis = false;
                                        
                                        v3 TestWallNormal = {};
                                        for(u32 WallIndex = 0;
                                            WallIndex < ArrayCount(Walls);
                                            ++WallIndex)
                                        {
                                            test_wall *Wall = Walls + WallIndex;
                                            
                                            r32 tEpsilon = 0.1f;
                                            //r32 tEpsilon = 0.001f;
                                            if(Wall->DeltaX != 0.0f)
                                            {
                                                r32 tResult = (Wall->X - Wall->RelX) / Wall->DeltaX;
                                                r32 Y = Wall->RelY + tResult * Wall->DeltaY;
                                                if((tResult >= 0.0f) && (tMinTest > tResult))
                                                {
                                                    if((Y >= Wall->MinY) && (Y <= Wall->MaxY))
                                                    {
                                                        tMinTest = Maximum(0.0f, tResult - tEpsilon);
                                                        TestWallNormal = Wall->Normal;
                                                        HitThis = true;
                                                    }
                                                }
                                            }
                                        }
                                        
                                        // TODO(casey): We need a concept of stepping onto vs. stepping
                                        // off of here so that we can prevent you from _leaving_
                                        // stairs instead of just preventing you from getting onto them.
                                        if(HitThis)
                                        {
                                            v3 TestP = Entity->P + tMinTest * PlayerDelta;
                                            if(SpeculativeCollide(Entity, TestEntity, TestP))
                                            {
                                                tMin = tMinTest;
                                                WallNormalMin = TestWallNormal;
                                                HitEntityMin = TestEntity;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            v3 WallNormal;
            sim_entity *HitEntity;
            r32 tStop;
            if(tMin < tMax)
            {
                tStop = tMin;
                HitEntity = HitEntityMin;
                WallNormal = WallNormalMin;
            }
            else
            {
                tStop = tMax;
                HitEntity = HitEntityMax;
                WallNormal = WallNormalMax;
            }
            
            Entity->P += tStop * PlayerDelta;
            DistanceRemaining -= tStop * PlayerDeltaLength;
            if(HitEntity)
            {
                PlayerDelta = DesiredPosition - Entity->P;
                
                b32 StopsOnCollision = HandleCollision(ModeWorld, Entity, HitEntity);
                if(StopsOnCollision)
                {
                    PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
                    Entity->dP = Entity->dP - 1 * Inner(Entity->dP, WallNormal) * WallNormal;
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }
    
    r32 Ground = 0.0f;
    
    // NOTE(casey): Handle events based on area overlapping
    // TODO(casey): Handle overlapping precisely by moving it into the collision loop?
    {
        // TODO(casey): Spatial partition here!
        for(u32 TestHighEntityIndex = 0;
            TestHighEntityIndex < SimRegion->EntityCount;
            ++TestHighEntityIndex)
        {
            sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;
            if(CanOverlap(ModeWorld, Entity, TestEntity) && EntitiesOverlap(Entity, TestEntity))
            {
                HandleOverlap(ModeWorld, Entity, TestEntity, dt, &Ground);
            }
        }
    }
    
    Ground += Entity->P.z - GetEntityGroundPoint(Entity).z;
    if((Entity->P.z <= Ground) ||
       (IsSet(Entity, EntityFlag_ZSupported) &&
        (Entity->dP.z == 0.0f)))
    {
        Entity->P.z = Ground;
        Entity->dP.z = 0;
        AddFlags(Entity, EntityFlag_ZSupported);
    }
    else
    {
        ClearFlags(Entity, EntityFlag_ZSupported);
    }
    
    if(Entity->DistanceLimit != 0.0f)
    {
        Entity->DistanceLimit = DistanceRemaining;
    }
    
    // TODO(casey): Change to using the acceleration vector
    if((Entity->dP.x == 0.0f) && (Entity->dP.y == 0.0f))
    {
        // NOTE(casey): Leave FacingDirection whatever it was
    }
    else if(AbsoluteValue(Entity->dP.x) > AbsoluteValue(Entity->dP.y))
    {
        if(Entity->dP.x > 0)
        {
            Entity->FacingDirection = 0;
        }
        else
        {
            Entity->FacingDirection = 2;
        }
    }
    else
    {
        if(Entity->dP.y > 0)
        {
            Entity->FacingDirection = 1;
        }
        else
        {
            Entity->FacingDirection = 3;
        }
    }
}