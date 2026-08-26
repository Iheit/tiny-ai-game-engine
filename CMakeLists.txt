cmake_minimum_required(VERSION 3.20)
project(TinyAIGameEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WIN32_EXECUTABLE TRUE)

add_library(TinyScript STATIC tinyscript/compiler.cpp tinyscript/vm.cpp)
target_include_directories(TinyScript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tinyscript)

add_library(TinyAIEngine STATIC src/engine.cpp)
target_include_directories(TinyAIEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(TinyAIEngine PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
target_link_libraries(TinyAIEngine PUBLIC user32 gdi32)

add_executable(TinyAIEditor WIN32 src/editor.cpp)
target_link_libraries(TinyAIEditor PRIVATE TinyAIEngine TinyScript)
target_compile_definitions(TinyAIEditor PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

add_executable(TinyAIRuntime WIN32 src/runtime.cpp)
target_link_libraries(TinyAIRuntime PRIVATE TinyAIEngine TinyScript)
target_compile_definitions(TinyAIRuntime PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)

install(TARGETS TinyAIEditor TinyAIRuntime RUNTIME DESTINATION .)
install(DIRECTORY templates/ DESTINATION templates)
