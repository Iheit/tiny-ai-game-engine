#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

struct Obj { std::string name; float x=0,y=0,z=0,rx=0,ry=0,rz=0,scale=1; };
static std::vector<Obj> scene={{"Cube",0,0,0,0,0,0,1}};
static HWND viewWnd, listWnd, statusWnd; static std::string projectFile="project.tiny"; static float angle=0;
static void status(const std::string&s){SetWindowTextA(statusWnd,s.c_str());}
static void save(){std::ofstream f(projectFile); f<<"TinyAI 1\n"; for(auto&o:scene)f<<"object "<<o.name<<" "<<o.x<<" "<<o.y<<" "<<o.z<<" "<<o.rx<<" "<<o.ry<<" "<<o.rz<<" "<<o.scale<<"\n"; status("Saved "+projectFile);}
static void load(){std::ifstream f(projectFile); if(!f)return; std::string h; std::getline(f,h); scene.clear(); std::string t,n; Obj o; while(f>>t){if(t=="object"){f>>o.name>>o.x>>o.y>>o.z>>o.rx>>o.ry>>o.rz>>o.scale;scene.push_back(o);}} status("Loaded "+projectFile); InvalidateRect(viewWnd,nullptr,FALSE);}
static POINT project(float x,float y,float z){float dz=z+5; return {(LONG)(480+x*180/dz),(LONG)(280-y*180/dz)};}
static void cube(HDC dc,const Obj&o){float a=angle+o.ry*0.01745f,c=cosf(a),s=sinf(a); float p[8][3]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}}; POINT q[8]; for(int i=0;i<8;i++){float x=p[i][0]*o.scale,y=p[i][1]*o.scale,z=p[i][2]*o.scale; float X=x*c-z*s,Z=x*s+z*c; q[i]=project(X+o.x,y+o.y,Z+o.z);} int e[][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}}; HPEN pen=CreatePen(PS_SOLID,2,RGB(80,190,255)); auto old=SelectObject(dc,pen); for(auto&v:e){MoveToEx(dc,q[v[0]].x,q[v[0]].y,nullptr);LineTo(dc,q[v[1]].x,q[v[1]].y);} SelectObject(dc,old);DeleteObject(pen);}
static LRESULT CALLBACK view(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);HBRUSH b=CreateSolidBrush(RGB(24,28,36));FillRect(dc,&r,b);DeleteObject(b);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(170,180,195));TextOutA(dc,18,16,"3D VIEWPORT",11);for(auto&o:scene)cube(dc,o);EndPaint(h,&ps);return 0;}return DefWindowProc(h,m,w,l);}
static void add(){scene.push_back({"Cube"+std::to_string(scene.size()),0,0,0,0,0,0,1});InvalidateRect(viewWnd,nullptr,FALSE);status("Added cube");}
static void build(){CreateDirectoryA("Build",nullptr); CopyFileA("TinyAIRuntime.exe","Build\\TinyAIRuntime.exe",FALSE); save(); CopyFileA(projectFile.c_str(),"Build\\project.tiny",FALSE); status("Build ready: Build\\TinyAIRuntime.exe");}
static LRESULT CALLBACK wnd(HWND h,UINT m,WPARAM w,LPARAM l){if(m==WM_COMMAND){switch(LOWORD(w)){case 101:add();break;case 102:save();break;case 103:load();break;case 104:build();break;} } if(m==WM_TIMER){angle+=0.01f;InvalidateRect(viewWnd,nullptr,FALSE);} if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProc(h,m,w,l);}
int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int){WNDCLASSA wc{};wc.hInstance=hi;wc.lpfnWndProc=wnd;wc.lpszClassName="TinyAIEditor";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassA(&wc);HWND h=CreateWindowA(wc.lpszClassName,"Tiny AI Game Engine",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,70,1100,700,nullptr,nullptr,hi,nullptr);CreateWindowA("BUTTON","Add Cube",WS_CHILD|WS_VISIBLE,10,10,100,32,h,(HMENU)101,hi,nullptr);CreateWindowA("BUTTON","Save",WS_CHILD|WS_VISIBLE,120,10,80,32,h,(HMENU)102,hi,nullptr);CreateWindowA("BUTTON","Load",WS_CHILD|WS_VISIBLE,210,10,80,32,h,(HMENU)103,hi,nullptr);CreateWindowA("BUTTON","Build Game",WS_CHILD|WS_VISIBLE,300,10,110,32,h,(HMENU)104,hi,nullptr);viewWnd=CreateWindowA("STATIC","",WS_CHILD|WS_VISIBLE|WS_BORDER,10,55,1060,560,h,nullptr,hi,nullptr);SetWindowLongPtrA(viewWnd,GWLP_WNDPROC,(LONG_PTR)view);statusWnd=CreateWindowA("STATIC","Ready",WS_CHILD|WS_VISIBLE,10,625,1060,24,h,nullptr,hi,nullptr);SetTimer(h,1,16,nullptr);MSG msg;while(GetMessage(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessage(&msg);}return 0;}
