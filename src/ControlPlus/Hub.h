#pragma once

#include "BLE/BLE.h"
#include "BLE/Sink.h"
#include "Port.h"

#include "rapidjson_wrap.h"

#include <memory>
#include <string>
#include <map>

namespace Ancorage::ControlPlus
{
class Hub : public BLE::Sink
{
public:
    Hub();

    bool Parse(const rapidjson::WValue::ConstObject& v);

    bool Connect();
    void Disconnect();

    void Test1();
    void Test2();

    std::wstring GetName() const;
    void Consume(const std::unique_ptr<BLE::Message>& m) override;
    void ButtonDown(uint8_t b);
    void ButtonUp(uint8_t b);

private:
    std::shared_ptr<BLE::BLEManager> ble_ = std::make_unique<BLE::BLEManager>();
    std::map<int, std::unique_ptr<Port>> ports_;
    std::wstring id_;
    std::wstring name_;
};
} // namespace Ancorage::ControlPlus
