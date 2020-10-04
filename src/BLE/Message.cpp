#include "Message.h"
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
        case Type::FWUpdateGoIntoBootMode:
        case Type::FWUpdateLockMemory:
        case Type::FWUpdateLockStatusRequest:
        case Type::FWLockStatus:
            break;
        case Type::PortInfoRequest:
            res = std::make_unique<PortInfoRequestMessage>();
            break;
        case Type::PortModeInfoRequest:
            res = std::make_unique<PortModeInfoRequestMessage>();
            break;
        case Type::PortInputFormatSetupSingle:
            res = std::make_unique<PortInputFormatSetupSingleMessage>();
            break;
        case Type::PortInputFormatSetupCombined:
            break;
        case Type::PortInfo:
            res = std::make_unique<PortInfoMessage>();
            break;
        case Type::PortModeInfo:
            res = std::make_unique<PortModeInfoMessage>();
            break;
        case Type::PortValueSingle:
            res = std::make_unique<PortValueSingleMessage>();
            break;
        case Type::PortValueCombined:
        case Type::PortInputFormatSingle:
        case Type::PortInputFormatCombined:
        case Type::VirtualPortSetup:
            break;
        case Type::PortOutputCommand: {
            if (size < 6) {
                return nullptr;
            }
            auto scb = buffer[5];
            switch (Utils::to_enum<PortOutputCommandMessage::SubCommand>(scb)) {
                case PortOutputCommandMessage::SubCommand::StartPower:
                case PortOutputCommandMessage::SubCommand::SetAccTime:
                case PortOutputCommandMessage::SubCommand::SetDecTime:
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeed:
                    res = std::make_unique<StartSpeedPortOutputCommandMessage>();
                    break;
                case PortOutputCommandMessage::SubCommand::StartSpeed2:
                case PortOutputCommandMessage::SubCommand::StartSpeedForTime:
                case PortOutputCommandMessage::SubCommand::StartSpeedForTime2:
                case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees:
                case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees2:
                case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition:
                case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition2:
                case PortOutputCommandMessage::SubCommand::PresetEncoder:
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
    res.reserve(16);
    res.push_back(0); // size
    res.push_back(0); // reserved hub id
    res.push_back(static_cast<uint8_t>(Utils::enum_value(type_)));
    toBuffer(res);
    assert(res.size() <= 255);
    // @TODO: Handle longer messages
    res[0] = static_cast<uint8_t>(res.size());
    return res;
}

const std::list<uint8_t>& Message::GetPorts() const
{
    return portIds_;
}

Message::Type Message::GetType() const
{
    return type_;
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

HubAttachedIOMessage::Event HubAttachedIOMessage::GetEvent() const
{
    return event_;
}

bool HubAttachedIOMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    event_ = Utils::to_enum<Event>(buffer_[itr++]);
    if (event_ == Event::AttachedIO || event_ == Event::AttachedVirtualIO) {
        uint16_t typeId = Get16(buffer_, itr);
        ioTypeId_ = Utils::to_enum<IOTypeID>(typeId);
    }
    if (event_ == Event::AttachedIO) {
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
        ss << std::endl
           << "\tIO type: " << static_cast<int>(Utils::enum_value(ioTypeId_)) << " "
           << EnumName(ioTypeId_);
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

PortInputFormatSetupSingleMessage::PortInputFormatSetupSingleMessage()
{
    type_ = Type::PortInputFormatSetupSingle;
}

bool PortInputFormatSetupSingleMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    mode_ = buffer_[itr++];
    deltaInterval_ = Get32(buffer_, itr);
    notificationEnabled_ = buffer_[itr++];
    return true;
}

void PortInputFormatSetupSingleMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId_) << std::endl
       << "\tMode" << static_cast<int>(mode_) << std::endl
       << "\tDelta Interval: " << deltaInterval_ << std::endl
       << "\tNotification Enabled: " << notificationEnabled_;
}

void PortInputFormatSetupSingleMessage::toBuffer(std::vector<uint8_t>& buf) const
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
    portIds_.push_back(portId_);
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

PortModeInfoRequestMessage::PortModeInfoRequestMessage()
{
    type_ = Type::PortModeInfoRequest;
}

bool PortModeInfoRequestMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    mode_ = buffer_[itr++];
    infoType_ = Utils::to_enum<PortModeInfoRequestMessage::InfoType>(buffer_[itr++]);
    return true;
}

void PortModeInfoRequestMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort: " << static_cast<int>(portId_) << std::endl
       << "\tMode: " << static_cast<int>(mode_) << std::endl
       << "\tInfo Type: " << EnumName(infoType_);
}

void PortModeInfoRequestMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    buf.push_back(portId_);
    buf.push_back(mode_);
    buf.push_back(Utils::enum_value(infoType_));
}

PortInfoMessage::PortInfoMessage()
{
    type_ = Type::PortInfo;
}

uint8_t PortInfoMessage::GetModeCount() const
{
    return totalModeCount_;
}

bool PortInfoMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    infoType_ = Utils::to_enum<PortInfoRequestMessage::InfoType>(buffer_[itr++]);
    if (infoType_ == PortInfoRequestMessage::InfoType::ModeInfo) {
        capabilities_ = buffer_[itr++];
        totalModeCount_ = buffer_[itr++];
        inputModes_ = Get16(buffer_, itr);
        outputModes_ = Get16(buffer_, itr);
    } else if (infoType_ == PortInfoRequestMessage::InfoType::PossibleCombinations) {
        modeCombinations_ = buffer_[itr++];
    } else {
        spdlog::error("Unknown info type: {}", infoType_);
        return false;
    }
    return true;
}

void PortInfoMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId_) << std::endl
       << "\tInfo Type: " << EnumName(infoType_);
    if (infoType_ == PortInfoRequestMessage::InfoType::ModeInfo) {
        ss << std::endl
           << "\tCapabilities: " << static_cast<int>(capabilities_) << std::endl
           << "\tTotal Mode Count: " << static_cast<int>(totalModeCount_) << std::endl
           << "\tInput Modes: " << static_cast<int>(inputModes_) << std::endl
           << "\tOutput Modes: " << static_cast<int>(outputModes_);
    } else if (infoType_ == PortInfoRequestMessage::InfoType::PossibleCombinations) {
        ss << std::endl << "\tMode Combinations: " << static_cast<int>(modeCombinations_);
    }
}

PortModeInfoMessage::PortModeInfoMessage()
{
    type_ = Type::PortModeInfo;
}

bool PortModeInfoMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    mode_ = buffer_[itr++];
    infoType_ = Utils::to_enum<PortModeInfoRequestMessage::InfoType>(buffer_[itr++]);
    switch (infoType_) {
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Name:
            std::copy_n(&buffer_[itr], size_ - itr, name_.data());
            itr = size_;
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Raw:
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Pct:
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Si:
            min_ = *(float*)(&buffer_[itr]);
            itr += 4;
            max_ = *(float*)(&buffer_[itr]);
            itr += 4;
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Symbol:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Mapping:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::MotorBias:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Capability:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::ValueFormat:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Unknown:
            break;
        default:
            break;
    }
    return true;
}

void PortModeInfoMessage::toString(std::stringstream& ss) const
{
    ss << std::endl
       << "\tPort ID: " << static_cast<int>(portId_) << std::endl
       << "\tMode: " << static_cast<int>(mode_) << std::endl
       << "\tInfo Type: " << EnumName(infoType_);
    switch (infoType_) {
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Name:
            ss << std::endl << "\tName: " << name_.data();
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Raw:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Pct:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Si:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Symbol:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Mapping:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::MotorBias:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::Capability:
            break;
        case Ancorage::BLE::PortModeInfoRequestMessage::InfoType::ValueFormat:
            break;
    }
}

PortValueSingleMessage::PortValueSingleMessage()
{
    type_ = Type::PortValueSingle;
}

uint32_t PortValueSingleMessage::GetValue() const
{
    return value_;
}

bool PortValueSingleMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
    if (size_ == 5) {
        value_ = buffer_[itr++];
    } else if (size_ == 6) {
        value_ = static_cast<int16_t>(Get16(buffer_, itr));
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
       << "\tValue: " << static_cast<int>(value_);
}

PortOutputCommandMessage::PortOutputCommandMessage()
{
    type_ = Type::PortOutputCommand;
}

bool PortOutputCommandMessage::parseBody(size_t& itr)
{
    portId_ = buffer_[itr++];
    portIds_.push_back(portId_);
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

StartSpeedPortOutputCommandMessage::StartSpeedPortOutputCommandMessage()
{
    subCommand_ = SubCommand::StartSpeed;
}

void StartSpeedPortOutputCommandMessage::toString(std::stringstream& ss) const
{
    PortOutputCommandMessage::toString(ss);
    ss << std::endl
       << "\tSpeed: " << static_cast<int>(speed_) << std::endl
       << "\tMaxPower: " << static_cast<int>(maxPower_) << std::endl
       << "\tUseProfile: " << static_cast<int>(useProfile_);
}

bool StartSpeedPortOutputCommandMessage::parseSubCommand(size_t& itr)
{
    speed_ = buffer_[itr++];
    maxPower_ = buffer_[itr++];
    useProfile_ = buffer_[itr++];
    return true;
}

void StartSpeedPortOutputCommandMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    PortOutputCommandMessage::toBuffer(buf);
    buf.push_back(speed_);
    buf.push_back(maxPower_);
    buf.push_back(useProfile_);
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
    payload_.insert(payload_.end(), buffer_.begin() + itr, buffer_.end());
    itr = size_;
    return true;
}

void WriteDirectModeDataPortOutputCommandMessage::toBuffer(std::vector<uint8_t>& buf) const
{
    PortOutputCommandMessage::toBuffer(buf);
    buf.push_back(mode_);
    buf.insert(buf.end(), payload_.begin(), payload_.end());
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

PortOutputCommandFeedbackMessage::PortOutputCommandFeedbackMessage()
{
    type_ = Type::PortOutputCommandFeedback;
}

bool PortOutputCommandFeedbackMessage::parseBody(size_t& itr)
{
    portId1_ = buffer_[itr++];
    portIds_.push_back(portId1_);
    message1_ = Utils::to_enum<FeedbackMessage>(buffer_[itr++]);
    if (itr < size_) {
        portId2_ = buffer_[itr++];
        portIds_.push_back(portId2_);
        message2_ = Utils::to_enum<FeedbackMessage>(buffer_[itr++]);
    }
    if (itr < size_) {
        portId3_ = buffer_[itr++];
        portIds_.push_back(portId3_);
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
