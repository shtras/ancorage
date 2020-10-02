#pragma once

#include "ControlPlus/Profile.h"

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
    void valueChange(int v, int delta);
    void valueSet(int v, int val);

    HINSTANCE inst_ = nullptr;
    HWND hwnd_ = nullptr;
    int cmdShow_ = 0;
    PCTSTR className_ = L"myWindowClass";
    std::thread t_;
    WNDCLASSEX wc_ = {};
    std::atomic<bool> created_{false};
    HWND controllerDialog_ = nullptr;
    std::map<int64_t, int> buttonEvents_;
    std::unique_ptr<ControlPlus::Profile> profile_ = std::make_unique<ControlPlus::Profile>();
    std::map<int, int> values_;
    int gear_ = 0;
    std::map<WPARAM, std::wstring> connectEvents_;
    std::map<WPARAM, std::wstring> disconnectEvents_;
};
} // namespace Ancorage::GUI
