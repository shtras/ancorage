#include "MessageFactory.h"
namespace Ancorage::BLE
{
std::shared_ptr<Message> MessageFactory::CreateHubActionsMessage(HubActionsMessage::ActionType t)
{
    auto m = std::make_shared<HubActionsMessage>();
    m->actionType_ = t;
    return m;
}

std::shared_ptr<Message> MessageFactory::CreatePortInfoRequestMessage(
    uint8_t portId, PortInfoRequestMessage::InfoType type)
{
    auto m = std::make_shared<PortInfoRequestMessage>();
    m->portId_ = portId;
    m->infoType_ = type;
    return m;
}

std::shared_ptr<Message> MessageFactory::CreateStartSpeedPortOutputCommandMessage(
    uint8_t portId, int8_t speed, int8_t maxPower, int8_t useProfile)
{
    auto m = std::make_shared<StartSpeedPortOutputCommandMessage>();
    m->portId_ = portId;
    m->speed_ = speed;
    m->maxPower_ = maxPower;
    m->useProfile_ = useProfile;
    return m;
}

std::shared_ptr<Message> MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
    uint8_t portId, int position, int8_t speed, int8_t power, int8_t endState)
{
    auto m = std::make_shared<GotoAbsolutePositionPortOutputCommandMessage>();
    m->portId_ = portId;
    m->pos_ = position;
    m->speed_ = speed;
    m->power_ = power;
    m->startupCompletion_ = 0x11;
    m->endState_ = endState;
    return m;
}
} // namespace Ancorage::BLE