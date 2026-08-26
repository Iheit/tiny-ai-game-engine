#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
struct Obj{std::string name;float x,y,z,rx,ry,rz,scale;}; static std::vector<Obj> s; static float a=0;
static POINT P(float x,float y,float z){float d=z+6;return{(LONG)(480+x*190/d),(LONG)(270-y*190/d)};}
static void draw(HDC dc,const Obj&o){float c=cosf(a),q=sinf(a);float v[8][3]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};POINT p[8];for(int i=0;i<8;i++){float x=v[i][0]*o.scale,y=v[i][1]*o.scale,z=v[i][2]*o.scale;float X=x*c-z*q,Z=x*q+z*c;p[i]=P(X+o.x,y+o.y,Z+o.z);}int e[][2]={{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};for(auto&x:e){MoveToEx(dc,p[x[0]].x,p[x[0]].y,nullptr);LineTo(dc,p[x[1]].x,p[x[1]].y);}}
static LRESULT CALLBACK W(HWND h,UINT m,WPARAM,LPARAM){if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);FillRect(dc,&r,(HBRUSH)GetStockObject(BLACK_BRUSH));HPEN pen=CreatePen(PS_SOLID,3,RGB(70,190,255));auto old=SelectObject(dc,pen);for(auto&o:s)draw(dc,o);SelectObject(dc,old);DeleteObject(pen);EndPaint(h,&ps);return 0;}if(m==WM_TIMER){a+=.015f;InvalidateRect(h,nullptr,FALSE);}if(m==WM_DESTROY)PostQuitMessage(0);return DefWindowProc(h,m,0,0);}
int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int){std::ifstream f("project.tiny");std::string t,n;if(f){std::string h;f>>h;while(f>>t)if(t=="object"){Obj o;f>>o.name>>o.x>>o.y>>o.z>>o.rx>>o.ry>>o.rz>>o.scale;s.push_back(o);}}if(s.empty())s.push_back({"Cube",0,0,0,0,0,0,1});WNDCLASSA c{};c.hInstance=hi;c.lpfnWndProc=W;c.lpszClassName="TinyAIRuntime";RegisterClassA(&c);HWND w=CreateWindowA(c.lpszClassName,"Tiny AI Game",WS_OVERLAPPEDWINDOW|WS_VISIBLE,200,120,960,540,nullptr,nullptr,hi,nullptr);SetTimer(w,1,16,nullptr);MSG m;while(GetMessage(&m,nullptr,0,0)){TranslateMessage(&m);DispatchMessage(&m);}return 0;}
