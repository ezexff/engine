# C++ Game Engine
Игровой движок, написанный на C++. Проект имеет кросплатформерную структуру (слой платформы и игровой код независимы друг от друга), а так же использует библиотеки [GLFW](https://www.glfw.org/), [ImGui](https://github.com/ocornut/imgui), [Glad](https://github.com/Dav1dde/glad) и [stb_image](https://github.com/nothings/stb).<br><br>
![C++ Game Engine screenshot](https://i.imgur.com/08RFODw.png)
![C++ Game Engine screenshot](https://i.imgur.com/DuFitsj.png)
<details>
<summary>Скриншоты прошлых версий</summary>
<img src="https://i.imgur.com/ue5gFgL.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/BNUPyQj.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/lFqbDXQ.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/icJtm0k.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/vRpIoxd.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/gW81zeb.png" alt="C++ Game Engine screenshot">
<img src="https://i.imgur.com/oZZdn5x.png" alt="C++ Game Engine screenshot">
</details>

## Setup
* Engine: Custom (from scratch without libs - only platform api and graphics api - for now i use some libs like GLFW, stb_image because i want skip routine things with WIN_API and do it later)
* Language: C++ (C coding style with some C++ features like function/operator overloading)
* Libraries: ImGui (developer version only)
* Graphics API: OpenGL
* IDE: Visual Studio Code
* Compile: through .bat with MSVC
* Debug: through .bat with Visual Studio
* Assets
  * Creation 3d-models in Blender
  * Importing 3d-model in Engine from my AssetLoader tool
  * AssetLoader converts 3d-model from any type to my custom type (Assimp)
* Main learning resources
  * [Handmade Hero](https://handmadehero.org/)
  * [Begin End](https://www.youtube.com/channel/UCz29nMCtFP5cuyuLR_0dFkw)
  * [OGLDEV](https://ogldev.org/)

## Engine
* Platform Layer (GLFW)
  * Memory: big memory block from VirtualAlloc()
  * OpenGL (Glad) + GLSL
  * Framerate
    * Locked and Unlocked
    * VSync
  * Window mode: fullscreen or windowed
  * Inputs
    * Keyboard
    * Mouse
  * Resolution independent rendering (getting window dimension)
  * Timers???
  * Debug
    * ImGui: Logging, Diagramming and etc.
    * Performance Counters
* Game Layer
  * Memory
    * WorldArena: static storage
      * GameState
      * Assets initialization
    * TranArena: updates every frame
      * SimRegion
      * GrounBuffer
      * RenderGroup
  * Types: Preprocessor directives
    * Custom names for standart types: int, float and etc.
    * Constant values: int min/max, float min/max, Pi, Tau and etc.
    * Assert(Expression)  
    * Kilobytes, Megabytes, Gigabytes, Terabytes
    * ArrayCount(Array)
  * Math: vectors, rectangles, matrices and scalar operations
  * Intrinsics: scalar operations through processor instructions
  * Inputs
    * Mouse
      * Camera pitch and yaw
      * ImGui
    * Keyboard: hero moving and shooting
  * Timers: models animations update???
  * Entities (игрок и объекты окружения)???
    * Equation of motion
    * Collision detection with gliding and overlapping
    * Transform matrices
  * Camera
  * Assets (импорт 3d-моделей, сгенерированных при помощи Assets Generator)???
  * SimRegion
  * RenderGroup (OpenGL)
    * Resolution independent rendering (FBO based on window dimension)
    * Persprective projection
    * Orthogonal projection
    * Environment Objects Rendering System (cистема рендеринга объектов окружения, объединённых в VBO)
    * Single Static Meshes (VBO одиночных мешей без анимаций)
    * Single Animated Meshes (VBO одиночных анимированных мешей)
    * Multiple Static Meshes (VBO инстансинг множества статичных мешей без анимаций - множество экземпляров мешей с разными матрицами преобразований)
    * Rendering debug info (world origin, lightings positions, collisions borders)
    * Water (FBO+RBO, Water Shader, Clipping Planes, DuDv Texture, Fresnel Effect, Normal Map)
    * Shadows (Shadow Mapping: Depth Map from Depth Buffer)
    * Grass Objects Rendering System (VBO инстансинг травы с облегчённым шейдером и без теней)
    * Seamless Ground Texture Rendering
  * ImGui: Logging, Diagramming and etc. Меню разработчика в ImGui для просмотра и редактирования параметров подсистем движка (Settings, Memory, Camera, Entities, Render, Light Sources, Shadows, Water)
  * Shaders
    * Water: Reflection Texture + Refraction Texture + Materials + Lighting
    * Grass: Materials + Lighting
    * 3d-model: Materials + Lighting + Shadows (Depth Shader, Shadow Shader, Shadow acne, Peter panning, PCF)

## Сборка проекта через MSVC Compiler при помощи build.bat
1.  Укажите во второй строке файла [build.bat](code/build.bat) свой путь до **vcvarsall.bat**
2.  Запуск [build.bat](code/build.bat) скомпилирует исполняемый файл **build/win32_engine.exe**
## Отладка проекта в Visual Studio при помощи debug.bat
1.  Укажите во второй строке файла [debug.bat](code/debug.bat) свой путь до **vcvarsall.bat**
2.  Запуск [debug.bat](code/debug.bat) откроет в Visual Studio исполняемый файл **build/win32_engine.exe**
3.  В обозревателе решений следует изменить рабочий каталог с **build** на **data**
