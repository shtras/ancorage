#include "XInputTest.h"

#include "spdlog_wrap.h"

#include <Windows.h>
#include <xinput.h>
#include <thread>

namespace Ancorage::XInput
{
void ControllerManager::Run()
{
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

bool differs(XINPUT_GAMEPAD& g1, XINPUT_GAMEPAD& g2)
{
    return memcmp(&g1, &g2, sizeof(XINPUT_GAMEPAD)) != 0;
}

void ControllerManager::threadProc()
{
    XINPUT_STATE states[XUSER_MAX_COUNT];
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        ZeroMemory(&states[i], sizeof(XINPUT_STATE));
    }
    while (running_) {
        DWORD dwResult;
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
            XINPUT_STATE state;
            ZeroMemory(&state, sizeof(XINPUT_STATE));
            dwResult = XInputGetState(i, &state);

            if (dwResult == ERROR_SUCCESS) {
                if (differs(states[i].Gamepad, state.Gamepad)) {
                    states[i] = state;
                    spdlog::debug("pkt: {} but: {} lt: {} rt: {} lx: {} ly: {} rx: {} ry: {}",
                        state.dwPacketNumber, state.Gamepad.wButtons, state.Gamepad.bLeftTrigger,
                        state.Gamepad.bRightTrigger, state.Gamepad.sThumbLX, state.Gamepad.sThumbLY,
                        state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
} // namespace Ancorage::XInput
