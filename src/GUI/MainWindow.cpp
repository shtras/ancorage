#include "MainWindow.h"
#include "Utils/Utils.h"
#include "GUI/IDs.h"
#include "Utils/Utils.h"
#include "Resources/resource.h"

#include "spdlog_wrap.h"

#include <commctrl.h>

#include <sstream>

namespace Ancorage::GUI
{
LRESULT CALLBACK MainWindow::static_wndProc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    MainWindow* mw = nullptr;
    if (msg == WM_CREATE) {
        auto str = reinterpret_cast<CREATESTRUCT*>(lParam);
        mw = reinterpret_cast<MainWindow*>(str->lpCreateParams);
    } else {
        auto d = GetWindowLongPtr(hwnd, GWLP_USERDATA);
        mw = reinterpret_cast<MainWindow*>(d);
    }
    return mw->wndProc(hwnd, msg, wParam, lParam);
}

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM)
{
    switch (Message) {
        case WM_INITDIALOG:

            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    EndDialog(hwnd, IDOK);
                    break;
            }
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

MainWindow::MainWindow(HINSTANCE inst, int cmdShow)
    : inst_(inst)
    , cmdShow_(cmdShow)
{
    buttonEvents_ = {{12, IDC_BUTTON_A}, {13, IDC_BUTTON_B}, {14, IDC_BUTTON_X}, {15, IDC_BUTTON_Y},
        {0, IDC_BUTTON_UP}, {1, IDC_BUTTON_DOWN}, {2, IDC_BUTTON_LEFT}, {3, IDC_BUTTON_RIGHT},
        {8, IDC_BUTTON_LB}, {9, IDC_BUTTON_RB}, {6, IDC_BUTTON_L3}, {7, IDC_BUTTON_R3}};
}

bool MainWindow::Init()
{
    wc_.cbSize = sizeof(wc_);
    wc_.style = 0;
    wc_.lpfnWndProc = &MainWindow::static_wndProc;
    wc_.cbClsExtra = 0;
    wc_.cbWndExtra = 0;
    wc_.hInstance = inst_;
    wc_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc_.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc_.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
    wc_.lpszClassName = className_;
    wc_.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

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

HWND MainWindow::GetHwnd()
{
    while (!created_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return hwnd_;
}

void MainWindow::threadProc()
{
    hwnd_ = CreateWindowEx(WS_EX_LEFT, className_, L"My new window", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, inst_, this);
    if (!hwnd_) {
        Utils::LogError(L"Could not create window");
        return;
    }
    SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    ShowWindow(hwnd_, cmdShow_);
    UpdateWindow(hwnd_);
    created_ = true;
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessage(controllerDialog_, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

INT_PTR CALLBACK DlgFunc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM)
{
    switch (msg) {
        case WM_INITDIALOG: {
            auto initScroolbar = [&hwnd](int id, int val) {
                auto sb = GetDlgItem(hwnd, id);
                SendMessage(sb, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
                SendMessage(sb, PBM_SETPOS, val, 0);
            };
            initScroolbar(IDC_RT, 0);
            initScroolbar(IDC_LT, 0);
            initScroolbar(IDC_RX, 50);
            initScroolbar(IDC_RY, 50);
            initScroolbar(IDC_LX, 50);
            initScroolbar(IDC_RX, 50);
            return TRUE;
        }
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case 123: {
                    return TRUE;
                }
            }
        }
        default:
            break;
    }
    return FALSE;
}

LRESULT MainWindow::wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
    auto editMessage = [&](const std::wstring& str) {
        auto edit = GetDlgItem(controllerDialog_, IDC_EDIT1);
        SetWindowText(edit, str.c_str());
    };
    auto setScrollbarLevel = [&](int id, float value) {
        auto lx = GetDlgItem(controllerDialog_, id);
        SendMessage(lx, PBM_SETPOS, static_cast<WPARAM>(value), 0);
    };
    switch (msg) {
        case WM_CREATE: {
            controllerDialog_ = CreateDialog(
                GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_FORMVIEW), hwnd, DlgFunc);
            ShowWindow(controllerDialog_, SW_SHOW);
            auto edit = GetDlgItem(controllerDialog_, IDC_EDIT1);
            SetWindowText(edit, L"Here be status");

            profile_->Load("profile.json");
            auto hubs = profile_->GetHubs();
            auto menu = GetMenu(hwnd);
            auto deviceMenu = GetSubMenu(menu, 1);
            int menuId = Utils::enum_value(GUI::Event::LAST_EVENT);
            for (const auto& hub : hubs) {
                auto subMenu = CreatePopupMenu();
                bool res = AppendMenu(subMenu, MF_STRING, menuId, L"&Connect");
                connectEvents_[menuId++] = hub;
                res = AppendMenu(subMenu, MF_STRING, menuId, L"&Disconnect");
                disconnectEvents_[menuId++] = hub;
                res = AppendMenu(deviceMenu, MF_STRING | MF_POPUP,
                    reinterpret_cast<UINT_PTR>(subMenu), hub.c_str());
            }

            //auto w = CreateWindowEx(WS_EX_CLIENTEDGE, MAKEINTRESOURCE(IDD_FORMVIEW), L"",
            //    WS_CHILD | WS_VISIBLE, 0, 0, 100, 100, hwnd, (HMENU)123, GetModuleHandle(NULL),
            //    NULL);
            break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_FILE_EXIT:
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case ID_DEVICE_TEST1:
                    //hub_->Test1();
                    break;
                case ID_DEVICE_TEST2:
                    //hub_->Test2();
                    break;
                case ID_HELP_ABOUT:
                    DialogBox(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_DIALOGBAR), hwnd,
                        AboutDlgProc);
                    break;
                default: {
                    if (connectEvents_.count(wParam) > 0) {
                        profile_->Connect(connectEvents_.at(wParam));
                    } else if (disconnectEvents_.count(wParam) > 0) {
                        profile_->Disconnect(disconnectEvents_.at(wParam));
                    }
                } break;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case Utils::enum_value(GUI::Event::ButtonDown): {
            if (buttonEvents_.count(wParam) > 0) {
                auto btn = GetDlgItem(controllerDialog_, buttonEvents_.at(wParam));
                SendMessage(btn, BM_SETSTATE, TRUE, 0);
            }
        } break;
        case Utils::enum_value(GUI::Event::ButtonUp): {
            if (buttonEvents_.count(wParam) > 0) {
                auto btn = GetDlgItem(controllerDialog_, buttonEvents_.at(wParam));
                SendMessage(btn, BM_SETSTATE, FALSE, 0);
            }
        } break;
        case Utils::enum_value(GUI::Event::LX): {
            setScrollbarLevel(IDC_LX, Utils::FromWparam(wParam) * 50 + 50);
            valueSet(2, static_cast<int>(-Utils::FromWparam(wParam) * 90));
        } break;
        case Utils::enum_value(GUI::Event::RX): {
            setScrollbarLevel(IDC_RX, Utils::FromWparam(wParam) * 50 + 50);
        } break;
        case Utils::enum_value(GUI::Event::LY): {
            setScrollbarLevel(IDC_LY, Utils::FromWparam(wParam) * 50 + 50);
        } break;
        case Utils::enum_value(GUI::Event::RY): {
            setScrollbarLevel(IDC_RY, Utils::FromWparam(wParam) * 50 + 50);
        } break;
        case Utils::enum_value(GUI::Event::LT): {
            setScrollbarLevel(IDC_LT, Utils::FromWparam(wParam) * 100);
            valueSet(0, static_cast<int>(-100 * Utils::FromWparam(wParam)));
            //valueSet(1, 100 * Utils::FromWparam(wParam));
        } break;
        case Utils::enum_value(GUI::Event::RT): {
            setScrollbarLevel(IDC_RT, Utils::FromWparam(wParam) * 100);
            valueSet(0, static_cast<int>(100 * Utils::FromWparam(wParam)));
            //valueSet(1, -100 * Utils::FromWparam(wParam));
        } break;
        case WM_KEYDOWN:
            if (wParam <= UINT8_MAX) {
                profile_->ButtonDown(static_cast<uint8_t>(wParam));
            }
            break;
        case WM_KEYUP:
            if (wParam <= UINT8_MAX) {
                profile_->ButtonUp(static_cast<uint8_t>(wParam));
            }
            break;
        default:
            break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MainWindow::valueChange(int v, int delta)
{
    auto pv = values_[v];
    pv += delta;
    if (pv > 100) {
        pv = 100;
    }
    if (pv < -100) {
        pv = -100;
    }
    if (pv != values_.at(v)) {
        values_[v] = pv;
        //hub_->Motor(v, pv);
    }
}

void MainWindow::valueSet(int v, int val)
{
    auto pv = values_[v];
    if (v < 2) {
        if (val > 100) {
            val = 100;
        }
        if (val < -100) {
            val = -100;
        }
        if (val == pv) {
            return;
        }
        values_[v] = val;
        //hub_->Motor(v, val);
    } else {
        if (abs(val - pv) < 10) {
            return;
        }
        values_[v] = val;
        //hub_->Servo(v, val, 60, 60);
    }
}

} // namespace Ancorage::GUI
