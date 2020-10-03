#pragma once

#include <cstdint>

namespace Ancorage::ControlPlus
{
class Port
{
public:
    enum class Type { Motor, Servo, Stepper, Unknown };

private:
    uint8_t idx_ = UINT8_MAX;
    Type type_ = Type::Unknown;
};
} // namespace Ancorage::ControlPlus
