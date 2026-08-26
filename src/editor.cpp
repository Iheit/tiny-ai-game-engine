#include <windows.h>
#ifdef CreateWindowA
#undef CreateWindowA
#endif
#define CreateWindowA(...) CreateWindowExA(0, __VA_ARGS__)
#include <commdlg.h>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdio>
"engine.hpp"
#include "script.hpp"
using namespace tiny;
namespace fs=std::filesystem;

static HWND gWnd,gView,gTree,gName,gX,gY,gZ,gRX,gRY,gRZ,gSX,gSY,gSZ,gOut;
static Engine g;
static int sel=-1;
static std::wstring project;

enum{NEWP=1,OPENP,SAVE,BUILD,PLAY,STOP,ADD_CUBE,ADD_SPHERE,ADD_PLANE,ADD_CAM,ADD_DIR,ADD_POINT,ADD_SPOT,DEL,DUP,APPLY,FOCUS};

static HWND edit(HWND p,const char*t,int x,int y,int w,int h){return CreateWindowA("EDIT",t,WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL,x,y,w,h,p,nullptr,nullptr,nullptr);}
static HWND button(HWND p,const char*t,int id,int x,int y,int w,int h=28){return CreateWindowA("BUTTON",t,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,x,y,w,h,p,(HMENU)(INT_PTR)id,nullptr,nullptr);}
static HWND label(HWND p,const char*t,int x,int y,int w=80,int h=20){return CreateWindowA("STATIC",t,WS_CHILD|WS_VISIBLE,x,y,w,h,p,nullptr,nullptr,nullptr);}
static HWND group(HWND p,const char*t,int x,int y,int w,int h){return CreateWindowA("BUTTON",t,WS_CHILD|WS_VISIBLE|BS_GROUPBOX,x,y,w,h,p,nullptr,nullptr,nullptr);}
static void out(const std::string&s){if(gOut)SetWindowTextA(gOut,s.c_str());}

static void refresh(){
    SendMessageA(gTree,LB_RESETCONTENT,0,0);
    for(auto&e:g.scene().entities){std::string s=e.name+"  ["+kindName(e.kind)+"]";SendMessageA(gTree,LB_ADDSTRING,0,(LPARAM)s.c_str());}
    if(sel>=0&&sel<(int)g.scene().entities.size()){
        auto&e=g.scene().entities[sel]; SetWindowTextA(gName,e.name.c_str()); char b[64];
        auto set=[&](HWND h,float v){sprintf_s(b,"%.3f",v);SetWindowTextA(h,b);};
        set(gX,e.transform.position.x);set(gY,e.transform.position.y);set(gZ,e.transform.position.z);
        set(gRX,e.transform.rotation.x);set(gRY,e.transform.rotation.y);set(gRZ,e.transform.rotation.z);
        set(gSX,e.transform.scale.x);set(gSY,e.transform.scale.y);set(gSZ,e.transform.scale.z);
        SendMessageA(gTree,LB_SETCURSEL,sel,0);
    }
    InvalidateRect(gView,nullptr,FALSE);
}
static void save(){if(project.empty()){out("No project open.");return;}if(!writeUtf8File(project,serialize(g.scene())))out("Save failed.");else out("Saved "+narrow(project));}
static bool load(){std::string s,e;if(!readUtf8File(project,s)){out("Could not read project.");return false;}if(!deserialize(s,g.scene(),e)){out("Load error: "+e);return false;}sel=g.scene().entities.empty()?-1:0;refresh();out("Loaded "+narrow(project));return true;}
static void newProject(){char f[MAX_PATH]="MyGame.tinyproj";OPENFILENAMEA o{};o.lStructSize=sizeof(o);o.hwndOwner=gWnd;o.lpstrFile=f;o.nMaxFile=MAX_PATH;o.lpstrFilter="Tiny project\0*.tinyproj\0\0";o.Flags=OFN_OVERWRITEPROMPT;if(!GetSaveFileNameA(&o))return;project=widen(f);g=Engine{};g.initialize(gView,800,560);g.setInput(gWnd);save();refresh();}
static void openProject(){char f[MAX_PATH]{};OPENFILENAMEA o{};o.lStructSize=sizeof(o);o.hwndOwner=gWnd;o.lpstrFile=f;o.nMaxFile=MAX_PATH;o.lpstrFilter="Tiny project\0*.tinyproj\0All files\0*.*\0\0";o.Flags=OFN_FILEMUSTEXIST;if(!GetOpenFileNameA(&o))return;project=widen(f);load();}
static void add(EntityKind k){Entity e;e.id=g.scene().nextId++;e.kind=k;e.name=kindName(k);int n=1;for(auto&q:g.scene().entities)if(q.kind==k)++n;e.name+=std::to_string(n);if(k==EntityKind::Plane)e.transform.scale={5,.1f,5};if(k==EntityKind::Cube){e.transform.position={0,1,0};e.dynamic=true;}if(k==EntityKind::Sphere)e.transform.position={2,1,0};if(k==EntityKind::PointLight)e.transform.position={2,3,-2};if(k==EntityKind::SpotLight)e.transform.position={-2,3,-2};g.scene().entities.push_back(e);sel=(int)g.scene().entities.size()-1;refresh();out(e.name+" added.");}
static float gf(HWND h,float old){char b[64];GetWindowTextA(h,b,64);char*e;float v=strtof(b,&e);return e==b?old:v;}
static void apply(){if(sel<0)return;auto&e=g.scene().entities[sel];char n[128];GetWindowTextA(gName,n,128);if(n[0])e.name=n;e.transform.position={gf(gX,e.transform.position.x),gf(gY,e.transform.position.y),gf(gZ,e.transform.position.z)};e.transform.rotation={gf(gRX,e.transform.rotation.x),gf(gRY,e.transform.rotation.y),gf(gRZ,e.transform.rotation.z)};e.transform.scale={std::max(.01f,gf(gSX,e.transform.scale.x)),std::max(.01f,gf(gSY,e.transform.scale.y)),std::max(.01f,gf(gSZ,e.transform.scale.z))};refresh();out("Inspector applied.");}
static void build(){if(project.empty()){out("Open or create a project first.");return;}save();fs::path dir=fs::path(project).parent_path()/"build";fs::create_directories(dir);wchar_t exe[MAX_PATH];GetModuleFileNameW(nullptr,exe,MAX_PATH);fs::path runtime=fs::path(exe).parent_path()/L"TinyAIRuntime.exe";if(!fs::exists(runtime)){out("Runtime executable is missing. Build the engine first.");return;}fs::copy_file(runtime,dir/"Game.exe",fs::copy_options::overwrite_existing);fs::copy_file(project,dir/"game.tinyproj",fs::copy_options::overwrite_existing);out("Built standalone game in "+narrow(dir.wstring()));}

static void layout(HWND h){RECT r{};GetClientRect(h,&r);int W=r.right,H=r.bottom;int left=210,right=300,top=76,bottom=96;int vw=(std::max)(200,W-left-right),vh=(std::max)(160,H-top-bottom);MoveWindow(gView,left,top,vw,vh,TRUE);MoveWindow(gOut,left,H-78,W-left-8,66,TRUE);}

static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(h==gView){
        if(m==WM_SIZE){g.resize(LOWORD(l),HIWORD(l));return 0;}
        if(m==WM_LBUTTONDOWN){POINT p{LOWORD(l),HIWORD(l)};for(int i=0;i<(int)g.scene().entities.size();++i){auto&e=g.scene().entities[i];if(e.kind==EntityKind::Camera)continue;RECT rr{};GetClientRect(gView,&rr);float sx=rr.right/2.0f+e.transform.position.x*35;float sy=rr.bottom/2.0f-e.transform.position.y*35;if((p.x-sx)*(p.x-sx)+(p.y-sy)*(p.y-sy)<900){sel=i;refresh();break;}}return 0;}
        return DefWindowProcA(h,m,w,l);
    }
    if(m==WM_SIZE){layout(h);return 0;}
    if(m==WM_COMMAND){
        int id=LOWORD(w),code=HIWORD(w);
        if(id==200&&code==LBN_SELCHANGE){sel=(int)SendMessageA(gTree,LB_GETCURSEL,0,0);refresh();return 0;}
        if(id==NEWP)newProject();else if(id==OPENP)openProject();else if(id==SAVE)save();else if(id==BUILD)build();
        else if(id==PLAY){g.play(true);out("Play mode running. WASD moves dynamic objects.");}
        else if(id==STOP){g.play(false);out("Play mode stopped.");}
        else if(id==ADD_CUBE)add(EntityKind::Cube);else if(id==ADD_SPHERE)add(EntityKind::Sphere);else if(id==ADD_PLANE)add(EntityKind::Plane);else if(id==ADD_CAM)add(EntityKind::Camera);else if(id==ADD_DIR)add(EntityKind::DirectionalLight);else if(id==ADD_POINT)add(EntityKind::PointLight);else if(id==ADD_SPOT)add(EntityKind::SpotLight);
        else if(id==DEL&&sel>=0){if(g.scene().entities[sel].id!=g.scene().cameraId){g.scene().entities.erase(g.scene().entities.begin()+sel);sel=-1;refresh();out("Deleted.");}}
        else if(id==DUP&&sel>=0){auto e=g.scene().entities[sel];e.id=g.scene().nextId++;e.name+=" Copy";e.transform.position.x+=1;g.scene().entities.push_back(e);sel=(int)g.scene().entities.size()-1;refresh();}
        else if(id==APPLY)apply();
        else if(id==FOCUS&&sel>=0){auto&e=g.scene().entities[sel];if(e.kind!=EntityKind::Camera){auto c=std::find_if(g.scene().entities.begin(),g.scene().entities.end(),[](auto&q){return q.kind==EntityKind::Camera;});if(c!=g.scene().entities.end()){c->transform.position={e.transform.position.x,e.transform.position.y+2,e.transform.position.z-6};out("Focused selection.");}}}
        return 0;
    }
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcA(h,m,w,l);
}

int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int){
    WNDCLASSA wc{};wc.lpfnWndProc=proc;wc.hInstance=hi;wc.lpszClassName="TinyEditor";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassA(&wc);
    gWnd=CreateWindowA("TinyEditor","Tiny AI Game Engine | Editor",WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_VISIBLE,80,50,1280,820,nullptr,nullptr,hi,nullptr);
    group(gWnd,"Project",8,38,190,40);button(gWnd,"New",NEWP,16,49,52);button(gWnd,"Open",OPENP,72,49,52);button(gWnd,"Save",SAVE,128,49,52);
    group(gWnd,"Build / Run",202,38,250,40);button(gWnd,"Build",BUILD,210,49,55);button(gWnd,"Play",PLAY,269,49,55);button(gWnd,"Stop",STOP,328,49,55);button(gWnd,"Focus",FOCUS,387,49,58);
    group(gWnd,"Hierarchy",8,82,194,500);gTree=CreateWindowA("LISTBOX","",WS_CHILD|WS_VISIBLE|WS_BORDER|LBS_NOTIFY|WS_VSCROLL,16,110,178,390,gWnd,(HMENU)200,hi,nullptr);
    group(gWnd,"Create",8,505,194,150);button(gWnd,"Cube",ADD_CUBE,16,528,52);button(gWnd,"Sphere",ADD_SPHERE,72,528,56);button(gWnd,"Plane",ADD_PLANE,132,528,56);button(gWnd,"Camera",ADD_CAM,16,562,58);button(gWnd,"Dir Light",ADD_DIR,78,562,72);button(gWnd,"Point",ADD_POINT,154,562,42);button(gWnd,"Spot",ADD_SPOT,16,596,52);button(gWnd,"Duplicate",DUP,74,596,72);button(gWnd,"Delete",DEL,150,596,44);
    gView=CreateWindowA("TinyEditor","",WS_CHILD|WS_VISIBLE|WS_BORDER,210,82,700,520,gWnd,nullptr,hi,nullptr);
    group(gWnd,"Inspector",918,82,334,500);label(gWnd,"Name",930,110,48);gName=edit(gWnd,"",978,108,250,24);
    const char*names[]={"X","Y","Z","RX","RY","RZ","SX","SY","SZ"};HWND*es[]={&gX,&gY,&gZ,&gRX,&gRY,&gRZ,&gSX,&gSY,&gSZ};
    for(int i=0;i<9;++i){int row=i/3,col=i%3;int x=932+col*102,y=150+row*58;label(gWnd,names[i],x,y,24);*es[i]=edit(gWnd,"0",x+24,y-2,68,24);}
    button(gWnd,"Apply Changes",APPLY,930,330,120,30);label(gWnd,"Tip: select an entity in Hierarchy or the viewport.",930,375,290,36);
    label(gWnd,"Viewport",220,60,120,20);label(gWnd,"Output",220,0,120,20);
    gOut=CreateWindowA("EDIT","Ready.  New/Open creates a scene you can build.",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_MULTILINE|ES_READONLY,210,700,1040,66,gWnd,nullptr,hi,nullptr);
    g.initialize(gView,700,520);g.setInput(gWnd);refresh();layout(gWnd);
    MSG msg;while(GetMessageA(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageA(&msg);if(g.playing())g.tick(.016f);g.render();}
    return 0;
}
