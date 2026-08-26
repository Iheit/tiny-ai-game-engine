# TinyAICompiler

This directory defines the compiler bundle shipped with Tiny AI Game Engine.

The release package is intended to contain a Windows-native MinGW-w64 C++ toolchain so end users can build games without installing Visual Studio, CMake, or another compiler.

The GitHub Actions release job assembles the compiler bundle from a clean Windows runner. The repository does not commit third-party compiler binaries into Git, keeping the source repository small and making the generated release package reproducible.

Expected layout in a release:

- `bin/` - compiler, linker and related executables
- `lib/` - C/C++ runtime libraries used by the toolchain
- `include/` - C/C++ headers
- `share/` - compiler support data
