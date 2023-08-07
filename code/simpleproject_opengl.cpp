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