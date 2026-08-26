# Windows distribution

Tiny AI Game Engine targets a zero-install experience for end users.

## Developer package

```text
Tiny AI Game Engine/
├── TinyAIEditor.exe
├── TinyAICompiler/
│   ├── bin/
│   ├── lib/
│   ├── include/
│   └── share/
├── Engine/
├── Templates/
└── Runtime/
```

The release ZIP is assembled by GitHub Actions on a Windows runner. The compiler bundle is built/downloaded during CI rather than committed to the source repository.

## User experience

1. Extract the ZIP.
2. Run `TinyAIEditor.exe`.
3. Create a project from `Templates/`.
4. Build the game from the editor.
5. The editor invokes the compiler from `TinyAICompiler/bin/` using absolute paths inside the extracted package.

No Visual Studio, CMake installation, or separate C++ compiler should be required on the user's machine.

## Important distinction

A compiler is needed to create game executables, but players of an exported game should not need the compiler. Exported games will receive only the runtime files they actually require.