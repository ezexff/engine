# C++ Game Engine

Custom. Low-level programming. From scratch and without libs. Without CRT. Minimum number of dependencies. Only platform api and graphics api

## Setup

* **Language:** C++ but "C coding style" with some C++ features like function and operator overloading
* **Libraries:** No!
* **Graphics API:** OpenGL
* **IDE:** [4Coder](https://4coder.net/)
* **Debug:** [The RAD Debugger Project](https://github.com/EpicGamesExt/raddebugger) and NVIDIA Nsight Graphics
* **Asset:** Engine import assets from `.eab` files that created by [Asset Builder](https://github.com/ezexff/asset-builder)

## MSVC compilation

1. Set third line [build.bat](code/build.bat) with your own path to `vcvarsall.bat`
2. Run [build.bat](code/build.bat) will compile `build/win32_engine.exe`

## Engine

<details>
<summary>Short changelog</summary>

* **2025:**
  * Profiler
  * Custom UI from scratch
    * Core and widgets code separation
    * Autolayout algorithm
    * Caching
    * Multiple windows with sort
    * Dragging
    * Resize
    * Padding
    * Unique node ID
    * Node interaction
    * Scroll bars
    * Format string
    * Text clipping
    * Window scrolling with mouse wheel
    * Instanced fast text rendering
  * ImGui disable preprocessor switch
  * Debug collation throught custom UI
  * Simple 2d physics
    * Transforms 2d polygons
    * Circle and polygons collision detection+resolution
    * Realistic collision response
    * Static entities
    * Two game modes with physics
    * Physics push buffer and rendering without flipped y
    * Contact points to every entity type and AABB test
    * Resolve collision with rotation and friction
  * Avg stat and ms graph
* **2024:**
  * Changed project structure (win32.exe, engine.dll, renderer.dll, data.eab)
  * Viewer in ImGui for .eab bitmaps
  * Audio through WASAPI
  * Asset system (bitmaps, sounds and fonts)
  * OpenGL shader support with live shader editing
  * Fonts rendering
  * Half-Life 2 movement 
  * Procedurally generated terrain chunks with pits and hills 
  * Directional light and shadows
  * Water entity
  * New rendering system with flags
  * Debug collation
  * Debug info in ImPlot graphs
  * Ortho push buffer for on screen rendering
* **2023:** Started writing [3rd game engine](https://github.com/ezexff/engine) without libs. Using ImGui only in debug build version
  * New rendering system through push buffer
  * Rendering static meshes throught one big VBO and animated meshes throught multiple VBOs
  * Grass instancing
  * Line segment intersection collisions
  * Fps lock
  * ImGui dev menu
  * Rendering debug elements (lights pos, collision box and etc.)
  * Water
  * Shadow mapping
  * Sim Region implementation from hmh
  * Seamless texture ground rendering
  * Seamless terrain chunk mesh rendering
  * Work queue for multithreading
  * Audio through DirectSound
  * Started Asset system implementation
* **2022:**
  * [Learning advanced OpenGL techniques](https://github.com/ezexff/learning_opengl)
  * [Asset Builder](https://github.com/ezexff/assets_generator)
* **2021:** Started writing [2nd game engine](https://github.com/ezexff/learning_opengl) with GLFW and stb_image
* **2020:** Started learning [Handmade Hero](https://handmadehero.org/) and writing 1st game engine (watched 87 episodes)

</details>

> [!TIP]
> More detailed info about changes in repository Commits

<details>
<summary>Videos</summary>

### 2023-2025

[![2025 Engine](https://img.youtube.com/vi/2owUrXn3sZ4/0.jpg)](https://www.youtube.com/watch?v=2owUrXn3sZ4)

### 2020-2022

[![Simple Looting Game](https://img.youtube.com/vi/Fz5yPYOjlAI/0.jpg)](https://www.youtube.com/watch?v=Fz5yPYOjlAI)

[![Some Rendering Techniques](https://img.youtube.com/vi/4fBstHXsY60/0.jpg)](https://www.youtube.com/watch?v=4fBstHXsY60)

[![Equation of Motion](https://img.youtube.com/vi/5f_eacPf-V8/0.jpg)](https://www.youtube.com/watch?v=5f_eacPf-V8)

</details>

<details>
<summary>Screenshots</summary>

### 2025

#### 
<img src="https://i.imgur.com/p97jh6i.png">
<img src="https://i.imgur.com/Wc1NLMI.png">
<img src="https://i.imgur.com/1SvibFp.png">
<img src="https://i.imgur.com/pYjnIlF.png">

### 2024

<img src="https://i.imgur.com/cGDpNIo.png">
<img src="https://i.imgur.com/wWGq702.png">
<img src="https://i.imgur.com/aPd24OJ.png">
<img src="https://i.imgur.com/Xwltila.png">
<img src="https://i.imgur.com/dF5uV3q.png">
<img src="https://i.imgur.com/CTprHxF.png">
<img src="https://i.imgur.com/P8EAYty.png">

### 2023

<img src="https://i.imgur.com/ScSqF2k.png">
<img src="https://i.imgur.com/mNnF25d.png">
<img src="https://i.imgur.com/66SAlev.png">
<img src="https://i.imgur.com/m0dVLfg.png">
<img src="https://i.imgur.com/oRULiTy.png">
<img src="https://i.imgur.com/2WkdFRJ.png">
<img src="https://i.imgur.com/3KfLifH.png">
<img src="https://i.imgur.com/SGAMw7X.png">
<img src="https://i.imgur.com/yoag0Nv.png">
<img src="https://i.imgur.com/qwDYdQb.png">
<img src="https://i.imgur.com/08RFODw.png">
<img src="https://i.imgur.com/DuFitsj.png">
<img src="https://i.imgur.com/ue5gFgL.png">
<img src="https://i.imgur.com/BNUPyQj.png">
<img src="https://i.imgur.com/lFqbDXQ.png">
<img src="https://i.imgur.com/icJtm0k.png">
<img src="https://i.imgur.com/vRpIoxd.png">
<img src="https://i.imgur.com/gW81zeb.png">
<img src="https://i.imgur.com/oZZdn5x.png">

</details>

[Learning notes](https://github.com/ezexff/learning-notes)

#### Main learning and inspiration resources

* [Handmade Hero](https://handmadehero.org/)
* OpenGL
  * [Begin End](https://www.youtube.com/@beginend95)
  * [OGLDEV](https://ogldev.org/)
* Physics
  * [Two-Bit Coding](https://www.youtube.com/@two-bitcoding8018)
  * [Rigid Body Dynamics](https://www.chrishecker.com/Rigid_Body_Dynamics)