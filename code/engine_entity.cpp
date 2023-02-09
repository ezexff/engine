void RotatePlayerCamera(entity_player *Player, r32 ZAngle, r32 XAngle, r32 Sensitivity, r32 dt)
{
    // по горизонтали
    Player->CameraZRot -= ZAngle * Sensitivity * dt;
    if(Player->CameraZRot < 0)
        Player->CameraZRot += 360;
    if(Player->CameraZRot > 360)
        Player->CameraZRot -= 360;

    // по вертикали
    Player->CameraXRot += XAngle * Sensitivity * dt;
    if(Player->CameraXRot < 0)
        Player->CameraXRot = 0;
    if(Player->CameraXRot > 180)
        Player->CameraXRot = 180;
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
    real32 ddPLength = LengthSq(ddPFromKeys);
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
    v2 Delta = (0.5f * ddP * Square(dt) + Player->dP * dt);

    // Update velocity: v' = at + v
    Player->dP = ddP * dt + Player->dP;

    // Update position
    v2 OldP = V2(Player->Position.x, Player->Position.y);
    v2 NewP = OldP + Delta;
    Player->Position.x = NewP.x;
    Player->Position.y = NewP.y;
}

void OGLSetCameraOnPlayer(entity_player *Player)
{
    glRotatef(-Player->CameraXRot, 1.0f, 0.0f, 0.0f);
    glRotatef(-Player->CameraZRot, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Player->Position.x, -Player->Position.y, -(Player->Position.z + Player->CameraYOffset));
}