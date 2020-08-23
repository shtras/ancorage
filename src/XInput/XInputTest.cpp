#include "XInputTest.h"

#include "spdlog_wrap.h"

#include <Windows.h>
#include <xinput.h>

namespace Ancorage::XInput
{
void Test1()
{
    DWORD dwResult;
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        // Simply get the state of the controller from XInput.
        dwResult = XInputGetState(i, &state);

        if (dwResult == ERROR_SUCCESS) {
            spdlog::info("Device {} is connected", i);
        } else {
            spdlog::info("Device {} is not connected: {}", i, dwResult);
        }
    }
}
} // namespace Ancorage::XInput
