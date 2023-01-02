//
// NOTE(me): Draw OXYZ Lines
//
internal void OGLDrawLinesOXYZ(v3 Normal, r32 LineWidth, r32 LineMin = -0.5, r32 LineMax = 0.5, r32 Offset = 0.001)
{
    // TODO(me): Add arrows?
    r32 LineVertices[] = {
        LineMin, Offset,  Offset,  // 0 - x
        LineMax, Offset,  Offset,  // 1 - x
        Offset,  LineMin, Offset,  // 0 - y
        Offset,  LineMax, Offset,  // 1 - y
        Offset,  Offset,  LineMin, // 0 - z
        Offset,  Offset,  LineMax  // 1 - z

    };
    r32 LineColors[] = {
        1, 0, 0, // 0 - red
        1, 0, 0, // 1 - red
        0, 1, 0, // 0 - green
        0, 1, 0, // 1 - green
        0, 0, 1, // 0 - blue
        0, 0, 1, // 1 - blue
    };
    glPushMatrix();

    glNormal3f(Normal.x, Normal.y, Normal.z);
    glLineWidth(LineWidth);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, LineVertices);
    glColorPointer(3, GL_FLOAT, 0, LineColors);
    glDrawArrays(GL_LINES, 0, 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glLineWidth(20);
    r32 ArrowVertices[] = {
        LineMax,
        Offset,
        Offset, // 0 - x
        LineMax + 0.25f * LineMax,
        Offset,
        Offset, // 1 - x
        Offset,
        LineMax,
        Offset, // 0 - y
        Offset,
        LineMax + 0.25f * LineMax,
        Offset, // 1 - y
        Offset,
        Offset,
        LineMax, // 0 - z
        Offset,
        Offset,
        LineMax + 0.25f * LineMax // 1 - z

    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, ArrowVertices);
    glColorPointer(3, GL_FLOAT, 0, LineColors);
    glDrawArrays(GL_LINES, 0, 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}

//
// NOTE(me): Draw Square
//
internal void OGLDrawColoredSquare(v4 Color)
{
    r32 SquareVertices[] = {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0};

    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, SquareVertices);

    glColor4f(Color.r, Color.g, Color.b, Color.a);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
}

//
// NOTE(me): Draw cube
//
internal void OGLDrawColoredCube(game_state *GameState)
{
    r32 CubeVertices[] = {
        // front
        0, 0, 1, // 0
        1, 0, 1, // 1
        1, 1, 1, // 2
        0, 1, 1, // 3
        // back
        0, 1, 0, // 4
        1, 1, 0, // 5
        1, 0, 0, // 6
        0, 0, 0, // 7
        // right
        1, 0, 1, // 8, 1
        1, 0, 0, // 9, 6
        1, 1, 0, // 10, 5
        1, 1, 1, // 11, 2
        // left
        0, 0, 0, // 12, 7
        0, 0, 1, // 13, 0
        0, 1, 1, // 14, 3
        0, 1, 0, // 15, 4
        // top
        0, 1, 0, // 16
        1, 1, 0, // 17
        1, 1, 1, // 18
        0, 1, 1, // 19
        // bot
        0, 0, 0, // 20
        1, 0, 0, // 21
        1, 0, 1, // 22
        0, 0, 1, // 23
    };

    GLuint CubeIndices[] = {
        0,  1,  2,  3,  // front - red
        4,  5,  6,  7,  // back - green
        8,  9,  10, 11, // right - blue
        12, 13, 14, 15, // left - purple
        16, 17, 18, 19, // top - yellow
        20, 21, 22, 23  // bot - white
    };
    int32 CubeIndicesCount = ArrayCount(CubeIndices);

    r32 CubeColors[] = {
        1,   0, 0,   1,   0, 0,   1,   0, 0,   1,   0, 0,   // red
        0,   1, 0,   0,   1, 0,   0,   1, 0,   0,   1, 0,   // green
        0,   0, 1,   0,   0, 1,   0,   0, 1,   0,   0, 1,   // blue
        0.5, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, 0.5, 0, 0.5, // purple
        1,   1, 0,   1,   1, 0,   1,   1, 0,   1,   1, 0,   // yellow
        1,   1, 1,   1,   1, 1,   1,   1, 1,   1,   1, 1    // white
    };
    glPushMatrix();

    glScalef(0.5, 0.5, 0.5);
    // glRotatef(RotAngle, RotX, RotY, RotZ);
    // glRotatef(GameState->theta, 1, 0, 0);
    // glRotatef(GameState->theta, 0, 1, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, CubeVertices);
    glColorPointer(3, GL_FLOAT, 0, CubeColors);
    glDrawElements(GL_QUADS, CubeIndicesCount, GL_UNSIGNED_INT, CubeIndices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}