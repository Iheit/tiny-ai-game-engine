#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace tiny {
using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum class EntityKind { Cube, Sphere, Plane, Camera, DirectionalLight, PointLight, SpotLight };
struct Transform { XMFLOAT3 position{0,0,0}; XMFLOAT3 rotation{0,0,0}; XMFLOAT3 scale{1,1,1}; };
struct Material { XMFLOAT4 color{0.65f,0.72f,0.86f,1}; float roughness=.55f; float metallic=0; std::wstring texture; };
struct Light { XMFLOAT3 color{1,1,1}; float intensity=1; float range=10; float inner=20; float outer=35; bool shadows=false; };
struct Entity { uint64_t id=0; std::string name; EntityKind kind=EntityKind::Cube; Transform transform{}; Material material{}; Light light{}; bool dynamic=false; bool visible=true; std::string script; };
struct Scene { std::string name="Main"; uint64_t nextId=1; std::vector<Entity> entities; uint64_t cameraId=0; };

std::string kindName(EntityKind);
std::string serialize(const Scene&);
bool deserialize(const std::string&, Scene&, std::string& error);

class Graphics {
public:
 bool initialize(HWND hwnd, int width, int height, bool vsync=true);
 void resize(int width,int height);
 void beginFrame(const float clear[4]);
 void drawScene(const Scene& scene, const XMMATRIX& view, const XMMATRIX& proj);
 void endFrame();
 void setVsync(bool v){vsync_=v;}
 bool ready() const{return device_!=nullptr;}
 ID3D11Device* device() const{return device_.Get();}
private:
 bool makeShaders(); bool makeBuffers(); void drawEntity(const Entity&,const XMMATRIX&,const XMMATRIX&,const XMMATRIX&);
 ComPtr<ID3D11Device> device_; ComPtr<ID3D11DeviceContext> context_; ComPtr<IDXGISwapChain> swap_; ComPtr<ID3D11RenderTargetView> rtv_; ComPtr<ID3D11DepthStencilView> dsv_; ComPtr<ID3D11Texture2D> depth_;
 ComPtr<ID3D11VertexShader> vs_; ComPtr<ID3D11PixelShader> ps_; ComPtr<ID3D11InputLayout> layout_; ComPtr<ID3D11Buffer> vb_; ComPtr<ID3D11Buffer> cb_;
 HWND hwnd_{}; int width_=1,height_=1; bool vsync_=true;
};

class Engine {
public:
 bool initialize(HWND hwnd,int w,int h); void resize(int w,int h); void tick(float dt); void render(); void play(bool v); bool playing()const{return playing_;}
 Scene& scene(){return scene_;} const Scene& scene()const{return scene_;} Graphics& graphics(){return gfx_;}
 void setInput(HWND h){input_=h;} bool keyDown(int vk) const{return (GetAsyncKeyState(vk)&0x8000)!=0;}
private: void simulate(float dt); XMMATRIX cameraView() const; XMMATRIX cameraProjection() const;
 HWND input_{}; Graphics gfx_; Scene scene_; bool playing_=false; float time_=0; float yaw_=0.0f;
};

bool writeUtf8File(const std::wstring&,const std::string&); bool readUtf8File(const std::wstring&,std::string&); std::wstring widen(const std::string&); std::string narrow(const std::wstring&);
}
