#pragma once

#include <memory>

#include "Message.h"
#include "Sink.h"

namespace Ancorage::BLE
{
class BLEManager
{
public:
    BLEManager();
    ~BLEManager();
    bool Connect(const std::wstring& id);
    void Run();
    void Stop();

    void SendBTMessage(const std::shared_ptr<Message>& m);
    void SetSink(Sink* sink);

private:
    class Impl;
    std::unique_ptr<Impl> impl_ = nullptr;
};

} // namespace Ancorage::BLE
