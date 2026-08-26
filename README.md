# Tiny AI Game Engine

Windows x64 C++20 engine/editor rebuilt around one shared scene and runtime core.

## Current implementation

- Direct3D 11 hardware renderer with swap chain, depth buffer, HLSL shaders and GPU vertex buffers.
- Shared `Scene` model used by editor and standalone runtime.
- Human-readable, versioned scene serialization with validation.
- Win32 editor shell with hierarchy, inspector, primitive/light creation, selection, duplicate/delete, save/open, Play/Stop and Build.
- Standalone Windows runtime that loads a scene and supports WASD movement for dynamic entities.
- TinyScript compiler/VM foundation with indentation-aware `start:`/`update:` blocks, variables, literals, `say`, `move`, `rotate`, `spawn`, and `destroy` commands, with diagnostics.
- Windows CI builds x64 Release binaries, runs CTest, and uploads a ZIP artifact.

## Build

The person using the packaged engine does not need Visual Studio or CMake. Developers can build with Visual Studio 2022 and CMake:

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release
    ctest --test-dir build -C Release --output-on-failure

## Honest limitations

This rebuild is substantially cleaner than the previous prototype, but it is **not yet the complete feature set requested for a mature beginner engine**. In particular, the current renderer implements lit cubes/planes but not yet full sphere/texture/normal-map/point-light/spot-light/shadow pipelines; the editor viewport does not yet have production transform gizmos or a full asset/script dock; undo/redo and recent-project persistence are not complete; TinyScript still lacks the requested full expression/control-flow language and is not yet wired as a per-entity gameplay runtime; audio and a complete physics solver are absent.

Those omissions are intentionally explicit. A button is not a feature merely because humanity has historically enjoyed putting buttons on things.
