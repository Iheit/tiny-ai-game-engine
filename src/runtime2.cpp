#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>

struct Obj { std::string name; int type=0; float x=0,y=0,z=0,rx=0,ry=0,rz=0,sx=1,sy=1,sz=1; int color=0; bool dynamic=false; };
struct V3 { float x,y,z; };
static std::vector<Obj> scene; static HWND win; static float camX=0,camY=2.0f,camZ=-10,camYaw=0,camPitch=0; static bool mouseLook=false; static POINT lastMouse{}; static ULONGLONG lastTick=0;
static V3 rot(V3 p,float rx,float ry,float rz){rx*=.0174532925f;ry*=.0174532925f;rz*=.0174532925f;float cx=cosf(rx),sx=sinf(rx),cy=cosf(ry),sy=sinf(ry),cz=cosf(rz),sz=sinf(rz);float y=p.y*cx-p.z*sx,z=p.y*sx+p.z*cx;p.y=y;p.z=z;float x=p.x*cy-p.z*sy;p.z=p.x*sy+p.z*cy;p.x=x;x=p.x*cz-p.y*sz;y=p.x*sz+p.y*cz;p.x=x;p.y=y;return p;}
static bool quoted(std::istream&in,std::string&out){in>>std::ws;if(in.peek()!='"'){in>>out;return bool(in);}in.get();std::getline(in,out,'"');return bool(in);}
static bool load(const std::string&path){std::ifstream f(path);if(!f)return false;std::string h;f>>h;if(h!="TinyAI")return false;int version=1;f>>version;scene.clear();std::string t;while(f>>t){if(t!="object")break;Obj o;if(!quoted(f,o.name))break;f>>o.type>>o.x>>o.y>>o.z>>o.rx>>o.ry>>o.rz>>o.sx>>o.sy>>o.sz>>o.color;if(version>=3){int d=0;f>>d;o.dynamic=d!=0;}if(f)scene.push_back(o);}return !scene.empty();}
static POINT proj(V3 p,RECT&r){float cy=cosf(camYaw),sy=sinf(camYaw),cp=cosf(camPitch),sp=sinf(camPitch);float X=p.x-camX,Y=p.y-camY,Z=p.z-camZ;float xx=X*cy-Z*sy,zz=X*sy+Z*cy;float yy=Y*cp-zz*sp,zz2=Y*sp+zz*cp;zz2=std::max(.15f,zz2);float f=.9f;return {(LONG)(r.right*.5f+xx*f*r.bottom/zz2),(LONG)(r.bottom*.5f-yy*f*r.bottom/zz2)};}
static void cube(HDC dc,const Obj&o,RECT&r){V3 vv[8]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};int ids[6][4]={{0,1,2,3},{4,5,6,7},{0,4,5,1},{3,2,6,7},{1,5,6,2},{0,3,7,4}};int shade[6]={-35,30,-10,20,10,-20};struct F{POINT p[4];float z;int s;};std::vector<F>fs;for(int k=0;k<6;k++){F q{};for(int j=0;j<4;j++){V3 p=vv[ids[k][j]];p.x*=o.sx;p.y*=o.sy;p.z*=o.sz;p=rot(p,o.rx,o.ry,o.rz);p.x+=o.x;p.y+=o.y;p.z+=o.z;q.p[j]=proj(p,r);q.z+=p.z;}q.z/=4;q.s=shade[k];fs.push_back(q);}std::sort(fs.begin(),fs.end(),[](const F&a,const F&b){return a.z>b.z;});for(auto&q:fs){POINT p[4]={q.p[0],q.p[1],q.p[2],q.p[3]};int base=120+o.color*25;int c=std::clamp(base+q.s,25,240);HBRUSH b=CreateSolidBrush(RGB(c,c+std::min(25,255-c),std::min(255,c+35)));HPEN pe=CreatePen(PS_SOLID,1,RGB(25,30,36));HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,pe);Polygon(dc,p,4);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(pe);}}
static void plane(HDC dc,const Obj&o,RECT&r){V3 p[4]={{-1,0,-1},{1,0,-1},{1,0,1},{-1,0,1}};POINT q[4];for(int i=0;i<4;i++){p[i].x*=o.sx;p[i].y*=o.sy;p[i].z*=o.sz;p[i]=rot(p[i],o.rx,o.ry,o.rz);p[i].x+=o.x;p[i].y+=o.y;p[i].z+=o.z;q[i]=proj(p[i],r);}HBRUSH b=CreateSolidBrush(RGB(70,75,82));HGDIOBJ ob=SelectObject(dc,b);Polygon(dc,q,4);SelectObject(dc,ob);DeleteObject(b);}
static void sphere(HDC dc,const Obj&o,RECT&r){POINT p=proj({o.x,o.y,o.z},r);int rad=(int)(38*std::max({o.sx,o.sy,o.sz})*std::min(1.f,10.f/std::max(2.f,fabsf(o.z-camZ))));HBRUSH b=CreateSolidBrush(RGB(105,155,215));HGDIOBJ ob=SelectObject(dc,b);Ellipse(dc,p.x-rad,p.y-rad,p.x+rad,p.y+rad);SelectObject(dc,ob);DeleteObject(b);}
static void draw(HDC dc,RECT&r){HBRUSH bg=CreateSolidBrush(RGB(18,22,28));FillRect(dc,&r,bg);DeleteObject(bg);for(auto&o:scene){if(o.name=="Main Camera")continue;if(o.type==1)sphere(dc,o,r);else if(o.type==2)plane(dc,o,r);else cube(dc,o,r);}SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(235,240,248));TextOutA(dc,16,14,"Tiny AI Game",-1);TextOutA(dc,16,36,"WASD move   RMB look   Space jump   Esc quit",-1);}
static LRESULT CALLBACK wndProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);draw(dc,r);EndPaint(h,&ps);return 0;}
    if(m==WM_RBUTTONDOWN){SetCapture(h);mouseLook=true;lastMouse={GET_X_LPARAM(l),GET_Y_LPARAM(l)};return 0;}
    if(m==WM_RBUTTONUP){ReleaseCapture();mouseLook=false;return 0;}
    if(m==WM_MOUSEMOVE&&mouseLook){int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);camYaw+=(x-lastMouse.x)*.004f;camPitch=std::clamp(camPitch+(y-lastMouse.y)*.004f,-1.35f,1.35f);lastMouse={x,y};InvalidateRect(h,nullptr,FALSE);return 0;}
    if(m==WM_KEYDOWN&&w==VK_ESCAPE){DestroyWindow(h);return 0;}
    if(m==WM_TIMER){ULONGLONG now=GetTickCount64();float dt=std::min(.05f,float(now-lastTick)/1000.f);lastTick=now;float speed=(GetAsyncKeyState(VK_SHIFT)&0x8000)?9.f:5.f;float sy=sinf(camYaw),cy=cosf(camYaw);if(GetAsyncKeyState('W')&0x8000){camX+=sy*speed*dt;camZ+=cy*speed*dt;}if(GetAsyncKeyState('S')&0x8000){camX-=sy*speed*dt;camZ-=cy*speed*dt;}if(GetAsyncKeyState('A')&0x8000){camX-=cy*speed*dt;camZ+=sy*speed*dt;}if(GetAsyncKeyState('D')&0x8000){camX+=cy*speed*dt;camZ-=sy*speed*dt;}for(auto&o:scene)if(o.dynamic){o.ry+=35.f*dt;o.y-=9.8f*dt;if(o.y<1){o.y=1;o.dynamic=false;}}InvalidateRect(h,nullptr,FALSE);}
    if(m==WM_DESTROY)PostQuitMessage(0);return DefWindowProcA(h,m,w,l);
}
int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR cmd,int){std::string path=cmd?cmd:"";while(!path.empty()&&(path.front()==' '||path.front()=='"'))path.erase(path.begin());while(!path.empty()&&path.back()=='"')path.pop_back();if(path.empty())path="project.tinyproj";load(path);WNDCLASSA c{};c.hInstance=hi;c.lpfnWndProc=wndProc;c.lpszClassName="TinyAIRuntime2";c.hCursor=LoadCursor(nullptr,IDC_ARROW);c.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassA(&c);win=CreateWindowA(c.lpszClassName,"Tiny AI Game",WS_OVERLAPPEDWINDOW|WS_VISIBLE,140,80,1200,760,nullptr,nullptr,hi,nullptr);lastTick=GetTickCount64();SetTimer(win,1,16,nullptr);MSG msg{};while(GetMessageA(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageA(&msg);}return 0;}
