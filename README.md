# Tiny AI Game Engine

A deliberately small Windows-only C++ 3D game-development starter engine.

## Current prototype

- Win32 window and game loop
- Software 3D rasterizer
- Perspective projection
- Depth buffer
- Vertex colors
- Basic keyboard input
- Rotating 3D cube demo
- No third-party runtime dependencies

## Build locally

Open a **Developer Command Prompt for Visual Studio 2022** in the repository and run:

    build.bat

Then run `build/Tiny3DEngine.exe`.

You can also use CMake:

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release

## Automatic Windows builds

Every push to `main`, pull request targeting `main`, and manual workflow dispatch runs the Windows build on a GitHub-hosted Windows runner. The workflow compiles the Release executable and uploads `Tiny3DEngine-Windows-x64.zip` as a workflow artifact.

GitHub workflow artifacts can be downloaded from the completed workflow run's **Artifacts** section. They are intended for build outputs such as binaries and ZIP packages.

## Controls

- Up Arrow: move camera closer
- Down Arrow: move camera farther away
- Close the window: quit

## Architecture

`src/engine.hpp` contains the small public engine API and software renderer. `src/engine.cpp` implements the Win32 host/window loop. `src/main.cpp` is the sample game.

This is a foundation, not a Unity replacement. Planned engine systems include a proper math/matrix layer, mesh loading, textures, materials, scene entities, transforms, lighting, audio, serialization, an editor, and eventually AI-assisted development tools.
