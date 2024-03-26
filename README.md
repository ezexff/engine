# C++ Game Engine
Game engine that i wrote when learned low-level programming

## Setup
* **Engine:** Custom. Low-level programming. From scratch and without libs. Minimum number of dependencies. Only platform api and graphics api
* **Language:** C++ but "C coding style" with some C++ features like function and operator overloading. That can help understand what compiler does with code and how code works on CPU in ASM instructions
* **IDE:** [4Coder](https://4coder.net/)
* **Graphics API:** OpenGL
* **Libraries:** Only for debug version [ImGui](https://github.com/ocornut/imgui)
* **Debug:** Visual Studio
* **Build:** through `.bat` with MSVC
  * Debug version with ImGui
  * Release version without CRT and ImGui
* **Asset:** [Asset Builder](https://github.com/ezexff/asset-builder) pack assets into `.eab` (engine asset builder) file. Engine import assets from `.eab` files
* Main learning and inspiration resources
  * [Handmade Hero](https://handmadehero.org/)
  * [Begin End](https://www.youtube.com/channel/UCz29nMCtFP5cuyuLR_0dFkw)
  * [OGLDEV](https://ogldev.org/)

## Engine
<details>
<summary>Platform-independent code</summary>

* ### Types
  * Custom names for C data types (int, float and etc.)
  * Constant values
    * Min and Max for int, float and etc.
    * Pi, Tau
  * Preprocessor directives
    * Assert()
    * InvalidCodePath
    * InvalidDefaultCase
    * Kilobytes(), Megabytes(), Gigabytes(), Terabytes()
    * ArrayCount()
  * No CRT
    * _fltused
    * memset()
  * Vector declarations (v2, v2s, v3, v3s, v4)
  * Rectangle declarations (rectangle2, rectangle3)
  * Matrix declarations (m4x4, m4x4_inv)
* ### Structs
  * Memory
    * `game_memory`
  * Renderer
    * `renderer_frame`
    * `camera`
    * `opengl`
  * Audio
    * `game_sound_output_buffer`
    * `loaded_sound`
  * Input
    * `platform_file_handle`
    * `platform_file_group`
    * `game_button_state`
    * `game_controller_input`
    * `game_input_mouse_button`
    * `game_input`
  * Other
    * `platform_api` pointers to win32 functions
      * Read data from files
        * GetAllFilesOfTypeBegin
        * GetAllFilesOfTypeEnd
        * OpenNextFile
        * ReadDataFromFile
        * FileError
  * Debug
    * `imgui` context, variables for windows visibility and etc.
* ### Functions
  * `SafeTruncateUInt64()`
  * `GetController()`
* ### Export/import function declarations
  * Game
    * `UpdateAndRender()`
    * `GetSoundSamples()`
  * Renderer
    * `BeginFrame()`
    * `EndFrame()`
---
  
</details>

<details>
<summary>Win32 code</summary>


* ### Memory
  * Alloc big memory block with VirtualAlloc()
  * Init pointers to platform API functions
* ### Renderer (OpenGL)
  * Load renderer from .dll
  * Init frame object
  * Init pointers to shader functions
  * Update Frame object at every frame with `BeginFrame()` and `EndFrame()`
* ### Audio
  * WASAPI
  * ~~DirectSound~~
* ### Input
  * Keyboard
  * Mouse (keys, wheel, cursor)
  * Read files
    * GetAllFilesOfType
    * OpenNextFile
    * FileError
  * ~~XInput gamepad~~
* ### Other
  * Timer
  * Fps lock
  * Toggle fullscreen
* ### Debug
  * ImGui log app
  * ImGui win32 window
    * Settings
      * Demo window
      * Window mode
      * Fps lock
    * Renderer
      * Frame info
      * OpenGL info
    * Audio
      * WASAPI info
    * Input
      * Cheatsheet of implemented inputs
---

</details>

<details>
<summary>Game code</summary>

* ### Memory
  * ConstArena static storage
  * TranArena updates every frame
* ### Renderer (OpenGL)
  * Frame through camera with perspective projection
  * Push buffer
    * Clear
    * RectOnGround
    * RectOutlineOnGround
    * BitmapOnGround
  * Frame through shader with effects (blur, emboss, grayscale, inverse, sepia)
  * Skybox
  * Ground chunks
* ### Audio (sound mixer)
  * Play sound
  * Test sine wave
    * Change tone volume
    * Change tone hz
* ### Mode
  * Test
    * Render
      * Clear screen
      * Rectangle on ground
      * Load shader from file
      * Compile shader
      * Link shader program
  * World
    * Input
      * Movement
      * Camera
      * Attack
    * Hash-based world storage
    * Entities (hero, wall, monstar, sword)
    * Sim region  
      * Update entities in area around camera (movement, collisions and etc.)
    * Render
      * Updatable entities
      * Bitmap at ground chunks
* ### Other
  * Math
    * Intrinsics
      * Scalar operations through processor instructions
      * SIMD intrinsics for trigonometric math functions (SVML)
    * Scalar operations
    * Vector operators and operations
    * Rectangle operators and operations
    * Matrix operators and operations
  * Multithreaded task workers
  * Asset
    * Reading EAB files
    * Load bitmaps and sounds through task workers
* ### Debug
  * ImGui log app
  * ImGui game window
    * Sim region window visibility
    * Memory
      * ConstArena info
      * TranArena info
    * Audio
      * Playing sound info
      * Test sine wave
        * Tone hz
        * Tone volume
      * Play 1st loaded sound
    * Frame
      * Push buffer info
      * Camera position and angle info
      * Preview for color and depth textures
      * Effects (blur, emboss, grayscale, inverse, sepia)
      * Live shaders editor window visibility
      * Variables info (IDs - textures, shaders, programs)
    * Input
      * Mouse pos and delta
      * Log mouse input
      * Log keyboard input
    * Threads
      * Current state info for 4 workers
    * Assets
      * Created memory blocks info
      * Count for every type of asset
      * EAB file tree
        * Show selected bitmap in preview window
        * Play selected sound
    * Mode
      * Change game mode
      * Change camera position
      * Background fill color
  * Bitmap preview window
  * Shaders editor window
  * Sim region window
    * Origin, bounds and etc. info
    * List of updatable entities

</details>

## Сборка проекта через MSVC Compiler при помощи build.bat
1. Укажите в третьей строке файла [build.bat](code/build.bat) свой путь до `vcvarsall.bat`
2. Запуск [build.bat](code/build.bat) скомпилирует исполняемый файл `build/win32_engine.exe`
## Отладка проекта в Visual Studio при помощи debug.bat
1. Запуск [debug.bat](code/debug.bat) откроет в Visual Studio исполняемый файл `build/win32_engine.exe`
2. В обозревателе решений следует изменить рабочий каталог с `build` на `data`
> **NOTE:** [debug.bat](code/debug.bat) необходимо запускать через CLI после `vcvarsall.bat`

## Legacy
<details>
<summary>Changes</summary>

```
2024.03.26
 + added font to asset builder and engine
 + added font preview in ImGui EAB tree
 + in ImGui asset list Text replaced by BulletText

2024.03.20
 + added sim region, world, entities from old engine
 + added functions for rectangle types
 + added ground chunk buffers and fill ground chunks filling
 + fixed collision when hit monstar

2024.02.24
 + added IsDown() and WasPressed() input functions
 + fixed Frame s32 Width, Height replaced to v2s Dim
 + added MoveCamera() method - camera in frame from mode camera
 + changing camera pos and angle from game modes
 + added string type
 + added name and size in filehandle
 + improved OpenglCompileShader() and OpenglLinkProgram()
 + added rendering through texture (ColorTexture, DepthTexture, VAO, VBO, FBO)
 + added vert and frag shader screen effects to frame (blur, emboss, grayscale, inverse, sepia)
 + improved structure of ImGui debug windows
 + added mouse delta for 3d camera
 + improved 3d camera and movement in World mode
 + some ImGui Text replaced by BulletText
 + added task work queues
 + added assets loading from .eab files through thread task workers
 + added EAB tree with bitmaps and sounds
 + added preview window for bitmaps
 + added skybox
 + skybox optimized
 + fixed camera angle
 + fixed dtForFrame in movement code
 + improved shaders loading code
 + fixed bug in platform file i/o code
 + reworked code for load shader texts
 + added bitmap on ground to push buffer renderer
```

</details>

<details>
<summary>Screenshots</summary>

### 23.03.2024
<img src="https://i.imgur.com/cGDpNIo.png" alt="5 - 23.03.2024">
<img src="https://i.imgur.com/wWGq702.png" alt="4 - 23.03.2024">
<img src="https://i.imgur.com/aPd24OJ.png" alt="3 - 23.03.2024">
<img src="https://i.imgur.com/Xwltila.png" alt="2 - 23.03.2024">
<img src="https://i.imgur.com/dF5uV3q.png" alt="1 - 23.03.2024">

### 18.02.2024
<img src="https://i.imgur.com/CTprHxF.png" alt="2 - 18.02.2024">

### 16.02.2024
<img src="https://i.imgur.com/P8EAYty.png" alt="1 - 16.02.2024">

### 26.12.2023
<img src="https://i.imgur.com/ScSqF2k.png" alt="19 - 26.12.2023">
<img src="https://i.imgur.com/mNnF25d.png" alt="18 - 26.12.2023">
<img src="https://i.imgur.com/66SAlev.png" alt="17 - 26.12.2023">
<img src="https://i.imgur.com/m0dVLfg.png" alt="16 - 26.12.2023">
<img src="https://i.imgur.com/oRULiTy.png" alt="15 - 26.12.2023">
<img src="https://i.imgur.com/2WkdFRJ.png" alt="14 - 26.12.2023">
<img src="https://i.imgur.com/3KfLifH.png" alt="13 - 26.12.2023">
<img src="https://i.imgur.com/SGAMw7X.png" alt="12 - 26.12.2023">
<img src="https://i.imgur.com/yoag0Nv.png" alt="11 - 26.12.2023">
<img src="https://i.imgur.com/qwDYdQb.png" alt="10 - 26.12.2023">

### 26.08.2023
<img src="https://i.imgur.com/08RFODw.png" alt="9 - 26.08.2023">
<img src="https://i.imgur.com/DuFitsj.png" alt="8 - 26.08.2023">

### 21.08.2023
<img src="https://i.imgur.com/ue5gFgL.png" alt="7 - 21.08.2023">

### 19.08.2023
<img src="https://i.imgur.com/BNUPyQj.png" alt="6 - 19.08.2023">

### 12.08.2023
<img src="https://i.imgur.com/lFqbDXQ.png" alt="5 - 12.08.2023">
<img src="https://i.imgur.com/icJtm0k.png" alt="4 - 12.08.2023">
<img src="https://i.imgur.com/vRpIoxd.png" alt="3 - 12.08.2023">
<img src="https://i.imgur.com/gW81zeb.png" alt="2 - 12.08.2023">

### 06.01.2023
<img src="https://i.imgur.com/oZZdn5x.png" alt="1 - 06.01.2023">

</details>

> **NOTE:** Current game engine version is 3

[1st version](https://github.com/ezexff/learning_opengl)

<details>
<summary>2nd version description</summary>

* ### Platform Layer (GLFW)
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
* ### Game Layer
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

</details>
