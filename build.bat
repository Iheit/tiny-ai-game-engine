@echo off
setlocal
if not exist build mkdir build
cl /nologo /std:c++17 /O2 /EHsc /DUNICODE /D_UNICODE src\engine.cpp src\main.cpp /Fe:build\Tiny3DEngine.exe user32.lib gdi32.lib
if errorlevel 1 exit /b 1
echo Built build\Tiny3DEngine.exe
