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