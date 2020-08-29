#pragma once

#include <memory>

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

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Ancorage::BLE
