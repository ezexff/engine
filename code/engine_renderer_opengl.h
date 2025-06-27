#if ENGINE_INTERNAL
debug_table *GlobalDebugTable;
#if ENGINE_IMGUI
app_log *Log;
#endif
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