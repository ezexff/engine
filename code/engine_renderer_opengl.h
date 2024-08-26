#if ENGINE_INTERNAL
debug_table *GlobalDebugTable;
app_log *Log;
#endif

struct frame_vertex
{
    v3 Pos;
    v2 TexCoord;
};

enum vao_vertex
{
    VertexAttributeIndex_Position,
    VertexAttributeIndex_TexCoord,
};

opengl *Opengl;