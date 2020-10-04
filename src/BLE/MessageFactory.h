#pragma once

#include "Message.h"

#include <memory>

namespace Ancorage::BLE
{
class MessageFactory
{
public:
    static std::shared_ptr<Message> CreateHubActionsMessage(HubActionsMessage::ActionType t);

    static std::shared_ptr<Message> CreatePortInfoRequestMessage(
        uint8_t portId, PortInfoRequestMessage::InfoType type);

    static std::shared_ptr<Message> CreatePortModeInfoRequestMessage(
        uint8_t portId, uint8_t mode, PortModeInfoRequestMessage::InfoType type);

    static std::shared_ptr<Message> CreatePortInputFormatSetupSingleMessage(
        uint8_t portId, uint8_t mode, uint32_t deltaInterval, bool notify);

    static std::shared_ptr<Message> CreateStartSpeedPortOutputCommandMessage(
        uint8_t portId, int8_t speed, int8_t maxPower, int8_t useProfile);

    static std::shared_ptr<Message> CreateGotoAbsolutePositionPortOutputCommandMessage(
        uint8_t portId, int position, int8_t speed, int8_t power, int8_t endState);
};
} // namespace Ancorage::BLE
