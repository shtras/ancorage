#pragma once

#include "rapidjson_wrap.h"

#include <cstdint>

namespace Ancorage::ControlPlus
{
class Port
{
public:
    enum class Type { Motor, Servo, Stepper, Unknown };

    bool Parse(const rapidjson::WValue::ConstObject& v);
    uint8_t GetId() const;

private:
    struct Action
    {
        enum class Type { Value, Forward, Backward, Absolute, Unknown };

        int value = 0;
        Type type = Type::Unknown;
    };

    uint8_t id_ = UINT8_MAX;
    Type type_ = Type::Unknown;
    int position_ = 0;
};
} // namespace Ancorage::ControlPlus
