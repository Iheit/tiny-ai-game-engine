#include "engine.hpp"
using namespace Tiny3D;
class Demo final: public Engine{
    float angle_=0, camZ_=3.5f;
    std::vector<Vec3> cube_{{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
    Color cols_[8]={{230,70,70,255},{70,220,100,255},{70,130,240,255},{240,210,70,255},{180,80,230,255},{60,210,210,255},{240,130,60,255},{210,210,210,255}};
public:
 void onUpdate(float dt) override {angle_+=dt; if(keyDown(VK_UP))camZ_-=dt*2; if(keyDown(VK_DOWN))camZ_+=dt*2; camZ_=std::clamp(camZ_,1.5f,8.0f);}
 void onRender(Renderer& r) override {
  int faces[12][3]={{0,1,2},{0,2,3},{1,5,6},{1,6,2},{5,4,7},{5,7,6},{4,0,3},{4,3,7},{3,2,6},{3,6,7},{4,5,1},{4,1,0}};
  Vertex v[8]; for(int i=0;i<8;i++){Vec3 p=rotateY(rotateX(cube_[i],angle_*0.6f),angle_);p.z+=camZ_;v[i]={p,cols_[i]};}
  for(auto& f:faces)r.drawTriangle(v[f[0]],v[f[1]],v[f[2]],1.15f);
 }
};
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){Demo app(960,540);return app.run()?0:1;}
