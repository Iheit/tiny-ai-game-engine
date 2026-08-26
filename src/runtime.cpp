#include <windows.h>
#include <filesystem>
#include <string>
#include "engine.hpp"

using namespace tiny;
namespace fs = std::filesystem;

static Engine g;

static LRESULT CALLBACK wnd(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_SIZE) {
        if (g.graphics().ready()) {
            g.resize(LOWORD(l), HIWORD(l));
        }
        return 0;
    }
    if (m == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hi, HINSTANCE, LPSTR, int show) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = wnd;
    wc.hInstance = hi;
    wc.lpszClassName = L"TinyRuntime";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    HWND h = CreateWindowW(
        L"TinyRuntime",
        L"Tiny AI Game",
        WS_OVERLAPPEDWINDOW,
        120, 80, 960, 640,
        nullptr, nullptr, hi, nullptr);
    if (!h) {
        return 1;
    }

    ShowWindow(h, show);
    UpdateWindow(h);

    if (!g.initialize(h, 960, 640)) {
        DestroyWindow(h);
        return 1;
    }

    wchar_t modulePath[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        const fs::path projectPath = fs::path(modulePath).parent_path() / L"game.tinyproj";
        std::string data;
        if (readUtf8File(projectPath.wstring(), data)) {
            std::string error;
            if (!deserialize(data, g.scene(), error)) {
                MessageBoxA(h, error.c_str(), "Tiny AI Game", MB_OK | MB_ICONERROR);
                return 1;
            }
        }
    }

    MSG msg{};
    ULONGLONG last = GetTickCount64();
    while (msg.message != WM_QUIT) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        const ULONGLONG now = GetTickCount64();
        const float dt = static_cast<float>(now - last) / 1000.0f;
        last = now;
        g.play(true);
        g.tick(dt);
        g.render();
    }

    return 0;
}
