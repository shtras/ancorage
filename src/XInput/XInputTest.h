#pragma once

#include <Windows.h>

#include <atomic>
#include <thread>
#include <vector>

namespace Ancorage::XInput
{
class ControllerManager
{
public:
    explicit ControllerManager(HWND hwnd);
    void Run();
    void Stop();

private:
    struct Controller
    {
        uint32_t Packet = 0;
        std::vector<bool> Buttons = std::vector<bool>(16, false);
        float RX = 0;
        float RY = 0;
        float LX = 0;
        float LY = 0;
        float RT = 0;
        float LT = 0;
    };
    void initControllers();
    bool queryControllers();
    void threadProc();
    float stickValue(int16_t v) const;
    float triggerValue(uint8_t v) const;

    std::atomic<bool> running_{false};
    std::vector<Controller> controllers_;
    std::thread t_;
    HWND hwnd_ = nullptr;
};

} // namespace Ancorage::XInput
