#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include "engine.hpp"

namespace fs = std::filesystem;
using namespace tiny;

namespace {
constexpr int ID_NEW = 1001;
constexpr int ID_OPEN = 1002;
constexpr int ID_SAVE = 1003;
constexpr int ID_BUILD = 1004;
constexpr int ID_PLAY = 1005;
constexpr int ID_STOP = 1006;
constexpr int ID_ADD_CUBE = 1101;
constexpr int ID_ADD_SPHERE = 1102;
constexpr int ID_ADD_PLANE = 1103;
constexpr int ID_ADD_CAMERA = 1104;
constexpr int ID_ADD_DIRECTIONAL = 1105;
constexpr int ID_ADD_POINT = 1106;
constexpr int ID_ADD_SPOT = 1107;
constexpr int ID_DELETE = 1108;
constexpr int ID_DUPLICATE = 1109;
constexpr int ID_FOCUS = 1110;
constexpr int ID_APPLY = 1201;
constexpr int ID_SCENE = 1301;
constexpr int ID_RECENT = 1401;
constexpr int ID_HUB_OPEN_SELECTED = 1402;

HINSTANCE gInstance{};
HWND gMain{}, gHub{}, gViewport{}, gScene{}, gOut{}, gInspectorName{};
std::array<HWND, 9> gTransform{};
Engine gEngine;
std::wstring gProject;
int gSelected = -1;
bool gInEditor = false;

HWND child(const char* cls, const char* text, DWORD style, int x, int y, int w, int h,
           HWND parent, int id = 0) {
    return CreateWindowExA(0, cls, text, style, x, y, w, h, parent,
        id ? reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)) : nullptr,
        gInstance, nullptr);
}

void show(HWND h, bool visible) {
    if (h) ShowWindow(h, visible ? SW_SHOW : SW_HIDE);
}

void setOutput(const std::string& text) {
    if (gOut) SetWindowTextA(gOut, text.c_str());
}

fs::path recentFile() {
    wchar_t appData[32768]{};
    DWORD n = GetEnvironmentVariableW(L"APPDATA", appData, static_cast<DWORD>(std::size(appData)));
    fs::path base = n ? fs::path(appData) : fs::current_path();
    base /= L"TinyAI";
    std::error_code ec;
    fs::create_directories(base, ec);
    return base / L"recent.txt";
}

std::vector<std::wstring> readRecent() {
    std::vector<std::wstring> result;
    std::ifstream in(recentFile(), std::ios::binary);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        fs::path p = fs::path(widen(line));
        if (fs::exists(p)) result.push_back(p.wstring());
    }
    return result;
}

void rememberRecent(const std::wstring& project) {
    auto items = readRecent();
    items.erase(std::remove(items.begin(), items.end(), project), items.end());
    items.insert(items.begin(), project);
    if (items.size() > 8) items.resize(8);
    std::ofstream out(recentFile(), std::ios::binary | std::ios::trunc);
    for (const auto& p : items) out << narrow(p) << '\n';
}

void refreshRecent() {
    HWND list = GetDlgItem(gHub, ID_RECENT);
    if (!list) return;
    SendMessageA(list, LB_RESETCONTENT, 0, 0);
    for (const auto& p : readRecent())
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(p.c_str()));
}

void refreshScene() {
    if (!gScene) return;
    SendMessageA(gScene, LB_RESETCONTENT, 0, 0);
    for (const auto& e : gEngine.scene().entities) {
        std::string text = e.name + "   [" + kindName(e.kind) + "]";
        SendMessageA(gScene, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
    if (gSelected >= 0 && gSelected < static_cast<int>(gEngine.scene().entities.size())) {
        SendMessageA(gScene, LB_SETCURSEL, gSelected, 0);
        const Entity& e = gEngine.scene().entities[gSelected];
        SetWindowTextA(gInspectorName, e.name.c_str());
        const float values[] = {
            e.transform.position.x, e.transform.position.y, e.transform.position.z,
            e.transform.rotation.x, e.transform.rotation.y, e.transform.rotation.z,
            e.transform.scale.x, e.transform.scale.y, e.transform.scale.z
        };
        char buf[64];
        for (size_t i = 0; i < gTransform.size(); ++i) {
            sprintf_s(buf, "%.3f", values[i]);
            SetWindowTextA(gTransform[i], buf);
        }
    }
}

bool initializeEngine() {
    if (gEngine.graphics().ready()) return true;
    RECT r{};
    GetClientRect(gViewport, &r);
    int w = std::max(1, r.right - r.left);
    int h = std::max(1, r.bottom - r.top);
    if (!gEngine.initialize(gViewport, w, h)) {
        MessageBoxA(gMain, "Direct3D 11 could not initialize the editor viewport.", "Tiny AI Game Engine", MB_OK | MB_ICONERROR);
        return false;
    }
    gEngine.setInput(gViewport);
    return true;
}

bool loadProject() {
    std::string data, error;
    if (!readUtf8File(gProject, data)) {
        setOutput("Could not read the project file.");
        return false;
    }
    if (!deserialize(data, gEngine.scene(), error)) {
        setOutput("Project load failed: " + error);
        return false;
    }
    gSelected = gEngine.scene().entities.empty() ? -1 : 0;
    rememberRecent(gProject);
    refreshScene();
    setOutput("Opened " + narrow(gProject));
    return true;
}

bool saveProject() {
    if (gProject.empty()) {
        setOutput("No project is open.");
        return false;
    }
    if (!writeUtf8File(gProject, serialize(gEngine.scene()))) {
        setOutput("Could not save the project.");
        return false;
    }
    rememberRecent(gProject);
    setOutput("Saved " + narrow(gProject));
    return true;
}

void showEditor() {
    gInEditor = true;
    show(gHub, false);
    show(gScene, true);
    show(gViewport, true);
    show(gOut, true);
    show(gInspectorName, true);
    for (HWND h : gTransform) show(h, true);
    RECT r{};
    GetClientRect(gMain, &r);
    int W = r.right, H = r.bottom;
    const int left = 238, right = 302, top = 72, bottom = 122;
    int vw = std::max(220, W - left - right);
    int vh = std::max(180, H - top - bottom);
    MoveWindow(gViewport, left, top, vw, vh, TRUE);
    MoveWindow(gScene, 10, 90, left - 20, 330, TRUE);
    MoveWindow(gOut, left, H - bottom + 28, vw, bottom - 36, TRUE);
    MoveWindow(gInspectorName, W - right + 70, 95, right - 92, 24, TRUE);
    for (int i = 0; i < 9; ++i) {
        int col = i % 3, row = i / 3;
        MoveWindow(gTransform[i], W - right + 35 + col * 82, 140 + row * 50, 70, 24, TRUE);
    }
    if (gEngine.graphics().ready()) gEngine.resize(vw, vh);
}

void showHub() {
    gInEditor = false;
    show(gHub, true);
    show(gScene, false);
    show(gViewport, false);
    show(gOut, false);
    show(gInspectorName, false);
    for (HWND h : gTransform) show(h, false);
    RECT r{};
    GetClientRect(gMain, &r);
    int W = r.right, H = r.bottom;
    int panelW = std::min(760, std::max(520, W - 80));
    int panelH = std::min(540, std::max(440, H - 100));
    MoveWindow(gHub, (W - panelW) / 2, 48, panelW, panelH, TRUE);
    refreshRecent();
}

void createProject() {
    char file[MAX_PATH] = "MyGame.tinyproj";
    OPENFILENAMEA dlg{};
    dlg.lStructSize = sizeof(dlg);
    dlg.hwndOwner = gMain;
    dlg.lpstrFile = file;
    dlg.nMaxFile = MAX_PATH;
    dlg.lpstrFilter = "Tiny Project\0*.tinyproj\0\0";
    dlg.Flags = OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&dlg)) return;
    gProject = widen(file);
    if (!initializeEngine()) return;
    showEditor();
    saveProject();
    refreshScene();
    SetWindowTextA(gMain, ("Tiny AI Game Engine - " + narrow(gProject)).c_str());
}

void openProjectPath(const std::wstring& path) {
    gProject = path;
    if (!initializeEngine()) return;
    if (!loadProject()) return;
    showEditor();
    SetWindowTextA(gMain, ("Tiny AI Game Engine - " + narrow(gProject)).c_str());
}

void openProjectDialog() {
    char file[MAX_PATH]{};
    OPENFILENAMEA dlg{};
    dlg.lStructSize = sizeof(dlg);
    dlg.hwndOwner = gMain;
    dlg.lpstrFile = file;
    dlg.nMaxFile = MAX_PATH;
    dlg.lpstrFilter = "Tiny Project\0*.tinyproj\0All Files\0*.*\0\0";
    dlg.Flags = OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&dlg)) openProjectPath(widen(file));
}

void buildGame() {
    if (!saveProject()) return;
    fs::path output = fs::path(gProject).parent_path() / "build";
    std::error_code ec;
    fs::create_directories(output, ec);
    wchar_t module[MAX_PATH]{};
    GetModuleFileNameW(nullptr, module, MAX_PATH);
    fs::path runtime = fs::path(module).parent_path() / L"TinyAIRuntime.exe";
    if (!fs::exists(runtime)) {
        setOutput("Runtime executable not found beside the editor.");
        return;
    }
    fs::copy_file(runtime, output / L"Game.exe", fs::copy_options::overwrite_existing, ec);
    if (ec) { setOutput("Failed to copy Game.exe."); return; }
    fs::copy_file(gProject, output / L"game.tinyproj", fs::copy_options::overwrite_existing, ec);
    if (ec) { setOutput("Failed to copy game.tinyproj."); return; }
    setOutput("Standalone game built in " + narrow(output.wstring()));
}

void addEntity(EntityKind kind) {
    Entity e;
    e.id = gEngine.scene().nextId++;
    e.kind = kind;
    int count = 1;
    for (const auto& q : gEngine.scene().entities) if (q.kind == kind) ++count;
    e.name = kindName(kind) + " " + std::to_string(count);
    if (kind == EntityKind::Plane) e.transform.scale = {5.0f, 0.1f, 5.0f};
    if (kind == EntityKind::Cube) { e.transform.position = {0, 1, 0}; e.dynamic = true; }
    if (kind == EntityKind::Sphere) e.transform.position = {2, 1, 0};
    if (kind == EntityKind::PointLight) e.transform.position = {2, 3, -2};
    if (kind == EntityKind::SpotLight) e.transform.position = {-2, 3, -2};
    gEngine.scene().entities.push_back(e);
    gSelected = static_cast<int>(gEngine.scene().entities.size()) - 1;
    refreshScene();
    setOutput(e.name + " added.");
}

void applyInspector() {
    if (gSelected < 0 || gSelected >= static_cast<int>(gEngine.scene().entities.size())) return;
    Entity& e = gEngine.scene().entities[gSelected];
    char name[128]{};
    GetWindowTextA(gInspectorName, name, static_cast<int>(std::size(name)));
    if (name[0]) e.name = name;
    auto read = [](HWND h, float fallback) {
        char text[64]{}; GetWindowTextA(h, text, static_cast<int>(std::size(text)));
        char* end = nullptr; float v = std::strtof(text, &end); return end == text ? fallback : v;
    };
    e.transform.position = {read(gTransform[0], e.transform.position.x), read(gTransform[1], e.transform.position.y), read(gTransform[2], e.transform.position.z)};
    e.transform.rotation = {read(gTransform[3], e.transform.rotation.x), read(gTransform[4], e.transform.rotation.y), read(gTransform[5], e.transform.rotation.z)};
    e.transform.scale = {std::max(0.01f, read(gTransform[6], e.transform.scale.x)), std::max(0.01f, read(gTransform[7], e.transform.scale.y)), std::max(0.01f, read(gTransform[8], e.transform.scale.z))};
    refreshScene();
    setOutput("Inspector changes applied.");
}

void selectViewport(int x, int y) {
    RECT r{}; GetClientRect(gViewport, &r);
    for (int i = 0; i < static_cast<int>(gEngine.scene().entities.size()); ++i) {
        const auto& e = gEngine.scene().entities[i];
        if (e.kind == EntityKind::Camera) continue;
        float sx = r.right * 0.5f + e.transform.position.x * 35.0f;
        float sy = r.bottom * 0.5f - e.transform.position.y * 35.0f;
        float dx = x - sx, dy = y - sy;
        if (dx * dx + dy * dy < 900.0f) {
            gSelected = i;
            refreshScene();
            setOutput("Selected " + e.name + ".");
            return;
        }
    }
}

LRESULT CALLBACK viewportProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_LBUTTONDOWN:
            SetFocus(hwnd);
            selectViewport(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_KEYDOWN:
            if (wp == VK_DELETE) PostMessageA(gMain, WM_COMMAND, ID_DELETE, 0);
            if (wp == 'F') PostMessageA(gMain, WM_COMMAND, ID_FOCUS, 0);
            return 0;
        case WM_SIZE:
            if (gEngine.graphics().ready()) gEngine.resize(std::max(1, LOWORD(lp)), std::max(1, HIWORD(lp)));
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK mainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_SIZE:
            if (gInEditor) showEditor(); else showHub();
            return 0;
        case WM_COMMAND: {
            int id = LOWORD(wp), code = HIWORD(wp);
            if (id == ID_SCENE && code == LBN_SELCHANGE) {
                int index = static_cast<int>(SendMessageA(gScene, LB_GETCURSEL, 0, 0));
                if (index != LB_ERR) { gSelected = index; refreshScene(); }
                return 0;
            }
            switch (id) {
                case ID_NEW: createProject(); break;
                case ID_OPEN: openProjectDialog(); break;
                case ID_SAVE: saveProject(); break;
                case ID_BUILD: buildGame(); break;
                case ID_PLAY: gEngine.play(true); setOutput("Play mode running."); break;
                case ID_STOP: gEngine.play(false); setOutput("Play mode stopped."); break;
                case ID_ADD_CUBE: addEntity(EntityKind::Cube); break;
                case ID_ADD_SPHERE: addEntity(EntityKind::Sphere); break;
                case ID_ADD_PLANE: addEntity(EntityKind::Plane); break;
                case ID_ADD_CAMERA: addEntity(EntityKind::Camera); break;
                case ID_ADD_DIRECTIONAL: addEntity(EntityKind::DirectionalLight); break;
                case ID_ADD_POINT: addEntity(EntityKind::PointLight); break;
                case ID_ADD_SPOT: addEntity(EntityKind::SpotLight); break;
                case ID_DELETE:
                    if (gSelected >= 0 && gSelected < static_cast<int>(gEngine.scene().entities.size())) {
                        if (gEngine.scene().entities[gSelected].id != gEngine.scene().cameraId) {
                            gEngine.scene().entities.erase(gEngine.scene().entities.begin() + gSelected);
                            gSelected = -1; refreshScene(); setOutput("Entity deleted.");
                        }
                    }
                    break;
                case ID_DUPLICATE:
                    if (gSelected >= 0 && gSelected < static_cast<int>(gEngine.scene().entities.size())) {
                        Entity e = gEngine.scene().entities[gSelected]; e.id = gEngine.scene().nextId++; e.name += " Copy"; e.transform.position.x += 1.0f;
                        gEngine.scene().entities.push_back(e); gSelected = static_cast<int>(gEngine.scene().entities.size()) - 1; refreshScene(); setOutput("Entity duplicated.");
                    }
                    break;
                case ID_FOCUS:
                    if (gSelected >= 0 && gSelected < static_cast<int>(gEngine.scene().entities.size())) {
                        const auto& e = gEngine.scene().entities[gSelected];
                        for (auto& c : gEngine.scene().entities) if (c.kind == EntityKind::Camera) { c.transform.position = {e.transform.position.x, e.transform.position.y + 2.0f, e.transform.position.z - 6.0f}; break; }
                        setOutput("Focused selection.");
                    }
                    break;
                case ID_APPLY: applyInspector(); break;
                case ID_HUB_OPEN_SELECTED: {
                    HWND list = GetDlgItem(gHub, ID_RECENT);
                    int i = static_cast<int>(SendMessageA(list, LB_GETCURSEL, 0, 0));
                    if (i != LB_ERR) { wchar_t path[32768]{}; SendMessageW(list, LB_GETTEXT, i, reinterpret_cast<LPARAM>(path)); openProjectPath(path); }
                    break;
                }
                default: break;
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (gInEditor && wp == 'S' && (GetKeyState(VK_CONTROL) & 0x8000)) saveProject();
            return 0;
        case WM_CLOSE: DestroyWindow(hwnd); return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

void buildHub() {
    gHub = child("STATIC", "", WS_CHILD | SS_CENTER, 0, 0, 700, 500, gMain);
    child("STATIC", "Tiny AI Game Engine", WS_CHILD | WS_VISIBLE | SS_CENTER, 70, 28, 620, 52, gHub);
    child("STATIC", "A small, beginner-friendly 3D editor", WS_CHILD | WS_VISIBLE | SS_CENTER, 90, 82, 580, 28, gHub);
    child("BUTTON", "New Project", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 150, 135, 180, 40, gHub, ID_NEW);
    child("BUTTON", "Open Project", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 135, 180, 40, gHub, ID_OPEN);
    child("STATIC", "Recent Projects", WS_CHILD | WS_VISIBLE | SS_LEFT, 150, 205, 220, 24, gHub);
    child("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, 150, 232, 400, 180, gHub, ID_RECENT);
    child("BUTTON", "Open Selected", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 150, 425, 135, 34, gHub, ID_HUB_OPEN_SELECTED);
    child("STATIC", "Tip: create a project, then use the toolbar, Scene tree, viewport and Inspector.", WS_CHILD | WS_VISIBLE | SS_CENTER, 90, 472, 580, 34, gHub);
}

void buildEditorUi() {
    child("BUTTON", "New", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 8, 8, 58, 28, gMain, ID_NEW);
    child("BUTTON", "Open", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 70, 8, 58, 28, gMain, ID_OPEN);
    child("BUTTON", "Save", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 132, 8, 58, 28, gMain, ID_SAVE);
    child("BUTTON", "Build", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 194, 8, 64, 28, gMain, ID_BUILD);
    child("BUTTON", "Play", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 262, 8, 58, 28, gMain, ID_PLAY);
    child("BUTTON", "Stop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 324, 8, 58, 28, gMain, ID_STOP);
    child("STATIC", "SCENE", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 64, 180, 20, gMain);
    gScene = child("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY | WS_VSCROLL, 10, 90, 210, 330, gMain, ID_SCENE);
    child("STATIC", "CREATE", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 430, 180, 20, gMain);
    struct B { const char* t; int id; } buttons[] = {
        {"Cube", ID_ADD_CUBE}, {"Sphere", ID_ADD_SPHERE}, {"Plane", ID_ADD_PLANE},
        {"Camera", ID_ADD_CAMERA}, {"Dir Light", ID_ADD_DIRECTIONAL}, {"Point", ID_ADD_POINT},
        {"Spot", ID_ADD_SPOT}, {"Duplicate", ID_DUPLICATE}, {"Delete", ID_DELETE}, {"Focus", ID_FOCUS}
    };
    for (int i = 0; i < static_cast<int>(std::size(buttons)); ++i) {
        int col = i % 2, row = i / 2;
        child("BUTTON", buttons[i].t, WS_CHILD | BS_PUSHBUTTON, 10 + col * 105, 455 + row * 32, 98, 27, gMain, buttons[i].id);
    }
    child("STATIC", "VIEWPORT", WS_CHILD | WS_VISIBLE | SS_LEFT, 242, 50, 120, 20, gMain);
    child("STATIC", "INSPECTOR", WS_CHILD | WS_VISIBLE | SS_LEFT, 930, 50, 180, 20, gMain);
    child("STATIC", "Name", WS_CHILD | WS_VISIBLE, 930, 78, 44, 20, gMain);
    gInspectorName = child("EDIT", "", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 980, 76, 245, 24, gMain);
    const char* names[] = {"X","Y","Z","RX","RY","RZ","SX","SY","SZ"};
    for (int i = 0; i < 9; ++i) {
        int col = i % 3, row = i / 3;
        int x = 932 + col * 96, y = 120 + row * 50;
        child("STATIC", names[i], WS_CHILD | WS_VISIBLE, x, y, 24, 20, gMain);
        gTransform[i] = child("EDIT", "0", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, x + 24, y - 2, 68, 24, gMain);
    }
    child("BUTTON", "Apply Changes", WS_CHILD | BS_PUSHBUTTON, 930, 285, 125, 30, gMain, ID_APPLY);
    child("STATIC", "OUTPUT", WS_CHILD | WS_VISIBLE | SS_LEFT, 242, 0, 100, 20, gMain);
    gOut = child("EDIT", "Ready. Create or open a project.", WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 242, 700, 650, 80, gMain);
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCmd) {
    gInstance = instance;
    WNDCLASSA mainClass{};
    mainClass.lpfnWndProc = mainProc;
    mainClass.hInstance = instance;
    mainClass.lpszClassName = "TinyAIEditorMain";
    mainClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    mainClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&mainClass);

    WNDCLASSA viewClass{};
    viewClass.lpfnWndProc = viewportProc;
    viewClass.hInstance = instance;
    viewClass.lpszClassName = "TinyAIEditorViewport";
    viewClass.hCursor = LoadCursor(nullptr, IDC_CROSS);
    viewClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassA(&viewClass);

    gMain = CreateWindowExA(0, mainClass.lpszClassName, "Tiny AI Game Engine", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            80, 40, 1280, 820, nullptr, nullptr, instance, nullptr);
    if (!gMain) return 1;

    gViewport = CreateWindowExA(0, viewClass.lpszClassName, "", WS_CHILD | WS_BORDER | WS_CLIPSIBLINGS,
                                238, 72, 700, 520, gMain, nullptr, instance, nullptr);
    buildEditorUi();
    buildHub();
    refreshRecent();
    showHub();
    ShowWindow(gMain, showCmd);
    UpdateWindow(gMain);

    auto last = std::chrono::steady_clock::now();
    MSG msg{};
    bool running = true;
    while (running) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;
        dt = std::min(dt, 0.1f);
        if (gInEditor && gEngine.graphics().ready()) {
            gEngine.tick(dt);
            gEngine.render();
        }
        Sleep(1);
    }
    return 0;
}
