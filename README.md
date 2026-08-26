# Tiny AI Game Engine

A Windows-first C++ 3D game-development engine prototype designed for simple end-user distribution.

## End-user goal

A release ZIP should run without Visual Studio, CMake, or a separately installed compiler. Launch `TinyAIEditor.exe`, edit a scene, and use **Build Game** to create a standalone runtime project.

## Current prototype

- Windows editor executable
- 3D viewport with animated cube rendering
- Add Cube, Save, Load, and Build Game controls
- Human-readable `project.tiny` scene format
- Standalone `TinyAIRuntime.exe`
- Starter project template
- GitHub Actions Windows x64 packaging
- No third-party runtime DLLs required by the current prototype

## Automatic builds

Every push to `main`, pull request targeting `main`, and manual workflow dispatch uses a GitHub-hosted Windows 2022 runner. The workflow builds Release binaries and packages `TinyAI-Game-Engine-Windows-x64.zip` as an artifact.

## Important compiler limitation

The current prototype does **not** ship a C++ compiler or compile arbitrary user C++ code. The editor's Build Game operation packages the engine runtime plus the scene data. This keeps the end-user package simple and avoids pretending that a full C++ toolchain is a tiny redistributable file.

The intended future solution is an engine-owned scripting/build layer that can be distributed with the editor, allowing users to create gameplay logic and export a standalone Windows game without installing developer software.

## Local development

Contributors need a Windows C++17 compiler and CMake. End users of a release do not.

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Controls

Editor: use the buttons at the top. The viewport continuously animates the sample scene.

Runtime: close the window to quit.

## Roadmap

1. Scene hierarchy and object inspector
2. Transform editing and camera controls
3. Mesh and texture assets
4. Materials and lighting
5. Collision and basic physics
6. Audio
7. Engine-owned gameplay scripting
8. One-click standalone game export
9. Packaging and versioned releases
10. AI-assisted scene and gameplay authoring
