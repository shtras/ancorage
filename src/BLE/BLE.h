#pragma once

#include <memory>

#include "Event.h"

namespace Ancorage::BLE
{
class BLEManager
{
public:
    BLEManager();
    ~BLEManager();
    bool Connect();
    void Run();
    void Stop();

    void SendBTMessage(const std::shared_ptr<Message>& m);

private:
    class Impl;
    std::unique_ptr<Impl> impl_ = nullptr;
};

} // namespace Ancorage::BLE
