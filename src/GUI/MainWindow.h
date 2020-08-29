#pragma once

#include <Windows.h>
#include <thread>
#include <atomic>
#include <map>

namespace Ancorage::GUI
{
class MainWindow
{
public:
    MainWindow(HINSTANCE inst, int cmdShow);
    bool Init();
    bool Run();
    bool Join();
    HWND GetHwnd();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    void threadProc();

    HINSTANCE inst_ = nullptr;
    HWND hwnd_ = nullptr;
    int cmdShow_ = 0;
    PCTSTR className_ = L"myWindowClass";
    std::thread t_;
    WNDCLASSEX wc_ = {};
    std::atomic<bool> created_{false};
    HWND controllerDialog_ = nullptr;
    std::map<int64_t, int> buttonEvents_;
};
} // namespace Ancorage::GUI
