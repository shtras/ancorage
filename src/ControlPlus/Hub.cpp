#include "Hub.h"

#include "BLE/Message.h"
#include "Utils/Utils.h"

namespace Ancorage::ControlPlus
{
Hub::Hub()
{
    ble_->SetSink(this);
}

bool Hub::Parse(const rapidjson::WValue::ConstObject& v)
{
    auto idO = Utils::GetT<std::wstring>(v, L"id");
    if (!idO) {
        spdlog::error("Missing id");
        return false;
    }
    id_ = *idO;
    auto nameO = Utils::GetT<std::wstring>(v, L"name");
    if (!nameO) {
        spdlog::error("Missing name");
        return false;
    }
    name_ = *nameO;

    auto portsO = Utils::GetT<rapidjson::WValue::ConstArray>(v, L"ports");
    if (!portsO) {
        spdlog::error("Missing ports");
        return false;
    }
    const auto& portsArr = *portsO;
    for (rapidjson::SizeType i = 0; i < portsArr.Size(); ++i) {
        auto portO = Utils::GetT<rapidjson::WValue::ConstObject>(portsArr[i]);
        if (!portO) {
            return false;
        }
        auto port = std::make_unique<Port>();
        if (!port->Parse(*portO)) {
            return false;
        }
        ports_[port->GetId()] = std::move(port);
    }
    return true;
}

bool Hub::Connect()
{
    if (!ble_->Connect(id_)) {
        return false;
    }
    ble_->Run();
    return true;
}

void Hub::Disconnect()
{
    ble_ = std::make_unique<BLE::BLEManager>();
}

void Hub::Test1()
{
    auto m = std::make_shared<BLE::PortInputFormatSetupSingle>();
    m->portId_ = 1;
    m->mode_ = 0;
    m->deltaInterval_ = 5;
    m->notificationEnabled_ = true;
    ble_->SendBTMessage(m);
}

void Hub::Test2()
{
    auto m = std::make_shared<BLE::GotoAbsolutePositionPortOutputCommandMessage>();
    m->portId_ = 3;
    m->startupCompletion_ = 0x11;
    m->pos_ = 90;
    m->speed_ = 60;
    m->power_ = 60;
    m->endState_ = 127;
    ble_->SendBTMessage(m);
}

void Hub::Motor(uint8_t idx, uint8_t power)
{
    auto m = std::make_shared<BLE::WriteDirectModeDataPortOutputCommandMessage>();
    m->portId_ = idx;
    m->startupCompletion_ = 0x11;
    m->mode_ = 0;
    m->payload_.push_back(power);
    ble_->SendBTMessage(m);
}

void Hub::Servo(uint8_t idx, int32_t pos, int8_t speed, int8_t power)
{
    auto m = std::make_shared<BLE::GotoAbsolutePositionPortOutputCommandMessage>();
    m->portId_ = idx;
    m->startupCompletion_ = 0x11;
    m->pos_ = pos;
    m->speed_ = speed;
    m->power_ = power;
    m->endState_ = 127;
    ble_->SendBTMessage(m);
}

std::wstring Hub::GetName() const
{
    return name_;
}

void Hub::Consume(const std::unique_ptr<BLE::Message>& m)
{
    (void)m;
}
} // namespace Ancorage::ControlPlus
