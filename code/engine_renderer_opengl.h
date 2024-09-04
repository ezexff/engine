#if ENGINE_INTERNAL
debug_table *GlobalDebugTable;
app_log *Log;
#endif

struct frame_vertex
{
    v3 Position;
    v2 TexCoords;
};

enum vao_vertex
{
    VertexAttributeIndex_Position,
    VertexAttributeIndex_TexCoord,
};

opengl *Opengl;