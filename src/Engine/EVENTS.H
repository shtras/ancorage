#pragma once

#include <Windows.h>

namespace Ancorage::Events
{
enum class Event { DummyEvent = WM_USER, ButtonUp, ButtonDown, RX, RY, LX, LY, RT, LT };
class EventSink
{
public:
    virtual ~EventSink()
    {
    }
};
} // namespace Ancorage::Events
