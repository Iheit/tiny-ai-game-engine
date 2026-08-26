# Tiny AI Game Engine

Tiny AI Game Engine is a Windows-first C++ 3D game engine focused on a simple editor, a readable gameplay language, and self-contained Windows builds.

## What exists now

### Editor

- Startup Hub with New Project and Open Project
- Conventional game-editor layout
- Scene hierarchy
- 3D viewport with perspective camera
- RMB orbit, Shift+LMB pan, mouse-wheel zoom
- Cube, sphere and plane primitives
- Selection, duplication and deletion
- Transform inspector for position, rotation and scale
- Scene save/load
- Asset folder browser
- Built-in TinyScript editor
- TinyScript compile validation
- Play/Stop mode with basic gravity
- Build Game package operation

### Runtime

`TinyAIRuntime.exe` is a standalone Windows runtime. It loads the human-readable scene format, renders the scene with filled 3D primitives, provides first-person-style camera movement, and runs a small physics update for dynamic objects.

### TinyScript

TinyScript is the intended gameplay language. It is designed to be much easier to learn than C++ and is compiled to bytecode for a small VM.

Example:

```tiny
speed = 5
health = 100

start:
    say "Player spawned"

update:
    if key W
        move 0 0 speed * time
```

The language specification is in `tinyscript/SPEC.md`. The current compiler is deliberately small and is being expanded incrementally. Unsupported syntax produces diagnostics instead of silently doing something else.

## Build and distribution

GitHub Actions uses a Windows 2022 runner to compile the editor, runtime and TinyScript smoke test. A successful run creates the `TinyAI-Game-Engine-Windows-x64` artifact containing the Windows editor, runtime, source, templates and documentation.

The release package is intended to require **no Visual Studio, CMake, or separately installed compiler** for the person using the engine.

## Build Game

The editor's Build button creates a `build` directory inside the project and packages a copy of the standalone runtime with the current scene. This is the foundation for a true one-click game exporter. Gameplay-script compilation and asset bundling are still being expanded, so this is not yet equivalent to a mature commercial engine exporter.

## Project layout

```text
MyGame/
├── project.tinyproj
├── assets/
├── scripts/
└── build/
```

## Developer build

Windows developers can build with Visual Studio 2022 and CMake:

```text
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The repository also contains a small TinyScript smoke test used by CI.

## Current limitations

This is not yet a replacement for Unreal, Unity, Godot, or another mature engine. Rendering is intentionally lightweight and currently uses a Windows software/GDI path. Asset import, textures, modern lighting, robust physics, audio, complete TinyScript semantics, component authoring, and production-grade export are still under development.

Those limitations are documented deliberately. A button labelled `Build` is not considered a feature merely because it exists.
