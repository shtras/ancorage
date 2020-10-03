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
    void Motor(uint8_t idx, uint8_t power);
    void Servo(uint8_t idx, int32_t pos, int8_t speed, int8_t power);

    std::wstring GetName() const;
    void Consume(const std::unique_ptr<BLE::Message>& m) override;

private:
    std::unique_ptr<BLE::BLEManager> ble_ = std::make_unique<BLE::BLEManager>();
    std::map<int, std::unique_ptr<Port>> ports_;
    std::wstring id_;
    std::wstring name_;
};
} // namespace Ancorage::ControlPlus
