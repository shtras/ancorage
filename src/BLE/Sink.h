#pragma once
#include "Message.h"

#include <memory>

namespace Ancorage::BLE
{
class Sink
{
public:
    virtual ~Sink() = default;
    virtual void Consume(const std::unique_ptr<BLE::Message>& m) = 0;
};
} // namespace Ancorage::BLE
