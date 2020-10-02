#include "HubHandler.h"

#include "BLE/Event.h"

namespace Ancorage::ControlPlus
{
HubHandler::HubHandler(std::wstring id, std::wstring name)
    : id_(std::move(id))
    , name_(std::move(name))
{
}

bool HubHandler::Connect()
{
    if (!ble_->Connect(id_)) {
        return false;
    }
    ble_->Run();
    return true;
}

void HubHandler::Disconnect()
{
    ble_ = std::make_unique<BLE::BLEManager>();
}

void HubHandler::Test1()
{
    auto m = std::make_shared<BLE::PortInputFormatSetupSingle>();
    m->portId_ = 1;
    m->mode_ = 0;
    m->deltaInterval_ = 5;
    m->notificationEnabled_ = true;
    ble_->SendBTMessage(m);
}

void HubHandler::Test2()
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

void HubHandler::Motor(uint8_t idx, uint8_t power)
{
    auto m = std::make_shared<BLE::WriteDirectModeDataPortOutputCommandMessage>();
    m->portId_ = idx;
    m->startupCompletion_ = 0x11;
    m->mode_ = 0;
    m->payload_.push_back(power);
    ble_->SendBTMessage(m);
}

void HubHandler::Servo(uint8_t idx, int32_t pos, int8_t speed, int8_t power)
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

std::wstring HubHandler::GetName() const
{
    return name_;
}
} // namespace Ancorage::ControlPlus
