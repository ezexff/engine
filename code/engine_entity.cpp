/*internal bool32 TestWall(real32 WallX, real32 RelX, real32 RelY,   //
                         real32 PlayerDeltaX, real32 PlayerDeltaY, //
                         real32 *tMin, real32 MinY, real32 MaxY)
{
    bool32 Hit = false;

    real32 tEpsilon = 0.0001f;
    if(PlayerDeltaX != 0.0f)
    {
        real32 tResult = (WallX - RelX) / PlayerDeltaX;
        real32 Y = RelY + tResult * PlayerDeltaY;
        if((tResult >= 0.0f) && (*tMin > tResult))
        {
            if((Y >= MinY) && (Y <= MaxY))
            {
                *tMin = Maximum(0.0f, tResult - tEpsilon);
                Hit = true;
            }
        }
    }

    return (Hit);
}

internal b32 TestWallR64(r32 WallX, r64 RelX, r64 RelY,      //
                         r32 PlayerDeltaX, r32 PlayerDeltaY, //
                         r64 *tMin, r32 MinY, r32 MaxY)
{
    bool32 Hit = false;

    r64 tEpsilon = 0.1f;

    r64 WallXR64 = (r64)WallX;
    r64 MinYR64 = (r64)MinY;
    r64 MaxYR64 = (r64)MaxY;

    if(PlayerDeltaX != 0.0f)
    {
        r64 tResult = (WallXR64 - RelX) / PlayerDeltaX;
        r64 Y = RelY + tResult * PlayerDeltaY;
        if((tResult >= 0.0f) && (*tMin > tResult))
        {
            if((Y >= MinYR64) && (Y <= MaxYR64))
            {
                *tMin = Maximum(0.0f, tResult - tEpsilon);
                Hit = true;
            }
        }
    }

    return (Hit);
}*/
/*
internal b32 TestWall(r32 WallX, r32 RelX, r32 RelY,      //
                      r32 PlayerDeltaX, r32 PlayerDeltaY, //
                      r32 *tMin, r32 MinY, r32 MaxY)
{
    b32 Hit = false;

    // r32 tEpsilon = 0.00001f;
    r32 tEpsilon = 0.001f;
    if(PlayerDeltaX != 0.0f)
    {
        r32 tResult = (WallX - RelX) / PlayerDeltaX; // расстояние до стены
        r32 Y = RelY + tResult * PlayerDeltaY;       // Y игрока при столкновении со стеной
        if((tResult >= 0.0f) && (*tMin > tResult))   // проверка расстояния до стены
        {
            if((Y >= MinY) && (Y <= MaxY)) // проверка Y игрока на столкновение со стеной
            {
                *tMin = Maximum(0.0f, tResult - tEpsilon);
                Hit = true;
            }
        }
    }

    return (Hit);
}*/
#if 0
internal void MovePlayerEOM(world *World, entity_player *Player, entity_clip *PlayerClip, //
                            v2 ddPFromKeys, r32 Speed, r32 dt)
{
    // Rigid body dynamics (Динамика жесткого тела): F = d/dt (mv)
    // Physics (Movement): Position = f(t), Velocity = f'(t), Acceleration = f"(t)
    // Example Movement Equation:
    // x  = f(t)  = 5t^2 + 2t + 3 (meters) or (m)
    // x' = f'(t) = 10t + 2 (meters/seconds) or (m/s)
    // x" = f"(t) = 10 = a (meters/seconds^2) or (m/s^2)
    //
    // Example Default Movement Equation with variables (a - acceleration, v - const old velocity, p - const old
    // position): x  = f(t)  = 5t^2 + vt + 3 (where 3 is p - old position) = (a/2)*t^2 + vt + p x' = f'(t) = at + 2
    // (where 2 is v - old velocity) = at + v x" = f"(t) = 10 = a
    //
    // We can get old position with Δt = 0
    // f(0)  = p
    // f'(0) = v
    // f"(0) = a
    //
    // New Movement Equation (Derivative)
    // f(t)  = 1/2*a*t^2 + vt + f(0)
    // f'(t) = at + v
    // f"(t) = a
    //
    // New Movement Equation (Code Integration):
    // x =  p' = (a/2)*t^2 + vt + p (new position)
    // x' = v' = at + v (new velocity)
    // x" = a  = a
    //
    // Vectors (reflection & gliding)
    // v' = refl velocity vector
    // v = velocity vector
    // r = wall perpendicular unit vector (wall vector length = 1)
    // Inner = Dot product
    // v' = v - 2 * Inner(v, r) * r
    //
    // 2 change to 1 in equation for projection vector on wall (gliding)
    // v' = gliding velocity vector
    // v' = v - 1 * Inner(v, r) * r
    //

    // исправление вектора ускорения при движении по диагонали
    r32 ddPLength = LengthSq(ddPFromKeys);
    if(ddPLength > 1.0f)
    {
        ddPFromKeys *= (1.0f / SquareRoot(ddPLength));
    }

    // поворот вектора ускорения в зависимости от направления взгляда игрока
    // r32 Angle = Player->CameraYaw / 180 * Pi32;
    r32 Angle = Player->CameraYaw * Pi32 / 180;
    v2 ddP = {};
    ddP.x = ddPFromKeys.x * Cos(Angle) - ddPFromKeys.y * Sin(Angle);
    ddP.y = ddPFromKeys.x * Sin(Angle) + ddPFromKeys.y * Cos(Angle);

    // мультипликатор ускорения
    ddP *= Speed;

    // добавляем к ускорению Force of Friction (силу трения)
    ddP += -1.0f * Player->dP;

    // New position: p' = (a/2)*t^2 + vt + p
    v2 PlayerDelta = (0.5f * ddP * Square(dt) + Player->dP * dt);

    // Update velocity: v' = at + v
    Player->dP = ddP * dt + Player->dP;

    // Update position
    Player->P = Offset(World, Player->P, PlayerDelta);
    // v2 OldPlayerPos = V2(Player->Position.x, Player->Position.y);
    // v2 NewPlayerPos = OldPlayerPos + PlayerDelta;
    // Player->Position.x = NewPlayerPos.x;
    // Player->Position.y = NewPlayerPos.y;
    // v2 OldPlayerPos = V2(Player->Position.x, Player->Position.y);

    // TODO
    // отрисовать кубы вокруг границ террейна и добавить с ними коллизии
    // высота и ширина на плоскости XY

    // NOTE(me): Minkowski Collision Detection
    /*func collision(a : Shape, b : Shape)->bool
    {
        //
        // shapes a and b collide if their center point
        // is inside the minkowski sum shape
        //
        let m : Shape = minkowski_sum(a, b);
        let p : Point = b.center();
        return m.contains_point(p);
    }*/

    // границы террейна
    // v2 TerrainCenterPos = PlayerClip->CenterPos;
    // r32 TerrainSide = PlayerClip->Side;
    // v2 TerrainCenterPos = V2(-10.0f, -10.0f);
    // r32 TerrainSide = 10.0f;

    // TODO(me):
    /*
    enum sim_entity_flags
    {
        // TODO(casey): Does it make more sense to have the flag be for _non_ colliding entities?
        // TODO(casey): Collides and ZSupported probably can be removed now/soon
        EntityFlag_Collides = (1 << 0),
        EntityFlag_Nonspatial = (1 << 1),
        EntityFlag_Moveable = (1 << 2),
        EntityFlag_ZSupported = (1 << 3),
        EntityFlag_Traversable = (1 << 4),

        EntityFlag_Simming = (1 << 30),
    };

    enum entity_type
    {
        EntityType_Null,

        EntityType_Space,

        EntityType_Hero,
        EntityType_Wall,
        EntityType_Familiar,
        EntityType_Monstar,
        EntityType_Sword,
        EntityType_Stairwell,
    };
    */
    // in camera space
    // v2 TmpPlayerP = GetPosInCameraSpace(World, &Player->P, &Player->P);
    // v2 TmpPlayerClipP = GetPosInCameraSpace(World, &Player->P, &PlayerClip->P);

    // for(u32 i = 0; i < 4; i++)
    //{
    /*r32 tMin = 1.0f;
    v2 WallNormal = {};
    u32 HitHighEntityIndex = 0;

    v2 DesiredPosition = TmpPlayerP + PlayerDelta;

    r32 DiameterW = PlayerClip->Side; //+ Player->Width;
    r32 DiameterH = PlayerClip->Side; //+ Player->Height;

    v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
    v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);

    v2 Rel = TmpPlayerP - TmpPlayerClipP;

    if(TestWall(MinCorner.x, Rel.x, Rel.y,    //
                PlayerDelta.x, PlayerDelta.y, //
                &tMin, MinCorner.y, MaxCorner.y))
    {
        WallNormal = V2(-1, 0);
        HitHighEntityIndex = 1;
    }

    if(TestWall(MaxCorner.x, Rel.x, Rel.y,    //
                PlayerDelta.x, PlayerDelta.y, //
                &tMin, MinCorner.y, MaxCorner.y))
    {
        WallNormal = v2{1, 0};
        HitHighEntityIndex = 1;
        //Log.AddLog("[test] dt=%f\n", dt);
        //Log.AddLog("[test] sqdt=%f\n", Square(dt));
    }

    if(TestWall(MinCorner.y, Rel.y, Rel.x,    //
                PlayerDelta.y, PlayerDelta.x, //
                &tMin, MinCorner.x, MaxCorner.x))
    {
        WallNormal = v2{0, -1};
        HitHighEntityIndex = 1;
    }

    if(TestWall(MaxCorner.y, Rel.y, Rel.x,    //
                PlayerDelta.y, PlayerDelta.x, //
                &tMin, MinCorner.x, MaxCorner.x))
    {
        WallNormal = v2{0, 1};
        HitHighEntityIndex = 1;
    }*/

    // OldPlayerPos += tMin * PlayerDelta;
    // world_position OldPlayerP = Player->P;
    // Player->P = Offset(World, OldPlayerP, tMin * PlayerDelta);
    // Player->P = Offset(World, Player->P, tMin * PlayerDelta);
    // TmpPlayerP += tMin * PlayerDelta;
    /*if(HitHighEntityIndex == 1)
    {
        Player->dP = Player->dP - 1 * Inner(Player->dP, WallNormal) * WallNormal;
        PlayerDelta = DesiredPosition - TmpPlayerP;
        PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
    }
    else
    {
        break;
    }*/
    //}

}
#endif

internal r32 TerrainGetHeight(entity_envobject *Terrain, r32 x, r32 y)
{
    // алгоритм нахождения приблизительной высоты на террейне
    // 1. если позиция камеры вне террейна, то высоту не меняем
    // 2. находим X и Y индексы клетки в массиве Terrain Map
    // 3. находим BaseOffset смещение от нулевой позиции в клетке (Camera.x - X) и (Camera.y - Y)
    // 4. находим первый вес h1 для текущей позиции камеры и позиции, смещённой на 1 по оси x
    // по формуле: h1 = ((1 - BaseOffsetX) * TMap[X][Y].z + BaseOffsetX * TMap[X + 1][Y].z)
    // 5. находим второй вес h2 для текущей позиции камеры, смещённой на 1 по оси y
    // и позиции, смещённой на 1 по осям x и y
    // по формуле: h2 = ((1 - BaseOffsetX) * TMap[X][Y + 1].z + BaseOffsetX * TMap[X + 1][Y + 1].z)
    // 6. находим приблизительную высоту по формуле:
    // Result = (1 - BaseOffsetY) * h1 + BaseOffsetY * h2

    r32 Result;

    // GameState->EnvObjects[0]->Model->Meshes[0]
    v3 *Positions = Terrain->Model->Meshes[0].Positions;

    if(!IsPosOnTerrain(x, y))
    {
        return 0.0f;
    }

    int32 X = (int32)x;
    int32 Y = (int32)y;
    r32 BaseOffsetX = x - X;
    r32 BaseOffsetY = y - Y;

    u32 Index0 = X * TMapH + Y;       // [i][j]
    u32 Index1 = (X + 1) * TMapH + Y; // [i+1][j]
    r32 h1 = ((1 - BaseOffsetX) * Positions[Index0].z + BaseOffsetX * Positions[Index1].z);

    u32 Index2 = X * TMapH + Y + 1;       // [i][j+1]
    u32 Index3 = (X + 1) * TMapH + Y + 1; // [i+1][j+1]
    r32 h2 = ((1 - BaseOffsetX) * Positions[Index2].z + BaseOffsetX * Positions[Index3].z);

    Result = (1 - BaseOffsetY) * h1 + BaseOffsetY * h2;

    return (Result);
}

internal m4x4 *CreateInstancingTransformMatrices(memory_arena *WorldArena,  //
                                                 entity_envobject *Terrain, //
                                                 u32 Count,                 //
                                                 v3 SMM,                    // Scale rand() Min, Max, Precision
                                                 v3 Rotate,                 // Rotate X, Y, Z
                                                 v3 RXMM,                   // Rotate X rand() Min, Max, Precision
                                                 v3 RYMM,                   // Rotate Y rand() Min, Max, Precision
                                                 v3 RZMM)                   // Rotate Z rand() Min, Max, Precision
{
    m4x4 *Result = PushArray(WorldArena, Count, m4x4);

    for(u32 i = 0; i < Count; i++)
    {
        v3 TranslationVec = V3(0, 0, 0);
        TranslationVec.x = (r32)(rand()) / (r32)(RAND_MAX / (TMapW - 2));
        TranslationVec.y = (r32)(rand()) / (r32)(RAND_MAX / (TMapH - 2));
        TranslationVec.z = TerrainGetHeight(Terrain, TranslationVec.x, TranslationVec.y);
        m4x4 TranslationM = Translation(TranslationVec);

        r32 RotX = DebugGetRandomNumberR32(RXMM.x, RXMM.y, (u32)RXMM.z);
        r32 RotY = DebugGetRandomNumberR32(RYMM.x, RYMM.y, (u32)RYMM.z);
        r32 RotZ = DebugGetRandomNumberR32(RZMM.x, RZMM.y, (u32)RZMM.z);
        m4x4 RotationM = XRotation(Rotate.x) * YRotation(Rotate.y) * ZRotation(Rotate.z) //
                         * XRotation(RotX) * YRotation(RotY) * ZRotation(RotZ);

        r32 Scale = DebugGetRandomNumberR32(SMM.x, SMM.y, (u32)SMM.z);
        // v3 ScaleVec = V3(Scale, Scale, Scale);
        // m4x4 ScalingM = Scaling(ScaleVec);
        m4x4 ScalingM = Scaling(Scale);

        Result[i] = TranslationM * RotationM * ScalingM;
        Result[i] = Transpose(Result[i]); // opengl to glsl format
    }

    return (Result);
}

// TODO(me): переделать всё что выше
inline move_spec //
DefaultMoveSpec(void)
{
    move_spec Result;

    Result.UnitMaxAccelVector = false;
    Result.Speed = 1.0f;
    Result.Drag = 0.0f;

    return (Result);
}
