void RotatePlayerCamera(entity_player *Player, r32 ZAngle, r32 XAngle, r32 Sensitivity)
{
    // по горизонтали
    Player->CameraZRot -= ZAngle * Sensitivity;
    if(Player->CameraZRot < 0)
        Player->CameraZRot += 360;
    if(Player->CameraZRot > 360)
        Player->CameraZRot -= 360;

    // по вертикали
    Player->CameraXRot += XAngle * Sensitivity;
    if(Player->CameraXRot < 0)
        Player->CameraXRot = 0;
    if(Player->CameraXRot > 180)
        Player->CameraXRot = 180;
}

internal b32 TestWall(r32 WallX, r32 RelX, r32 RelY,      //
                      r32 PlayerDeltaX, r32 PlayerDeltaY, //
                      r32 *tMin, r32 MinY, r32 MaxY)
{
    b32 Hit = false;

    r32 tEpsilon = 0.1f;
    if(PlayerDeltaX != 0.0f)
    {
        r32 tResult = (WallX - RelX) / PlayerDeltaX;
        r32 Y = RelY + tResult * PlayerDeltaY;
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

void MovePlayerEOM(entity_player *Player, v2 ddPFromKeys, r32 Speed, r32 dt)
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
    r32 Angle = Player->CameraZRot / 180 * Pi32;
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
    v2 NewPlayerPos = V2(Player->Position.x, Player->Position.y);

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
    v2 TerrainCenterPos = V2(-10.0f, -10.0f);
    r32 TerrainSide = 10.0f;

    r32 PlayerWidth = 0.5f;
    r32 PlayerHeight = 0.5f;

    for(u32 i = 0; i < 4; i++)
    {
        r32 tMin = 1.0f;
        v2 WallNormal = {};
        u32 HitHighEntityIndex = 0;

        v2 DesiredPosition = NewPlayerPos + PlayerDelta;

        r32 DiameterW = TerrainSide + PlayerWidth;
        r32 DiameterH = TerrainSide + PlayerHeight;

        v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
        v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);

        v2 Rel = NewPlayerPos - TerrainCenterPos;

#if 1
        if(TestWall(MinCorner.x, Rel.x, Rel.y,           //
                    PlayerDelta.x, PlayerDelta.y, &tMin, //
                    MinCorner.y, MaxCorner.y))
        {
            WallNormal = V2(-1, 0);
            HitHighEntityIndex = 1;
        }

        if(TestWall(MaxCorner.x, Rel.x, Rel.y,           //
                    PlayerDelta.x, PlayerDelta.y, &tMin, //
                    MinCorner.y, MaxCorner.y))
        {
            WallNormal = v2{1, 0};
            HitHighEntityIndex = 1;
        }

        if(TestWall(MinCorner.y, Rel.y, Rel.x,           //
                    PlayerDelta.y, PlayerDelta.x, &tMin, //
                    MinCorner.x, MaxCorner.x))
        {
            WallNormal = v2{0, -1};
            HitHighEntityIndex = 1;
        }

        if(TestWall(MaxCorner.y, Rel.y, Rel.x,           //
                    PlayerDelta.y, PlayerDelta.x, &tMin, //
                    MinCorner.x, MaxCorner.x))
        {
            WallNormal = v2{0, 1};
            HitHighEntityIndex = 1;
        }
#endif
        NewPlayerPos += tMin * PlayerDelta;
        Player->Position.x = NewPlayerPos.x;
        Player->Position.y = NewPlayerPos.y;
        if(HitHighEntityIndex == 1)
        {
            Player->dP = Player->dP - 1 * Inner(Player->dP, WallNormal) * WallNormal;
            PlayerDelta = DesiredPosition - NewPlayerPos;
            PlayerDelta = PlayerDelta - 1 * Inner(PlayerDelta, WallNormal) * WallNormal;
        }
        else
        {
            break;
        }

    }
}

void OGLSetCameraOnPlayer(entity_player *Player)
{
    glRotatef(-Player->CameraXRot, 1.0f, 0.0f, 0.0f);
    glRotatef(-Player->CameraZRot, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Player->Position.x, -Player->Position.y, -(Player->Position.z + Player->CameraYOffset));
}