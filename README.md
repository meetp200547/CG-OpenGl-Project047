# 3D Desktop Setup — OpenGL Computer Graphics Project

A procedural **OpenGL 3.3 Core Profile** 3D workspace scene written in **C++17** with **GLFW**, **GLEW**, and **GLM**—featuring Blinn-Phong illumination, real-time shadow mapping, soft contact shadows, procedural static textures, and a live animated monitor video texture.

---

## Features

- **100% Procedural 3D Modeling (`src/Mesh.cpp`, `src/SceneObjects.cpp`)**
  - Primitive generators (`createCube`, `createQuad`, `createCylinder`, `createSphere`).
  - Beveled & corner-rounded furniture (`createRoundedBox`, `createRoundedRim`) for the golden-oak tabletop, flush bullnose edge-banding, deskmat, floating shelf, and area carpet.
  - Parametric S-curved ergonomic chair backrest & headrest (`createCurvedPanel`).
  - Hierarchical model transformations for multi-part objects (rotating ceiling fan, angled gaming laptop, 5-star caster chair).
- **Procedural & Dynamic Texturing (`src/Texture.cpp`)**
  - Seamless periodic textures for golden-oak wood grain, matte plaster walls, dark slate floor tiles, tufted loop-pile carpet with serged border, brushed stainless steel, and a dark IDE laptop display.
  - Real-time animated video texture streamed every frame via `glTexSubImage2D` on the center monitor.
- **Physically Grounded Lighting & Multi-Layer Shadows (`shaders/`)**
  - **3 Realistic Light Sources**: Wall-mounted linear LED tubelight (`uTubeLight`), dynamic video-tinted monitor screen glow (`uMonitorLight`), and cool laptop IDE screen glow (`uLaptopLight`).
  - **Two-Pass Shadow Mapping**: `4096×4096` depth FBO (`shadow.vert` / `shadow.frag`) with 16-tap rotated anisotropic Poisson Disk PCF soft filtering.
  - **Soft Area & Contact Shadows + AO**: Grounded contact occlusion under desk objects, chair casters, desk legs, shelf items, and room corners, plus procedural micro-bump surface detail.
- **Instant Scene Customization (`src/SceneConfig.h`)**
  - Single-file toggles (`show*`) and color palette (`glm::vec3`) for every object and light in the scene.

---

## Project Structure

```text
├── CMakeLists.txt
├── shaders/
│   ├── scene.vert          # MVP + light-space vertex transformation
│   ├── scene.frag          # Blinn-Phong, area lights, PCF shadows & micro-bump
│   ├── shadow.vert         # Pass-1 depth shader
│   └── shadow.frag
└── src/
    ├── main.cpp            # OpenGL setup, 2-pass render loop, camera & shortcuts
    ├── SceneConfig.h       # Quick visibility toggles & color cheat sheet
    ├── SceneObjects.h/cpp  # 13 modular scene components + drawCustomObjects()
    ├── Mesh.h/cpp          # VAO/VBO/EBO & procedural geometry generators
    ├── Texture.h/cpp       # Procedural textures & live video streaming
    └── Shader.h/cpp        # GLSL shader compiler & uniform helpers
```

---

## Build & Run (Windows / MinGW / CMake)

```powershell
cmake -S . -B build -G Ninja
cmake --build build
.\build\DesktopSetup3D.exe
```

---

## Controls

| Input | Action |
| :--- | :--- |
| **Left Mouse Drag / `WASD` / Arrows** | Orbit camera |
| **Mouse Scroll** | Zoom in / out |
| **`Space`** | Play / Pause monitor video |
| **`0` or `O`** | Add / Remove extra water bottle on desk |
| **`F`** | Toggle wireframe mode |
| **`P`** | Save `screenshot.bmp` |
| **`R`** | Reset camera |
| **`Esc`** | Exit |
