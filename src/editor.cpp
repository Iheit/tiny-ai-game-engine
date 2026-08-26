#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shlobj.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <cstdio>
#include "../tinyscript/compiler.hpp"

namespace fs = std::filesystem;

struct Obj { std::string name; int type=0; float x=0,y=0,z=0,rx=0,ry=0,rz=0,sx=1,sy=1,sz=1; int color=0; bool dynamic=false; };
struct V3 { float x,y,z; };
struct PV { float x,y,z; int shade; };

static std::vector<Obj> scene;
static int selected=-1;
static bool hub=true, playing=false, dirty=false, dragging=false, panMode=false;
static HWND mainWnd=nullptr, viewport=nullptr, hierarchy=nullptr, assets=nullptr, output=nullptr, scriptEdit=nullptr, nameEdit=nullptr;
static HWND xEdit=nullptr,yEdit=nullptr,zEdit=nullptr,rxEdit=nullptr,ryEdit=nullptr,rzEdit=nullptr,sxEdit=nullptr,syEdit=nullptr,szEdit=nullptr;
static std::string projectFile, projectDir, scriptFile;
static float yaw=-0.65f,pitch=0.35f,distance=14.f,panX=0,panY=1.0f; static POINT dragStart{};
static bool mouseLook=false; static ULONGLONG lastTick=0;
static constexpr int ID_NEW=101,ID_OPEN=102,ID_RECENT=103,ID_SETTINGS=104,ID_ADD_CUBE=105,ID_ADD_SPHERE=106,ID_ADD_PLANE=107,ID_DELETE=108,ID_DUP=109,ID_SAVE=110,ID_LOAD=111,ID_PLAY=112,ID_STOP=113,ID_HUB=114,ID_APPLY=115,ID_FOCUS=117,ID_BUILD=118,ID_SCRIPT_SAVE=119,ID_SCRIPT_COMPILE=120,ID_SCRIPT_NEW=121;
static constexpr int ID_HIER=200, ID_ASSETS=201;

static void setStatus(const std::string& s){ if(output) SetWindowTextA(output,s.c_str()); }
static void setFloat(HWND h,float v){char b[64];sprintf_s(b,"%.3f",v);SetWindowTextA(h,b);}
static float getFloat(HWND h,float old){char b[64];GetWindowTextA(h,b,64);char*e=nullptr;float v=strtof(b,&e);return e!=b?v:old;}
static HWND makeEdit(const char* text,int x,int y,int w,int h,DWORD style=WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL){return CreateWindowA("EDIT",text,style,x,y,w,h,mainWnd,nullptr,GetModuleHandleA(nullptr),nullptr);}
static void button(const char* text,int id,int x,int y,int w,int h=28){CreateWindowA("BUTTON",text,WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,x,y,w,h,mainWnd,(HMENU)id,GetModuleHandleA(nullptr),nullptr);}
static void label(const char* text,int x,int y,int w=70){CreateWindowA("STATIC",text,WS_CHILD|WS_VISIBLE,x,y,w,20,mainWnd,nullptr,GetModuleHandleA(nullptr),nullptr);}

static void refreshHierarchy(){
    if(!hierarchy)return; SendMessageA(hierarchy,LB_RESETCONTENT,0,0);
    for(auto& o:scene){std::string s=(o.type==1?"[Sphere] ":o.type==2?"[Plane] ":"[Cube] ")+o.name;SendMessageA(hierarchy,LB_ADDSTRING,0,(LPARAM)s.c_str());}
    if(selected>=0 && selected<(int)scene.size()){
        SendMessageA(hierarchy,LB_SETCURSEL,selected,0); auto&o=scene[selected];
        SetWindowTextA(nameEdit,o.name.c_str());setFloat(xEdit,o.x);setFloat(yEdit,o.y);setFloat(zEdit,o.z);setFloat(rxEdit,o.rx);setFloat(ryEdit,o.ry);setFloat(rzEdit,o.rz);setFloat(sxEdit,o.sx);setFloat(syEdit,o.sy);setFloat(szEdit,o.sz);
    }
    InvalidateRect(viewport,nullptr,FALSE);
}

static void saveScene(){
    if(projectFile.empty()){setStatus("No project is open.");return;}
    std::ofstream f(projectFile,std::ios::trunc); if(!f){setStatus("ERROR: could not save scene.");return;}
    f<<"TinyAI 3\n"; for(auto&o:scene) f<<"object \""<<o.name<<"\" "<<o.type<<' '<<o.x<<' '<<o.y<<' '<<o.z<<' '<<o.rx<<' '<<o.ry<<' '<<o.rz<<' '<<o.sx<<' '<<o.sy<<' '<<o.sz<<' '<<o.color<<' '<<(o.dynamic?1:0)<<"\n";
    dirty=false; setStatus("Saved " + projectFile);
}
static bool readQuoted(std::istream&in,std::string&out){in>>std::ws;if(in.peek()!='\"'){in>>out;return bool(in);}in.get();std::getline(in,out,'\"');return bool(in);}
static bool loadScene(){
    if(projectFile.empty())return false;std::ifstream f(projectFile);if(!f){setStatus("ERROR: could not open scene.");return false;}std::string h;f>>h;if(h!="TinyAI"){setStatus("ERROR: unsupported project file.");return false;}
    int version=1;f>>version;scene.clear();std::string t;while(f>>t){if(t!="object")break;Obj o;if(!readQuoted(f,o.name))break;f>>o.type>>o.x>>o.y>>o.z>>o.rx>>o.ry>>o.rz>>o.sx>>o.sy>>o.sz>>o.color;if(version>=3){int d=0;f>>d;o.dynamic=d!=0;}if(f)scene.push_back(o);}selected=scene.empty()?-1:0;dirty=false;refreshHierarchy();setStatus("Loaded " + projectFile);return true;
}
static void ensureFolders(){if(!projectDir.empty()){fs::create_directories(fs::path(projectDir)/"assets");fs::create_directories(fs::path(projectDir)/"scripts");fs::create_directories(fs::path(projectDir)/"build");}}
static void newProject(){
    char file[MAX_PATH]="MyGame.tinyproj";OPENFILENAMEA o{};o.lStructSize=sizeof(o);o.hwndOwner=mainWnd;o.lpstrFile=file;o.nMaxFile=MAX_PATH;o.lpstrFilter="Tiny AI Project\0*.tinyproj\0\0";o.Flags=OFN_OVERWRITEPROMPT;
    if(!GetSaveFileNameA(&o))return;projectFile=file;projectDir=fs::path(projectFile).parent_path().string();ensureFolders();scene={{"Main Camera",0,0,2,-5,0,0,1,1,1,0,false},{"Ground",2,0,-1,0,0,0,6,.1f,6,1,false},{"Player",0,0,1,0,0,0,1,1,1,0,true}};selected=2;dirty=true;saveScene();scriptFile.clear();showEditor();
}
static void openProject(){char file[MAX_PATH]={};OPENFILENAMEA o{};o.lStructSize=sizeof(o);o.hwndOwner=mainWnd;o.lpstrFile=file;o.nMaxFile=MAX_PATH;o.lpstrFilter="Tiny AI Project\0*.tinyproj\0All Files\0*.*\0\0";o.Flags=OFN_FILEMUSTEXIST;if(!GetOpenFileNameA(&o))return;projectFile=file;projectDir=fs::path(projectFile).parent_path().string();ensureFolders();loadScene();showEditor();}
static void addObj(int type){const char*base=type==1?"Sphere":type==2?"Plane":"Cube";int n=1;for(auto&o:scene)if(o.name.rfind(base,0)==0)n++;Obj o;o.name=std::string(base)+std::to_string(n);o.type=type;o.x=(float)((int)scene.size()%5-2)*2.f;o.y=type==2?0:1;o.z=(float)((int)scene.size()/5)*2.f;o.dynamic=type!=2;scene.push_back(o);selected=(int)scene.size()-1;dirty=true;refreshHierarchy();setStatus(o.name+" added.");}
static void deleteObj(){if(selected<0||selected>=(int)scene.size())return;if(scene[selected].name=="Main Camera"){setStatus("The main camera cannot be deleted.");return;}scene.erase(scene.begin()+selected);selected=scene.empty()?-1:std::min(selected,(int)scene.size()-1);dirty=true;refreshHierarchy();setStatus("Object deleted.");}
static void duplicateObj(){if(selected<0||selected>=(int)scene.size())return;Obj o=scene[selected];o.name+=" Copy";o.x+=1;o.z+=1;scene.push_back(o);selected=(int)scene.size()-1;dirty=true;refreshHierarchy();setStatus("Object duplicated.");}
static void applyInspector(){if(selected<0||selected>=(int)scene.size())return;auto&o=scene[selected];char n[128];GetWindowTextA(nameEdit,n,128);if(n[0])o.name=n;o.x=getFloat(xEdit,o.x);o.y=getFloat(yEdit,o.y);o.z=getFloat(zEdit,o.z);o.rx=getFloat(rxEdit,o.rx);o.ry=getFloat(ryEdit,o.ry);o.rz=getFloat(rzEdit,o.rz);o.sx=std::max(.01f,getFloat(sxEdit,o.sx));o.sy=std::max(.01f,getFloat(syEdit,o.sy));o.sz=std::max(.01f,getFloat(szEdit,o.sz));dirty=true;refreshHierarchy();setStatus("Inspector applied.");}

static POINT project(V3 v,RECT&r){float cy=cosf(yaw),sy=sinf(yaw),cp=cosf(pitch),sp=sinf(pitch);float X=v.x-panX,Y=v.y-panY,Z=v.z;float xx=X*cy-Z*sy,zz=X*sy+Z*cy;float yy=Y*cp-zz*sp,zz2=Y*sp+zz*cp+distance;zz2=std::max(.15f,zz2);float f=.95f;return {(LONG)(r.right*.5f+xx*f*r.bottom/zz2),(LONG)(r.bottom*.5f-yy*f*r.bottom/zz2)};}
static V3 rot(V3 p,float rx,float ry,float rz){rx*=.0174532925f;ry*=.0174532925f;rz*=.0174532925f;float cx=cosf(rx),sx=sinf(rx),cy=cosf(ry),sy=sinf(ry),cz=cosf(rz),sz=sinf(rz);float y=p.y*cx-p.z*sx,z=p.y*sx+p.z*cx;p.y=y;p.z=z;float x=p.x*cy-p.z*sy;p.z=p.x*sy+p.z*cy;p.x=x;x=p.x*cz-p.y*sz;y=p.x*sz+p.y*cz;p.x=x;p.y=y;return p;}
static int faceColor(int base,int light){int v=std::clamp(base+light,20,245);return v;}
static void fillBox(HDC dc,const Obj&o,RECT&r,bool sel){
    V3 vv[8]={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};int f[6][4]={{0,1,2,3},{4,5,6,7},{0,4,5,1},{3,2,6,7},{1,5,6,2},{0,3,7,4}};int shades[6]={-35,30,-10,20,10,-20};struct Face{POINT p[4];float z;int shade;};std::vector<Face> fs;for(int k=0;k<6;k++){Face q{};q.z=0;q.shade=shades[k];for(int j=0;j<4;j++){V3 p=vv[f[k][j]];p.x*=o.sx;p.y*=o.sy;p.z*=o.sz;p=rot(p,o.rx,o.ry,o.rz);p.x+=o.x;p.y+=o.y;p.z+=o.z;q.p[j]=project(p,r);q.z+=p.z;}q.z/=4;fs.push_back(q);}std::sort(fs.begin(),fs.end(),[](auto&a,auto&b){return a.z>b.z;});for(auto&q:fs){POINT pp[4]={q.p[0],q.p[1],q.p[2],q.p[3]};HBRUSH b=CreateSolidBrush(RGB(faceColor(105+o.color*30,q.shade),faceColor(125+o.color*20,q.shade),faceColor(155+o.color*15,q.shade)));HPEN pen=CreatePen(PS_SOLID,sel?2:1,sel?RGB(255,190,70):RGB(35,40,48));HGDIOBJ ob=SelectObject(dc,b),op=SelectObject(dc,pen);Polygon(dc,pp,4);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(pen);} }
static void fillPlane(HDC dc,const Obj&o,RECT&r,bool sel){V3 a{-1,-.02f,-1},b{1,-.02f,-1},c{1,-.02f,1},d{-1,-.02f,1};for(V3*p:{&a,&b,&c,&d}){p->x*=o.sx;p->y*=o.sy;p->z*=o.sz;*p=rot(*p,o.rx,o.ry,o.rz);p->x+=o.x;p->y+=o.y;p->z+=o.z;}POINT q[4]={project(a,r),project(b,r),project(c,r),project(d,r)};HBRUSH br=CreateSolidBrush(RGB(72,78,86));HPEN pe=CreatePen(PS_SOLID,sel?2:1,sel?RGB(255,190,70):RGB(35,40,48));HGDIOBJ ob=SelectObject(dc,br),op=SelectObject(dc,pe);Polygon(dc,q,4);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(pe);}
static void fillSphere(HDC dc,const Obj&o,RECT&r,bool sel){POINT p=project({o.x,o.y,o.z},r);int rad=(int)(38*std::max({o.sx,o.sy,o.sz})*std::min(1.f,10.f/distance));HBRUSH br=CreateSolidBrush(RGB(110,150,205));HPEN pe=CreatePen(PS_SOLID,sel?3:1,sel?RGB(255,190,70):RGB(35,40,48));HGDIOBJ ob=SelectObject(dc,br),op=SelectObject(dc,pe);Ellipse(dc,p.x-rad,p.y-rad,p.x+rad,p.y+rad);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(pe);}
static void drawGrid(HDC dc,RECT&r){HPEN p=CreatePen(PS_SOLID,1,RGB(48,53,62));HGDIOBJ old=SelectObject(dc,p);for(int i=-20;i<=20;i++){POINT a=project({(float)i,0,-20},r),b=project({(float)i,0,20},r);MoveToEx(dc,a.x,a.y,nullptr);LineTo(dc,b.x,b.y);a=project({-20,0,(float)i},r);b=project({20,0,(float)i},r);MoveToEx(dc,a.x,a.y,nullptr);LineTo(dc,b.x,b.y);}SelectObject(dc,old);DeleteObject(p);}
static int hitTest(int mx,int my,RECT&r){int best=-1;float bd=70;for(int i=0;i<(int)scene.size();i++){if(scene[i].name=="Main Camera")continue;POINT p=project({scene[i].x,scene[i].y,scene[i].z},r);float radius=scene[i].type==1?45.f:50.f;if(std::hypot(float(mx-p.x),float(my-p.y))<bd+radius){bd=radius;best=i;}}return best;}

static void populateAssets(){if(!assets)return;SendMessageA(assets,LB_RESETCONTENT,0,0);if(projectDir.empty())return;fs::path dir=fs::path(projectDir)/"assets";if(!fs::exists(dir))return;for(auto&e:fs::recursive_directory_iterator(dir))if(e.is_regular_file()){auto s=e.path().filename().string();SendMessageA(assets,LB_ADDSTRING,0,(LPARAM)s.c_str());}}
static void loadScript(const std::string& path){scriptFile=path;std::ifstream f(path);std::stringstream ss;ss<<f.rdbuf();SetWindowTextA(scriptEdit,ss.str().c_str());setStatus("Script: "+path);}
static void saveScript(){if(scriptFile.empty()){char file[MAX_PATH]="Player.tiny";OPENFILENAMEA o{};o.lStructSize=sizeof(o);o.hwndOwner=mainWnd;o.lpstrFile=file;o.nMaxFile=MAX_PATH;o.lpstrFilter="TinyScript\0*.tiny\0\0";o.Flags=OFN_OVERWRITEPROMPT;if(!GetSaveFileNameA(&o))return;scriptFile=file;}int n=GetWindowTextLengthA(scriptEdit);std::string s(n,'\0');GetWindowTextA(scriptEdit,s.data(),n+1);std::ofstream f(scriptFile,std::ios::trunc);if(!f){setStatus("ERROR: could not save script.");return;}f<<s;setStatus("Script saved.");}
static void compileScript(){int n=GetWindowTextLengthA(scriptEdit);std::string s(n,'\0');GetWindowTextA(scriptEdit,s.data(),n+1);auto r=tinyscript::compile(s);if(r.ok())setStatus("TinyScript compiled successfully.");else{std::string msg="TinyScript errors: ";for(auto&e:r.diagnostics)msg+="line "+std::to_string(e.line)+": "+e.message+" | ";setStatus(msg);}}

static void buildGame(){
    if(projectFile.empty()){setStatus("Open a project before building.");return;}saveScene();ensureFolders();fs::path out=fs::path(projectDir)/"build";fs::create_directories(out);
    fs::path runtime=fs::path(GetModuleFileNameA?"":""); char exe[MAX_PATH]={};GetModuleFileNameA(nullptr,exe,MAX_PATH);fs::path editorPath=exe;fs::path runtimePath=editorPath.parent_path()/"TinyAIRuntime.exe";
    fs::path target=out/(fs::path(projectFile).stem().string()+".exe");bool copied=CopyFileA(runtimePath.string().c_str(),target.string().c_str(),FALSE)!=0;fs::copy_file(projectFile,out/fs::path(projectFile).filename(),fs::copy_options::overwrite_existing);if(copied)setStatus("Build complete: "+target.string());else setStatus("Build scene package created, but TinyAIRuntime.exe was not beside the editor.");
}

static LRESULT CALLBACK viewportProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_LBUTTONDOWN){SetCapture(h);dragStart={GET_X_LPARAM(l),GET_Y_LPARAM(l)};if(w&MK_SHIFT)panMode=true;else{RECT r;GetClientRect(h,&r);int hit=hitTest(dragStart.x,dragStart.y,r);selected=hit;refreshHierarchy();}return 0;}
    if(m==WM_LBUTTONUP){ReleaseCapture();panMode=false;return 0;}
    if(m==WM_RBUTTONDOWN){SetCapture(h);dragStart={GET_X_LPARAM(l),GET_Y_LPARAM(l)};mouseLook=true;return 0;}
    if(m==WM_RBUTTONUP){ReleaseCapture();mouseLook=false;return 0;}
    if(m==WM_MOUSEMOVE){int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);if(mouseLook){yaw+=(x-dragStart.x)*.008f;pitch=std::clamp(pitch+(y-dragStart.y)*.008f,-1.45f,1.45f);dragStart={x,y};InvalidateRect(h,nullptr,FALSE);}else if(panMode){panX-=(x-dragStart.x)*.012f;panY+=(y-dragStart.y)*.012f;dragStart={x,y};InvalidateRect(h,nullptr,FALSE);}return 0;}
    if(m==WM_MOUSEWHEEL){distance*=powf(.9f,(float)GET_WHEEL_DELTA_WPARAM(w)/120.f);distance=std::clamp(distance,2.f,120.f);InvalidateRect(h,nullptr,FALSE);return 0;}
    if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);HBRUSH bg=CreateSolidBrush(RGB(25,28,34));FillRect(dc,&r,bg);DeleteObject(bg);drawGrid(dc,r);for(int i=0;i<(int)scene.size();i++){auto&o=scene[i];if(o.name=="Main Camera")continue;if(o.type==1)fillSphere(dc,o,r,i==selected);else if(o.type==2)fillPlane(dc,o,r,i==selected);else fillBox(dc,o,r,i==selected);}SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(190,200,215));TextOutA(dc,12,10,"Perspective",10);TextOutA(dc,12,30,"RMB Orbit   Shift+LMB Pan   Wheel Zoom   Click Select",47);if(playing){SetTextColor(dc,RGB(255,190,70));TextOutA(dc,12,52,"PLAY MODE",9);}EndPaint(h,&ps);return 0;}return DefWindowProcA(h,m,w,l);
}

static void showHub();
static void showEditor(){hub=false;playing=false;ShowWindow(viewport,SW_SHOW);ShowWindow(hierarchy,SW_SHOW);ShowWindow(assets,SW_SHOW);ShowWindow(scriptEdit,SW_SHOW);for(HWND h:{nameEdit,xEdit,yEdit,zEdit,rxEdit,ryEdit,rzEdit,sxEdit,syEdit,szEdit})ShowWindow(h,SW_SHOW);for(int id:{ID_NEW,ID_OPEN,ID_ADD_CUBE,ID_ADD_SPHERE,ID_ADD_PLANE,ID_DELETE,ID_DUP,ID_SAVE,ID_LOAD,ID_PLAY,ID_STOP,ID_HUB,ID_APPLY,ID_FOCUS,ID_BUILD,ID_SCRIPT_SAVE,ID_SCRIPT_COMPILE,ID_SCRIPT_NEW})ShowWindow(GetDlgItem(mainWnd,id),SW_SHOW);for(int id:{ID_RECENT,ID_SETTINGS})ShowWindow(GetDlgItem(mainWnd,id),SW_HIDE);refreshHierarchy();populateAssets();}
static void showHub(){hub=true;playing=false;for(int id:{ID_NEW,ID_OPEN,ID_RECENT,ID_SETTINGS})ShowWindow(GetDlgItem(mainWnd,id),SW_SHOW);for(int id:{ID_ADD_CUBE,ID_ADD_SPHERE,ID_ADD_PLANE,ID_DELETE,ID_DUP,ID_SAVE,ID_LOAD,ID_PLAY,ID_STOP,ID_HUB,ID_APPLY,ID_FOCUS,ID_BUILD,ID_SCRIPT_SAVE,ID_SCRIPT_COMPILE,ID_SCRIPT_NEW})ShowWindow(GetDlgItem(mainWnd,id),SW_HIDE);ShowWindow(viewport,SW_HIDE);ShowWindow(hierarchy,SW_HIDE);ShowWindow(assets,SW_HIDE);ShowWindow(scriptEdit,SW_HIDE);for(HWND h:{nameEdit,xEdit,yEdit,zEdit,rxEdit,ryEdit,rzEdit,sxEdit,syEdit,szEdit})ShowWindow(h,SW_HIDE);setStatus("Tiny AI Game Engine Hub");InvalidateRect(mainWnd,nullptr,TRUE);}

static void createEditorControls(){
    button("New",ID_NEW,10,8,70);button("Open",ID_OPEN,85,8,70);button("Add Cube",ID_ADD_CUBE,165,8,78);button("Sphere",ID_ADD_SPHERE,248,8,70);button("Plane",ID_ADD_PLANE,323,8,65);button("Delete",ID_DELETE,393,8,65);button("Duplicate",ID_DUP,463,8,75);button("Save",ID_SAVE,543,8,65);button("Play",ID_PLAY,613,8,65);button("Stop",ID_STOP,683,8,65);button("Build",ID_BUILD,753,8,70);button("Hub",ID_HUB,828,8,60);button("Focus",ID_FOCUS,893,8,65);
    hierarchy=CreateWindowA("LISTBOX","",WS_CHILD|LBS_NOTIFY|WS_BORDER|WS_VSCROLL,8,55,225,310,mainWnd,(HMENU)ID_HIER,GetModuleHandleA(nullptr),nullptr);
    assets=CreateWindowA("LISTBOX","",WS_CHILD|WS_BORDER|WS_VSCROLL,8,380,225,170,mainWnd,(HMENU)ID_ASSETS,GetModuleHandleA(nullptr),nullptr);
    viewport=CreateWindowA("TinyAIViewport","",WS_CHILD|WS_BORDER,240,55,610,495,mainWnd,nullptr,GetModuleHandleA(nullptr),nullptr);
    label("SCENE",15,38,80);label("ASSETS",15,363,80);label("INSPECTOR",865,38,100);
    label("Name",865,65);nameEdit=makeEdit("",925,62,190,24);label("Position",865,95,70);xEdit=makeEdit("0",925,92,55,24);yEdit=makeEdit("0",985,92,55,24);zEdit=makeEdit("0",1045,92,55,24);
    label("Rotation",865,130,70);rxEdit=makeEdit("0",925,127,55,24);ryEdit=makeEdit("0",985,127,55,24);rzEdit=makeEdit("0",1045,127,55,24);
    label("Scale",865,165,70);sxEdit=makeEdit("1",925,162,55,24);syEdit=makeEdit("1",985,162,55,24);szEdit=makeEdit("1",1045,162,55,24);button("Apply",ID_APPLY,925,198,100);
    label("SCRIPT",865,238,80);scriptEdit=CreateWindowA("EDIT","",WS_CHILD|WS_BORDER|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|WS_HSCROLL,865,265,250,250,mainWnd,nullptr,GetModuleHandleA(nullptr),nullptr);button("New Script",ID_SCRIPT_NEW,865,525,78);button("Save",ID_SCRIPT_SAVE,948,525,60);button("Compile",ID_SCRIPT_COMPILE,1013,525,70);
    output=CreateWindowA("EDIT","",WS_CHILD|WS_BORDER|ES_READONLY|ES_MULTILINE,240,558,875,75,mainWnd,nullptr,GetModuleHandleA(nullptr),nullptr);
}

static LRESULT CALLBACK mainProc(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_COMMAND){int id=LOWORD(w);if(id==ID_HIER&&HIWORD(w)==LBN_SELCHANGE){selected=(int)SendMessageA(hierarchy,LB_GETCURSEL,0,0);refreshHierarchy();return 0;}switch(id){case ID_NEW:newProject();break;case ID_OPEN:openProject();break;case ID_ADD_CUBE:addObj(0);break;case ID_ADD_SPHERE:addObj(1);break;case ID_ADD_PLANE:addObj(2);break;case ID_DELETE:deleteObj();break;case ID_DUP:duplicateObj();break;case ID_SAVE:saveScene();break;case ID_LOAD:loadScene();break;case ID_APPLY:applyInspector();break;case ID_FOCUS:if(selected>=0){panX=scene[selected].x;panY=scene[selected].y;distance=8;}InvalidateRect(viewport,nullptr,FALSE);break;case ID_PLAY:playing=true;lastTick=GetTickCount64();setStatus("PLAY MODE: physics simulation running.");break;case ID_STOP:playing=false;setStatus("Editor mode.");break;case ID_HUB:showHub();break;case ID_BUILD:buildGame();break;case ID_SCRIPT_SAVE:saveScript();break;case ID_SCRIPT_COMPILE:compileScript();break;case ID_SCRIPT_NEW:{scriptFile.clear();SetWindowTextA(scriptEdit,"# TinyScript\n\nstart:\n    say \"Hello from Tiny AI!\"\n\nupdate:\n    if key W\n        move 0 0 5 * time\n");setStatus("New TinyScript created.");break;}case ID_RECENT:setStatus("Recent projects are represented by the last opened project in this build.");break;case ID_SETTINGS:setStatus("Settings: Windows x64, 3D editor, TinyScript.");break;}}
    if(m==WM_TIMER&&playing){ULONGLONG now=GetTickCount64();float dt=std::min(.05f,float(now-lastTick)/1000.f);lastTick=now;for(auto&o:scene)if(o.dynamic&&o.name!="Main Camera"){o.y-=9.8f*dt;if(o.y<1){o.y=1;}}InvalidateRect(viewport,nullptr,FALSE);}
    if(m==WM_KEYDOWN){if(w==VK_DELETE)deleteObj();if(w=='F'&&selected>=0){panX=scene[selected].x;panY=scene[selected].y;distance=8;InvalidateRect(viewport,nullptr,FALSE);}}
    if(m==WM_SIZE){int W=LOWORD(l),H=HIWORD(l);MoveWindow(hierarchy,8,55,225,(H-110)/2,TRUE);MoveWindow(assets,8,60+(H-110)/2,225,(H-110)/2,TRUE);MoveWindow(viewport,240,55,W-490,H-145,TRUE);int ix=W-245;MoveWindow(nameEdit,ix+60,62,175,24,TRUE);MoveWindow(xEdit,ix+60,92,55,24,TRUE);MoveWindow(yEdit,ix+120,92,55,24,TRUE);MoveWindow(zEdit,ix+180,92,55,24,TRUE);MoveWindow(rxEdit,ix+60,127,55,24,TRUE);MoveWindow(ryEdit,ix+120,127,55,24,TRUE);MoveWindow(rzEdit,ix+180,127,55,24,TRUE);MoveWindow(sxEdit,ix+60,162,55,24,TRUE);MoveWindow(syEdit,ix+120,162,55,24,TRUE);MoveWindow(szEdit,ix+180,162,55,24,TRUE);MoveWindow(scriptEdit,ix,265,235,H-350,TRUE);MoveWindow(output,240,H-82,W-250,70,TRUE);}
    if(m==WM_CLOSE){if(dirty&&MessageBoxA(h,"The scene has unsaved changes. Exit anyway?","Tiny AI",MB_YESNO|MB_ICONWARNING)!=IDYES)return 0;DestroyWindow(h);}if(m==WM_DESTROY)PostQuitMessage(0);if(m==WM_PAINT&&hub){PAINTSTRUCT ps;HDC dc=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);HBRUSH b=CreateSolidBrush(RGB(22,25,31));FillRect(dc,&r,b);DeleteObject(b);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,RGB(235,240,248));HFONT big=CreateFontA(42,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,"Segoe UI");HGDIOBJ old=SelectObject(dc,big);TextOutA(dc,70,75,"TINY AI",-1);SelectObject(dc,old);DeleteObject(big);SetTextColor(dc,RGB(150,160,175));TextOutA(dc,74,125,"3D GAME ENGINE",-1);TextOutA(dc,74,165,"Create a project or open an existing one.",-1);EndPaint(h,&ps);return 0;}return DefWindowProcA(h,m,w,l);
}

int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int){
    WNDCLASSA c{};c.hInstance=hi;c.lpfnWndProc=mainProc;c.lpszClassName="TinyAIEditor";c.hCursor=LoadCursor(nullptr,IDC_ARROW);c.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassA(&c);WNDCLASSA v{};v.hInstance=hi;v.lpfnWndProc=viewportProc;v.lpszClassName="TinyAIViewport";v.hCursor=LoadCursor(nullptr,IDC_CROSS);RegisterClassA(&v);
    mainWnd=CreateWindowA(c.lpszClassName,"Tiny AI Game Engine",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,60,1160,760,nullptr,nullptr,hi,nullptr);createEditorControls();SetTimer(mainWnd,1,16,nullptr);showHub();MSG msg{};while(GetMessageA(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageA(&msg);}return 0;
}
