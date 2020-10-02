#include "XInputTest.h"

#include "GUI/IDs.h"
#include "Utils/Utils.h"

#include "spdlog_wrap.h"

#include <Windows.h>
#include <xinput.h>
#include <thread>
#include <climits>

namespace Ancorage::XInput
{
ControllerManager::ControllerManager(HWND hwnd)
    : hwnd_(hwnd)
{
}

void ControllerManager::Run()
{
    initControllers();
    running_ = true;
    t_ = std::thread(&ControllerManager::threadProc, this);
}

void ControllerManager::Stop()
{
    running_ = false;
    if (t_.joinable()) {
        t_.join();
    }
}

void ControllerManager::initControllers()
{
    controllers_.clear();
    controllers_.resize(XUSER_MAX_COUNT);
}

bool ControllerManager::queryControllers()
{
    if (controllers_.size() != XUSER_MAX_COUNT) {
        spdlog::error("Controllers not initialized");
        return false;
    }
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        auto res = XInputGetState(i, &state);
        if (res != ERROR_SUCCESS) {
            continue;
        }
        auto& ctr = controllers_[i];
        if (ctr.Packet == state.dwPacketNumber) {
            continue;
        }
        for (int b = 0; b < 16; ++b) {
            int btnMask = 1 << b;
            bool pressed = state.Gamepad.wButtons & btnMask;
            if (pressed != ctr.Buttons[b]) {
                spdlog::info("Button {} is now {}", b, (pressed ? "pressed" : "released"));
                ctr.Buttons[b] = pressed;
                auto event = pressed ? GUI::Event::ButtonDown : GUI::Event::ButtonUp;
                SendMessage(hwnd_, Utils::enum_value(event), b, 0);
            }
        }
        const float e = 1e-3f;
        auto lx = stickValue(state.Gamepad.sThumbLX);
        auto rx = stickValue(state.Gamepad.sThumbRX);
        auto ly = stickValue(state.Gamepad.sThumbLY);
        auto ry = stickValue(state.Gamepad.sThumbRY);
        auto lt = triggerValue(state.Gamepad.bLeftTrigger);
        auto rt = triggerValue(state.Gamepad.bRightTrigger);
        auto compareAssign = [&](float& v1, float v2, GUI::Event event) {
            if (v2 < -1.0f) {
                v2 = -1.0f;
            }
            if (fabs(v1 - v2) < e) {
                return;
            }
            v1 = v2;
            if (event != GUI::Event::DummyEvent) {
                SendMessage(hwnd_, Utils::enum_value(event), Utils::FromFloat(v1), 0);
            }
        };
        compareAssign(ctr.LX, lx, GUI::Event::LX);
        compareAssign(ctr.RX, rx, GUI::Event::RX);
        compareAssign(ctr.LY, ly, GUI::Event::LY);
        compareAssign(ctr.RY, ry, GUI::Event::RY);
        compareAssign(ctr.LT, lt, GUI::Event::LT);
        compareAssign(ctr.RT, rt, GUI::Event::RT);
    }
    return true;
}

float ControllerManager::stickValue(int16_t v) const
{
    return static_cast<float>(v) / static_cast<float>(SHRT_MAX);
}

float ControllerManager::triggerValue(uint8_t v) const
{
    return static_cast<float>(v) / static_cast<float>(UCHAR_MAX);
}

void ControllerManager::threadProc()
{
    XINPUT_STATE states[XUSER_MAX_COUNT];
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        ZeroMemory(&states[i], sizeof(XINPUT_STATE));
    }
    while (running_) {
        if (!queryControllers()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
} // namespace Ancorage::XInput
