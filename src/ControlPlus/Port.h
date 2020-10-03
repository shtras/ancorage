#pragma once

#include "BLE/BLE.h"

#include "rapidjson_wrap.h"

#include <cstdint>
#include <map>

namespace Ancorage::ControlPlus
{
class Port
{
public:
    enum class Type { Motor, Servo, Stepper, Unknown };
    explicit Port(BLE::BLEManager* ble);
    bool Parse(const rapidjson::WValue::ConstObject& v);
    uint8_t GetId() const;
    void ButtonDown(uint8_t b);
    void ButtonUp(uint8_t b);

private:
    struct Action
    {
        enum class Type { Value, Forward, Backward, Absolute, Unknown };

        int value = 0;
        int zeroValue = 0;
        Type type = Type::Unknown;
    };

    void executeAction(const Action& a, bool start);

    uint8_t id_ = UINT8_MAX;
    Type type_ = Type::Unknown;
    int position_ = 0;
    BLE::BLEManager* ble_ = nullptr;
    std::map<uint8_t, Action> continuousActions_;
    std::map<uint8_t, Action> singleActions_;
};
} // namespace Ancorage::ControlPlus
