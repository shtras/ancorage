#pragma once

#include "BLE/BLE.h"
#include "BLE/Message.h"
#include "Utils/Utils.h"

#include "rapidjson_wrap.h"

#include <cstdint>
#include <map>
#include <atomic>

namespace Ancorage::ControlPlus
{
class Port
{
public:
    enum class Type { Motor, Servo, Stepper, Unknown };
    explicit Port(BLE::BLEManager* ble);
    ~Port();

    bool Parse(const rapidjson::WValue::ConstObject& v);
    uint8_t GetId() const;
    void ButtonDown(uint8_t b);
    void ButtonUp(uint8_t b);
    void OnMessage(const std::unique_ptr<BLE::Message>& m);
    void OnConnect();

private:
    struct Action
    {
        enum class Type { Value, Forward, Backward, Absolute, Unknown };

        int value = 0;
        int zeroValue = 0;
        Type type = Type::Unknown;
    };

    void executeAction(const Action& a, bool start);
    void connectProc();
    void onInitializingMessage(const std::unique_ptr<BLE::Message>& m);

    uint8_t id_ = UINT8_MAX;
    Type type_ = Type::Unknown;
    int initialPos_ = 0;
    int position_ = 0;
    BLE::BLEManager* ble_ = nullptr;
    std::map<uint8_t, Action> continuousActions_;
    std::map<uint8_t, Action> singleActions_;
    Utils::Semaphore initSem_;
    std::thread connectT_;
    uint8_t numModes_ = 0;
    uint8_t absPosMode_ = UINT8_MAX;
    BLE::Message::Type nextInitMessage_ = BLE::Message::Type::Unknown;
    std::atomic<bool> initialized_{false};
};
} // namespace Ancorage::ControlPlus
