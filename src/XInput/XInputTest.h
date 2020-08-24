#pragma once

#include <atomic>
#include <thread>

namespace Ancorage::XInput
{
class ControllerManager
{
public:
    void Run();
    void Stop();
    void InitControllers();

private:
    struct Controller
    {
    };
    void threadProc();

    std::atomic<bool> running_{false};
    std::thread t_;
};

} // namespace Ancorage::XInput
