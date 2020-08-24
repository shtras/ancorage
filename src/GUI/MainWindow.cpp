#include "MainWindow.h"
#include "Utils/Utils.h"
#include "Resources/resource.h"

namespace Ancorage::GUI
{
LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    auto d = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto mw = reinterpret_cast<MainWindow*>(d);
    return mw->wndProc(hwnd, msg, wParam, lParam);
}

MainWindow::MainWindow(HINSTANCE inst, int cmdShow)
    : inst_(inst)
    , cmdShow_(cmdShow)
{
}

bool MainWindow::Init()
{
    wc_.cbSize = sizeof(wc_);
    wc_.style = 0;
    wc_.lpfnWndProc = &MainWindow::WndProc;
    wc_.cbClsExtra = 0;
    wc_.cbWndExtra = 0;
    wc_.hInstance = inst_;
    wc_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc_.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc_.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wc_.lpszClassName = className_;
    wc_.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc_)) {
        Utils::LogError(L"Could not register window class");
        return false;
    }
    return true;
}

bool MainWindow::Run()
{
    t_ = std::thread(&MainWindow::threadProc, this);
    return true;
}

bool MainWindow::Join()
{
    if (t_.joinable()) {
        t_.join();
    }
    return true;
}

void MainWindow::threadProc()
{
    auto hwnd =
        CreateWindowEx(WS_EX_LEFT, className_, L"My new window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, inst_, nullptr);
    if (!hwnd) {
        Utils::LogError(L"Could not create window");
        return;
    }
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd, cmdShow_);
    UpdateWindow(hwnd);
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

LRESULT MainWindow::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (msg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
} // namespace Ancorage::GUI
