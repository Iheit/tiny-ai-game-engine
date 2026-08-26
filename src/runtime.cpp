#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string>
#include "engine.hpp"
using namespace tiny;
static Engine g; static std::wstring file;
static LRESULT CALLBACK wnd(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_SIZE){g.resize(LOWORD(l),HIWORD(l));return 0;}if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int){WNDCLASSW wc{};wc.lpfnWndProc=wnd;wc.hInstance=hi;wc.lpszClassName=L"TinyRuntime";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassW(&wc);HWND h=CreateWindowW(L"TinyRuntime",L"Tiny Game",WS_OVERLAPPEDWINDOW|WS_VISIBLE,120,80,960,640,nullptr,nullptr,hi,nullptr);g.initialize(h,960,640);int argc=0;LPWSTR*av=CommandLineToArgvW(GetCommandLineW(),&argc);if(argc>1){std::string data;if(readUtf8File(av[1],data)){std::string err;deserialize(data,g.scene(),err);}}if(av)LocalFree(av);MSG msg{};ULONGLONG last=GetTickCount64();while(msg.message!=WM_QUIT){while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}auto now=GetTickCount64();float dt=float(now-last)/1000.0f;last=now;g.play(true);g.tick(dt);g.render();}return 0;}
