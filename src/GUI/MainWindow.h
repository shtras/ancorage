#pragma once

#include <Windows.h>
#include <thread>

namespace Ancorage::GUI
{
class MainWindow
{
public:
    MainWindow(HINSTANCE inst, int cmdShow);
    bool Init();
    bool Run();
    bool Join();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    void threadProc();

    HINSTANCE inst_ = nullptr;
    int cmdShow_ = 0;
    PCTSTR className_ = L"myWindowClass";
    std::thread t_;
};
} // namespace Ancorage::GUI
