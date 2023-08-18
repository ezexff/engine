void RotatePlayerCamera(entity_player *Player, r32 ZAngle, r32 XAngle, r32 Sensitivity)
{
    // по горизонтали
    Player->CameraYaw -= ZAngle * Sensitivity;
    if(Player->CameraYaw < 0)
        Player->CameraYaw += 360;
    if(Player->CameraYaw > 360)
        Player->CameraYaw -= 360;

    // по вертикали
    Player->CameraPitch += XAngle * Sensitivity;
    if(Player->CameraPitch < 0)
        Player->CameraPitch = 0;
    if(Player->CameraPitch > 180)
        Player->CameraPitch = 180;

    // по вертикали (inversed)
    Player->CameraPitchInversed -= XAngle * Sensitivity;
    if(Player->CameraPitchInversed < 0)
        Player->CameraPitchInversed = 0;
    if(Player->CameraPitchInversed > 180)
        Player->CameraPitchInversed = 180;
}

internal bool32 TestWall(real32 WallX, real32 RelX, real32 RelY, real32 PlayerDeltaX, real32 PlayerDeltaY, real32 *tMin,
                         real32 MinY, real32 MaxY)
{
    bool32 Hit = false;

    real32 tEpsilon = 0.001f;
    // real32 tEpsilon = 0.01f;
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
}
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

void MovePlayerEOM(entity_player *Player, entity_clip *PlayerClip, v2 ddPFromKeys, r32 Speed, r32 dt)
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
    // v2 OldPlayerPos = V2(Player->Position.x, Player->Position.y);
    // v2 NewPlayerPos = OldPlayerPos + PlayerDelta;
    // Player->Position.x = NewPlayerPos.x;
    // Player->Position.y = NewPlayerPos.y;
    v2 OldPlayerPos = V2(Player->Position.x, Player->Position.y);

    // TODO
    // отрисовать кубы вокруг границ тиррейна и добавить с ними коллизии
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

    // границы тиррейна
    v2 TerrainCenterPos = PlayerClip->CenterPos;
    r32 TerrainSide = PlayerClip->Side;
    // v2 TerrainCenterPos = V2(-10.0f, -10.0f);
    // r32 TerrainSide = 10.0f;

    v2 WallNormal = {};
    u32 HitHighEntityIndex = 0;
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
    v2 DesiredPosition = OldPlayerPos + PlayerDelta;

#if 1
    for(u32 i = 0; i < 4; i++)
    {
        // r32 tMin = 1.0f;

        r32 DiameterW = TerrainSide + Player->Width;
        r32 DiameterH = TerrainSide + Player->Height;

        v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
        v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);

        // v2 MinCorner = -0.5f * V2(TerrainSide, TerrainSide);
        // v2 MaxCorner = 0.5f * V2(TerrainSide, TerrainSide);

        // TODO(me): происходит потеря рязрядов при округление числа, т.к. в числе 0.000... больше разрядов, чем
        // в 50.0...
        // TODO(me): FIX dtForFrame?????
        // При вычитании близких чисел значимые разряды могут потеряться

        r64 tMin = 1.0f;

        r64 RelX = (r64)OldPlayerPos.x - (r64)TerrainCenterPos.x;
        r64 RelY = (r64)OldPlayerPos.y - (r64)TerrainCenterPos.y;

        if(TestWallR64(MinCorner.x, RelX, RelY,      //
                       PlayerDelta.x, PlayerDelta.y, //
                       &tMin, MinCorner.y, MaxCorner.y))
        {
            WallNormal = V2(-1, 0);
            HitHighEntityIndex = 1;
        }

        if(TestWallR64(MaxCorner.x, RelX, RelY,      //
                       PlayerDelta.x, PlayerDelta.y, //
                       &tMin, MinCorner.y, MaxCorner.y))
        {
            WallNormal = v2{1, 0};
            HitHighEntityIndex = 1;
        }

        if(TestWallR64(MinCorner.y, RelY, RelX,      //
                       PlayerDelta.y, PlayerDelta.x, //
                       &tMin, MinCorner.x, MaxCorner.x))
        {
            WallNormal = v2{0, -1};
            HitHighEntityIndex = 1;
        }

        if(TestWallR64(MaxCorner.y, RelY, RelX,      //
                       PlayerDelta.y, PlayerDelta.x, //
                       &tMin, MinCorner.x, MaxCorner.x))
        {
            WallNormal = v2{0, 1};
            HitHighEntityIndex = 1;
        }

        /*r32 tMin = 1.0f;

        v2 Rel = OldPlayerPos - TerrainCenterPos; // расстояние от игрока до стены

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

        OldPlayerPos += (r32)tMin * PlayerDelta;
        Player->Position.x = OldPlayerPos.x;
        Player->Position.y = OldPlayerPos.y;
        if(HitHighEntityIndex == 1)
        {
            Player->dP = Player->dP - 1 * Inner(Player->dP, WallNormal) * WallNormal;
            PlayerDelta = DesiredPosition - OldPlayerPos;
            PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
        }
        else
        {
            break;
        }
    }
#else
    r32 PlayerSide = 0.5f;
    r32 MinPlayerX = OldPlayerPos.x + (0.5f * -PlayerSide);
    r32 MaxPlayerX = OldPlayerPos.x + (0.5f * PlayerSide);
    r32 MinPlayerY = OldPlayerPos.y + (0.5f * -PlayerSide);
    r32 MaxPlayerY = OldPlayerPos.y + (0.5f * PlayerSide);

    vec2 PlayerVertices[] = {
        {MinPlayerX, MinPlayerY},
        {MaxPlayerX, MaxPlayerY},
        {MinPlayerX, MaxPlayerY},
        {MaxPlayerX, MinPlayerY},
    };

    // r32 ClipSide = 10.0f;
    r32 MinClipX = TerrainCenterPos.x + (0.5f * -TerrainSide);
    r32 MaxClipX = TerrainCenterPos.x + (0.5f * TerrainSide);
    r32 MinClipY = TerrainCenterPos.y + (0.5f * -TerrainSide);
    r32 MaxClipY = TerrainCenterPos.y + (0.5f * TerrainSide);

    r32 RelX = OldPlayerPos.x - TerrainCenterPos.x;

    vec2 WallVerticesA[] = {
        {MinClipX, MinClipY},
        {MinClipX, MaxClipY},
    };
    if(gjk(PlayerVertices, 4, WallVerticesA, 2))
    {
        WallNormal = V2(-1, 0);
        HitHighEntityIndex = 1;
    }

    vec2 WallVerticesB[] = {
        {MaxClipX, MinClipY},
        {MaxClipX, MaxClipY},
    };
    if(gjk(PlayerVertices, 4, WallVerticesB, 2))
    {
        WallNormal = V2(1, 0);
        HitHighEntityIndex = 1;
    }

    vec2 WallVerticesC[] = {
        {MinClipX, MinClipY},
        {MaxClipX, MinClipY},
    };
    if(gjk(PlayerVertices, 4, WallVerticesC, 2))
    {
        WallNormal = V2(0, -1);
        HitHighEntityIndex = 1;
    }

    vec2 WallVerticesD[] = {
        {MinClipX, MaxClipY},
        {MaxClipX, MaxClipY},
    };
    if(gjk(PlayerVertices, 4, WallVerticesD, 2))
    {
        WallNormal = V2(0, 1);
        HitHighEntityIndex = 1;
    }

    // OldPlayerPos += tMin * PlayerDelta;
    OldPlayerPos += PlayerDelta;
    Player->Position.x = OldPlayerPos.x;
    Player->Position.y = OldPlayerPos.y;
    if(HitHighEntityIndex)
    {
        Player->dP = Player->dP - 1 * Inner(Player->dP, WallNormal) * WallNormal;
        // PlayerDelta = DesiredPosition - OldPlayerPos;
        PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
    }
#endif
}

void OGLSetCameraOnPlayer(entity_player *Player)
{
    glRotatef(-Player->CameraPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-Player->CameraYaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Player->Position.x, -Player->Position.y, -Player->Position.z);
}