#include "Event.h"
#include "Utils/Utils.h"

#include "spdlog_wrap.h"
#include "magic_enum.hpp"

namespace Ancorage::BLE
{
uint16_t Get16(const std::vector<uint8_t> b, size_t& itr)
{
    uint16_t res = (b[itr + 1] << 8) | b[itr];
    itr += 2;
    return res;
}

uint32_t Get32(const std::vector<uint8_t> b, size_t& itr)
{
    auto low = Get16(b, itr);
    auto high = Get16(b, itr);

    return (low << 16) | high;
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
            break;
        case Type::HubAlerts:
            break;
        case Type::HubAttachedIO:
            res = std::make_unique<HubAttachedIOMessage>();
            break;
        case Type::GenericError:
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
            break;
        case Type::PortModeInfoRequest:
            break;
        case Type::PortInputFormatSetupSingle:
            break;
        case Type::PortInputFormatSetupCombined:
            break;
        case Type::PortInfo:
            break;
        case Type::PortModeInfo:
            break;
        case Type::PortValueSingle:
            break;
        case Type::PortValueCombined:
            break;
        case Type::PortInputFormatSingle:
            break;
        case Type::PortInputFormatCombined:
            break;
        case Type::VirtualPortSetup:
            break;
        case Type::PortOutputCommand:
            break;
        case Type::PortOutputCommandFeedback:
            break;
    }
    if (!res) {
        spdlog::error("Unsupported message type: {}",
            magic_enum::enum_name(Utils::to_enum<Message::Type>(b)));
        return nullptr;
    }
    if (!res->parse(buffer, size)) {
        spdlog::error("Message parsing failed");
        return nullptr;
    }
    return res;
}

std::string Message::ToString()
{
    std::stringstream ss;
    ss << "Type: " << magic_enum::enum_name(type_);
    toString(ss);
    return ss.str();
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

void HubAttachedIOMessage::toString(std::stringstream& ss)
{
    ss << std::endl
       << "Port: " << static_cast<int>(portId_) << std::endl
       << "Event: " << magic_enum::enum_name(event_);
    if (event_ == Event::AttachedIO || event_ == Event::AttachedVirtualIO) {
        ss << std::endl << "IO type: " << magic_enum::enum_name(ioTypeId_);
    }
    if (event_ == Event::AttachedIO) {
        ss << std::endl
           << "Hardware revision: " << hardwareRev_ << std::endl
           << "Software revision: " << softwareRev_;
    }
    if (event_ == Event::AttachedVirtualIO) {
        ss << std::endl << "Port ID A: " << portA_ << std::endl << "Port ID B: " << portB_;
    }
}
} // namespace Ancorage::BLE
