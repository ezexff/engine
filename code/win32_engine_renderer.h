// NOTE(ezexff): Export/import renderer function
#define WIN32_LOAD_RENDERER(name) void name(renderer_frame *Frame, HDC WindowDC)
#define WIN32_LOAD_RENDERER_ENTRY() WIN32_LOAD_RENDERER(Win32LoadRenderer)

typedef WIN32_LOAD_RENDERER(win32_load_renderer);

enum loaded_renderer_name
{
    LoadedRenderer_empty,
    LoadedRenderer_opengl,
    LoadedRenderer_vulkan,
    LoadedRenderer_direct3d,
};