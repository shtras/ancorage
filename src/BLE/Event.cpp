#include "Event.h"
#include "Utils/Utils.h"
#include "EventNames.h"

#include "spdlog_wrap.h"

namespace Ancorage::BLE
{
uint16_t Get16(const std::vector<uint8_t>& b, size_t& itr)
{
    uint16_t res = (b[itr + 1] << 8) | b[itr];
    itr += 2;
    return res;
}

uint32_t Get32(const std::vector<uint8_t>& b, size_t& itr)
{
    auto low = Get16(b, itr);
    auto high = Get16(b, itr);

    return (low << 16) | high;
}

void Set32(std::vector<uint8_t>& b, uint32_t v)
{
    b.push_back(v & 0xff);
    b.push_back(static_cast<uint8_t>((v & 0xff00) >> 8));
    b.push_back(static_cast<uint8_t>((v & 0xff0000) >> 16));
    b.push_back(static_cast<uint8_t>((v & 0xff000000) >> 24));
}

bool Message::IsType(uint8_t b)
{
    switch (Utils::to_enum<Message::Type>(b)) {
        case Type::HubProperties:
        case Type::HubActions:
        case Type::HubAlerts:
        case Type::HubAttachedIO:
        case Type::GenericError:
        case Type::HWNetworkCommands:
        case Type::FWUpdateGoIntoBootMode:
        case Type::FWUpdateLockMemory:
        case Type::FWUpdateLockStatusRequest:
        case Type::FWLockStatus:
        case Type::PortInfoRequest:
        case Type::PortModeInfoRequest:
        case Type::PortInputFormatSetupSingle:
        case Type::PortInputFormatSetupCombined:
        case Type::PortInfo:
        case Type::PortModeInfo:
        case Type::PortValueSingle:
        case Type::PortValueCombined:
        case Type::PortInputFormatSingle:
        case Type::PortInputFormatCombined:
        case Type::VirtualPortSetup:
        case Type::PortOutputCommand:
        case Type::PortOutputCommandFeedback:
            return true;
    }
    return false;
}

std::unique_ptr<Message> Message::Parse(uint8_t* buffer, size_t size)
{
    if (size < 3) {
        spdlog::error("Message too short: {}", size);
        return nullptr;
    }
    auto b = buffer[2];
    if (!IsType(b)) {
        spdlog::error("Unknown message type: {}", static_cast<int>(b));
        return nullptr;
    }
    std::unique_ptr<Message> res = nullptr;
    switch (Utils::to_enum<Message::Type>(b)) {
        case Type::HubProperties:
            break;
        case Type::HubActions:
            res = std::make_unique<HubActionsMessage>();
            break;
        case Type::HubAlerts:
            break;
        case Type::HubAttachedIO:
            res = std::make_unique<HubAttachedIOMessage>();
            break;
        case Type::GenericError:
            res = std::make_unique<GenericErrorMessage>();
            break;
        case Type::HWNetworkCommands:
            break;
        case Type::FWUpdateGoIntoBootMode:
            break;
        case Type::FWUpdateLockMemory:
            break;
        case Type::FWUpdateLockStatusRequest:
            break;
        case Type::FWLockStatus:
            break;
        case Type::PortInfoRequest:
            res = std::make_unique<PortInfoRequestMessage>();
            break;
        case Type::PortModeInfoRequest:
            break;
        case Type::PortInputFormatSetupSingle:
            res = std::make_unique<PortInputFormatSetupSingle>();
            break;
        case Type::PortInputFormatSetupCombined:
            break;
        case Type::PortInfo:
            break;
        case Type::PortModeInfo:
            break;
        case Type::PortValueSingle:
            res = std::make_unique<PortValueSingleMessage>();
            break;
        case Type::PortValueCombined:
            break;
        case Type::PortInputFormatSingle:
            break;
        case Type::PortInputFormatCombined:
            break;
        case Type::VirtualPortSetup:
            break;
        case Type::PortOutputCommand: {
            if (size < 6) {
                return nullptr;
            }
            auto scb = buffer[5];
            switch (Utils::to_enum<PortOutputCommandMessage::SubCommand>(scb)) {
                case PortOutputCommandMessage::SubCommand::StartPower:
                    break;
                case PortOutputCommandMessage::SubCommand::SetAccTime:
                    break;
                case PortOutputCommandMessage::SubCommand::SetDecTime:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeed:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeed2:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeedForTime:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeedForTime2:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees2:
                    break;
                case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition:
                    break;
                case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition2:
                    break;
                case PortOutputCommandMessage::SubCommand::PresetEncoder:
                    break;
                case PortOutputCommandMessage::SubCommand::WriteDirect:
                    break;
                case PortOutputCommandMessage::SubCommand::WriteDirectModeData:
                    res = std::make_unique<WriteDirectModeDataPortOutputCommandMessage>();
                    break;
            }
        } break;
        case Type::PortOutputCommandFeedback:
            res = std::make_unique<PortOutputCommandFeedbackMessage>();
            break;
    }
    if (!res) {
        spdlog::error("Unsupported message type: {} {}", static_cast<int>(b),
            EnumName(Utils::to_enum<Message::Type>(b)));
        return nullptr;
    }
    if (!res->parse(buffer, size)) {
        spdlog::error("Message parsing failed");
        //return nullptr;
    }
    return res;
} // namespace Ancorage::BLE

std::string Message::ToString() const
{
    std::stringstream ss;
    ss << std::endl << "\tType: " << EnumName(type_);
    toString(ss);
    return ss.str();
}

std::vector<uint8_t> Message::ToBuffer() const
{
    std::vector<uint8_t> res;
    res.push_back(0); // size
    res.push_back(0); // reserved hub id
    res.push_back(static_cast<uint8_t>(Utils::enum_value(type_)));
    toBuffer(res);
    assert(res.size() <= 255);
    // @TODO: Handle longer messages
    res[0] = static_cast<uint8_t>(res.size());
    return res;
}

bool Message::parse(uint8_t* buffer, size_t size)
{
    buffer_.insert(buffer_.end(), buffer, buffer + size);
    size_t itr = 0;
    if (!parseHeader(itr)) {
        return false;
    }
    if (!parseBody(itr)) {
        return false;
    }
    if (itr != size) {
        return false;
    }
    return true;
}

bool Message::parseHeader(size_t& itr)
{
    auto b1 = buffer_[itr++];
    size_ = b1 & 0x7f;
    if (b1 & 0x80) {
        size_ <<= 8;
        size_ |= buffer_[itr++];
    }
    if (buffer_[itr++] != 0) {
        // Hub ID
        return false;
    }
    auto type = Utils::to_enum<Message::Type>(buffer_[itr++]); // type;
    if (type != type_) {
        return false;
    }
    return true;
}

HubActionsMessage::HubActionsMessage()
{
    type_ = Type::HubActions;
}

bool HubActionsMessage::parseBody(size_t& itr)
{
    auto b0 = buffer_[itr++];
    actionType_ = Utils::to_enum<ActionType>(b0);
    return true;
}

void HubActionsMessage::toString(std::stringstream& ss) const
{
    ss << std::endl << "\tAction type: " << EnumName(actionType_);
}

void HubActionsMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    buf.push_back(static_cast<uint8_t>(Utils::enum_value(actionType_)));
}

GenericErrorMessage::GenericErrorMessage()
{
    type_ = Type::GenericError;
}

bool GenericErrorMessage::parseBody(size_t& itr)
{
    auto b0 = buffer_[itr++];
    auto b1 = buffer_[itr++];
    commandType_ = Utils::to_enum<Type>(b0);
    code_ = Utils::to_enum<Code>(b1);
    return true;
}

void GenericErrorMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tCommand: " << EnumName(commandType_) << std::endl
       << "\tError: " << EnumName(code_);
}

HubAttachedIOMessage::HubAttachedIOMessage()
{
    type_ = Message::Type::HubAttachedIO;
}

bool HubAttachedIOMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    event_ = Utils::to_enum<Event>(buffer_[itr++]);
    if (event_ == Event::AttachedIO || event_ == Event::AttachedVirtualIO) {
        uint16_t typeId = Get16(buffer_, itr);
        ioTypeId_ = Utils::to_enum<IOTypeID>(typeId);
    }
    if (event_ == Event::AttachedIO || event_ == Event::DetachedIO) {
        hardwareRev_ = Get32(buffer_, itr);
        softwareRev_ = Get32(buffer_, itr);
    }
    if (event_ == Event::AttachedVirtualIO) {
        portA_ = buffer_[itr++];
        portB_ = buffer_[itr++];
    }
    return true;
}

void HubAttachedIOMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort: " << static_cast<int>(portId_) << std::endl
       << "\tEvent: " << EnumName(event_);
    if (event_ == Event::AttachedIO || event_ == Event::AttachedVirtualIO) {
        ss << std::endl << "\tIO type: " << EnumName(ioTypeId_);
    }
    if (event_ == Event::AttachedIO) {
        ss << std::endl
           << "\tHardware revision: " << hardwareRev_ << std::endl
           << "\tSoftware revision: " << softwareRev_;
    }
    if (event_ == Event::AttachedVirtualIO) {
        ss << std::endl << "\tPort ID A: " << portA_ << std::endl << "\tPort ID B: " << portB_;
    }
}

PortInputFormatSetupSingle::PortInputFormatSetupSingle()
{
    type_ = Type::PortInputFormatSetupSingle;
}

bool PortInputFormatSetupSingle::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    mode_ = buffer_[itr++];
    deltaInterval_ = Get32(buffer_, itr);
    notificationEnabled_ = buffer_[itr++];
    return true;
}

void PortInputFormatSetupSingle::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId_) << std::endl
       << "\tMode" << static_cast<int>(mode_) << std::endl
       << "\tDelta Interval: " << deltaInterval_ << std::endl
       << "\tNotification Enabled: " << notificationEnabled_;
}

void PortInputFormatSetupSingle::toBuffer(std::vector<uint8_t>& buf) const
{
    buf.push_back(portId_);
    buf.push_back(mode_);
    Set32(buf, deltaInterval_);
    buf.push_back(notificationEnabled_);
}

PortInfoRequestMessage::PortInfoRequestMessage()
{
    type_ = Type::PortInfoRequest;
}

bool PortInfoRequestMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    infoType_ = Utils::to_enum<InfoType>(buffer_[itr++]);
    return true;
}

void PortInfoRequestMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort: " << static_cast<int>(portId_) << std::endl
       << "\tInfo Type: " << EnumName(infoType_);
}

void PortInfoRequestMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    buf.push_back(portId_);
    buf.push_back(Utils::enum_value(infoType_));
}

PortValueSingleMessage::PortValueSingleMessage()
{
    type_ = Type::PortValueSingle;
}

bool PortValueSingleMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    if (size_ == 5) {
        value_ = buffer_[itr++];
    } else if (size_ == 6) {
        value_ = Get16(buffer_, itr);
    } else if (size_ == 8) {
        value_ = Get32(buffer_, itr);
    } else {
        return false;
    }
    return true;
}

void PortValueSingleMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId_) << std::endl
       << "\tValue: " << value_;
}

PortOutputCommandMessage::PortOutputCommandMessage()
{
    type_ = Type::PortOutputCommand;
}

bool PortOutputCommandMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    startupCompletion_ = buffer_[itr++];
    if (!parseSubCommand(itr)) {
        return false;
    }
    return true;
}

void PortOutputCommandMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort: " << static_cast<int>(portId_) << std::endl
       << "\tStartup Completion: " << static_cast<int>(startupCompletion_) << std::endl
       << "\tSub command: " << EnumName(subCommand_);
}

void PortOutputCommandMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    buf.push_back(portId_);
    buf.push_back(startupCompletion_);
    buf.push_back(static_cast<uint8_t>(Utils::enum_value(subCommand_)));
}

GotoAbsolutePositionPortOutputCommandMessage::GotoAbsolutePositionPortOutputCommandMessage()
{
    subCommand_ = SubCommand::GotoAbsolutePosition;
}

void GotoAbsolutePositionPortOutputCommandMessage::toString(std::stringstream& ss) const
{
    PortOutputCommandMessage::toString(ss);
    ss << std::endl
       << "\tAbsPos: " << pos_ << std::endl
       << "\tSpeed: " << static_cast<int>(speed_) << std::endl
       << "\tPower: " << static_cast<int>(power_) << std::endl
       << "\tEndState: " << static_cast<int>(endState_);
}

bool GotoAbsolutePositionPortOutputCommandMessage::parseSubCommand(size_t& itr)
{
    pos_ = Get32(buffer_, itr);
    speed_ = buffer_[itr++];
    power_ = buffer_[itr++];
    endState_ = buffer_[itr++];
    return true;
}

void GotoAbsolutePositionPortOutputCommandMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    PortOutputCommandMessage::toBuffer(buf);
    Set32(buf, pos_);
    buf.push_back(speed_);
    buf.push_back(power_);
    buf.push_back(endState_);
    buf.push_back(0);
}

WriteDirectModeDataPortOutputCommandMessage::WriteDirectModeDataPortOutputCommandMessage()
{
    subCommand_ = SubCommand::WriteDirectModeData;
}

void WriteDirectModeDataPortOutputCommandMessage::toString(std::stringstream& ss) const
{
    PortOutputCommandMessage::toString(ss);
    ss << std::endl << "\tMode: " << static_cast<int>(mode_);
}

bool WriteDirectModeDataPortOutputCommandMessage::parseSubCommand(size_t& itr)
{
    mode_ = buffer_[itr++];
    while (itr < size_) {
        payload_.push_back(buffer_[itr++]);
    }
    return true;
}

void WriteDirectModeDataPortOutputCommandMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    PortOutputCommandMessage::toBuffer(buf);
    buf.push_back(mode_);
    buf.insert(buf.end(), payload_.begin(), payload_.end());
}

PortOutputCommandFeedbackMessage::PortOutputCommandFeedbackMessage()
{
    type_ = Type::PortOutputCommandFeedback;
}

bool PortOutputCommandFeedbackMessage::parseBody(size_t& itr)
{
    portId1_ = buffer_[itr++];
    message1_ = Utils::to_enum<FeedbackMessage>(buffer_[itr++]);
    if (itr < size_) {
        portId2_ = buffer_[itr++];
        message2_ = Utils::to_enum<FeedbackMessage>(buffer_[itr++]);
    }
    if (itr < size_) {
        portId3_ = buffer_[itr++];
        message3_ = Utils::to_enum<FeedbackMessage>(buffer_[itr++]);
    }
    return true;
}

void PortOutputCommandFeedbackMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId1_) << std::endl
       << "\tMessage: " << Utils::enum_value(message1_) << " " << EnumName(message1_);
    if (portId2_ != 0xff) {
        ss << std::endl
           << "\tPort ID 2: " << static_cast<int>(portId2_) << std::endl
           << "\tMessage 2: " << EnumName(message2_);
    }
    if (portId3_ != 0xff) {
        ss << std::endl
           << "\tPort ID 3: " << static_cast<int>(portId3_) << std::endl
           << "\tMessage 3: " << EnumName(message3_);
    }
}

} // namespace Ancorage::BLE
