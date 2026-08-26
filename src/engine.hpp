#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace Tiny3D {
struct Vec3 { float x{}, y{}, z{}; };
struct Color { uint8_t r{}, g{}, b{}, a{255}; };
struct Vertex { Vec3 p; Color c; };
struct Triangle { Vertex a,b,c; };

inline Vec3 operator+(Vec3 a, Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline Vec3 operator-(Vec3 a, Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline Vec3 operator*(Vec3 a,float s){return {a.x*s,a.y*s,a.z*s};}
inline float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline float length(Vec3 a){return std::sqrt(dot(a,a));}
inline Vec3 normalize(Vec3 a){float l=length(a); return l>0.0001f?a*(1.0f/l):Vec3{};}
inline Vec3 rotateY(Vec3 p,float a){float c=std::cos(a),s=std::sin(a);return {p.x*c-p.z*s,p.y,p.x*s+p.z*c};}
inline Vec3 rotateX(Vec3 p,float a){float c=std::cos(a),s=std::sin(a);return {p.x,p.y*c-p.z*s,p.y*s+p.z*c};}

class Renderer {
public:
    Renderer(int w,int h):w_(w),h_(h),pixels_(w*h),z_(w*h){}
    void resize(int w,int h){w_=w;h_=h;pixels_.resize(w*h);z_.resize(w*h);}
    void clear(Color c){uint32_t v=(uint32_t(c.a)<<24)|(uint32_t(c.r)<<16)|(uint32_t(c.g)<<8)|c.b; std::fill(pixels_.begin(),pixels_.end(),v); std::fill(z_.begin(),z_.end(),1e30f);}
    void drawTriangle(Vertex va,Vertex vb,Vertex vc,float fov=1.0f,float nearZ=0.1f){
        auto project=[&](Vertex v){
            float z=v.p.z; if(z<=nearZ) z=nearZ;
            float sx=(v.p.x/z*fov*0.5f+0.5f)*float(w_);
            float sy=(-v.p.y/z*fov*0.5f+0.5f)*float(h_);
            return P{sx,sy,z,v.c};
        };
        P a=project(va),b=project(vb),c=project(vc);
        float minx=std::max(0.0f,std::floor(std::min({a.x,b.x,c.x}))), maxx=std::min(float(w_-1),std::ceil(std::max({a.x,b.x,c.x})));
        float miny=std::max(0.0f,std::floor(std::min({a.y,b.y,c.y}))), maxy=std::min(float(h_-1),std::ceil(std::max({a.y,b.y,c.y})));
        float area=edge(a,b,c.x,c.y); if(std::abs(area)<0.0001f)return;
        for(int y=int(miny);y<=int(maxy);++y) for(int x=int(minx);x<=int(maxx);++x){
            float w0=edge(b,c,float(x)+.5f,float(y)+.5f),w1=edge(c,a,float(x)+.5f,float(y)+.5f),w2=edge(a,b,float(x)+.5f,float(y)+.5f);
            if((w0>=0&&w1>=0&&w2>=0)||(w0<=0&&w1<=0&&w2<=0)){
                w0/=area;w1/=area;w2/=area; float z=1.0f/(w0/a.z+w1/b.z+w2/c.z); int i=y*w_+x;
                if(z<z_[i]){z_[i]=z; Color col{uint8_t(std::clamp(w0*a.c.r+w1*b.c.r+w2*c.c.r,0.0f,255.0f)),uint8_t(std::clamp(w0*a.c.g+w1*b.c.g+w2*c.c.g,0.0f,255.0f)),uint8_t(std::clamp(w0*a.c.b+w1*b.c.b+w2*c.c.b,0.0f,255.0f)),255};pixels_[i]=(uint32_t(col.a)<<24)|(uint32_t(col.r)<<16)|(uint32_t(col.g)<<8)|col.b;}
            }
        }
    }
    void present(HDC dc){BITMAPINFO bi{};bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);bi.bmiHeader.biWidth=w_;bi.bmiHeader.biHeight=-h_;bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;StretchDIBits(dc,0,0,w_,h_,0,0,w_,h_,pixels_.data(),&bi,DIB_RGB_COLORS,SRCCOPY);}
private:
    struct P{float x,y,z;Color c;};
    static float edge(const P&a,const P&b,float x,float y){return (x-a.x)*(b.y-a.y)-(y-a.y)*(b.x-a.x);}
    int w_,h_; std::vector<uint32_t> pixels_; std::vector<float> z_;
};

class Engine {
public:
    Engine(int width=960,int height=540):width_(width),height_(height),renderer_(width,height){}
    bool run();
    virtual void onStart(){}
    virtual void onUpdate(float){}
    virtual void onRender(Renderer&){}
    bool keyDown(int vk) const{return (GetAsyncKeyState(vk)&0x8000)!=0;}
    int width()const{return width_;} int height()const{return height_;}
private:
    static LRESULT CALLBACK wndProc(HWND,UINT,WPARAM,LPARAM);
    bool createWindow(); void render();
    HWND hwnd_{}; int width_,height_; Renderer renderer_; LARGE_INTEGER freq_{},last_{}; bool running_{};
    static Engine* instance_;
};
}
