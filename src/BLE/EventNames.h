#pragma once
#include <string_view>
#include <cassert>

namespace Ancorage::BLE
{
template <typename T>
constexpr std::string_view EnumName(T)
{
    return "";
}

template <>
constexpr std::string_view EnumName(Message::Type t)
{
    switch (t) {
        case Message::Type::HubProperties:
            return "Hub Properties";
        case Message::Type::HubActions:
            return "Hub Actions";
        case Message::Type::HubAlerts:
            return "Hub Alerts";
        case Message::Type::HubAttachedIO:
            return "Hub Attached IO";
        case Message::Type::GenericError:
            return "Generic Error";
        case Message::Type::HWNetworkCommands:
            return "H/W Network Commands";
        case Message::Type::FWUpdateGoIntoBootMode:
            return "F/W Update - Go Into Boot Mode";
        case Message::Type::FWUpdateLockMemory:
            return "F/W Update Lock memory";
        case Message::Type::FWUpdateLockStatusRequest:
            return "F/W Update Lock Status Request";
        case Message::Type::FWLockStatus:
            return "F/W Lock Status";
        case Message::Type::PortInfoRequest:
            return "Port Information Request";
        case Message::Type::PortModeInfoRequest:
            return "Port Mode Information Request";
        case Message::Type::PortInputFormatSetupSingle:
            return "Port Input Format Setup (Single)";
        case Message::Type::PortInputFormatSetupCombined:
            return "Port Input Format Setup (CombinedMode)";
        case Message::Type::PortInfo:
            return "Port Information";
        case Message::Type::PortModeInfo:
            return "Port Mode Information";
        case Message::Type::PortValueSingle:
            return "Port Value (Single)";
        case Message::Type::PortValueCombined:
            return "Port Value (CombinedMode)";
        case Message::Type::PortInputFormatSingle:
            return "Port Input Format (Single)";
        case Message::Type::PortInputFormatCombined:
            return "Port Input Format (CombinedMode)";
        case Message::Type::VirtualPortSetup:
            return "Virtual Port Setup";
        case Message::Type::PortOutputCommand:
            return "Port Output Command";
        case Message::Type::PortOutputCommandFeedback:
            return "Port Output Command Feedback";
        case Message::Type::Unknown:
            return "Unknown";
    }
    assert(0);
    return "N/A";
}

template <>
constexpr std::string_view EnumName(HubActionsMessage::ActionType t)
{
    switch (t) {
        case HubActionsMessage::ActionType::SwitchOff:
            return "Switch Off Hub";
        case HubActionsMessage::ActionType::Disconnect:
            return "Disconnect";
        case HubActionsMessage::ActionType::VCCPortControlOn:
            return "VCC Port Control On";
        case HubActionsMessage::ActionType::VCCPortControlOff:
            return "VCC Port Control Off";
        case HubActionsMessage::ActionType::ActivateBusyInd:
            return "Activate BUSY Indication";
        case HubActionsMessage::ActionType::ResetBusyInd:
            return "Reset BUSY Indication";
        case HubActionsMessage::ActionType::Shutdown:
            return "Shutdown the Hub without any up-stream information send";
        case HubActionsMessage::ActionType::WillSwitchOff:
            return "Hub Will Switch Off";
        case HubActionsMessage::ActionType::WillDisconnect:
            return "Hub Will Disconnect";
        case HubActionsMessage::ActionType::WillGoIntoBoot:
            return "Hub Will Go Into Boot Mode";
        case HubActionsMessage::ActionType::Unknown:
            return "Unknown";
    }
    assert(0);
    return "N/A";
}

template <>
constexpr std::string_view EnumName(GenericErrorMessage::Code t)
{
    switch (t) {
        case GenericErrorMessage::Code::AKC:
            return "ACK";
        case GenericErrorMessage::Code::MACK:
            return "MACK";
        case GenericErrorMessage::Code::BufferOverflow:
            return "Buffer Overflow";
        case GenericErrorMessage::Code::Timeout:
            return "Timeout";
        case GenericErrorMessage::Code::CommandNotRecognized:
            return "Command NOT recognized";
        case GenericErrorMessage::Code::InvalidUse:
            return "Invalid Use";
        case GenericErrorMessage::Code::Overcurrent:
            return "Overcurrent";
        case GenericErrorMessage::Code::InternalError:
            return "Internal Error";
        case GenericErrorMessage::Code::Unknown:
            return "Unknown";
    }
    assert(0);
    return "N/A";
}

template <>
constexpr std::string_view EnumName(HubAttachedIOMessage::Event t)
{
    switch (t) {
        case HubAttachedIOMessage::Event::DetachedIO:
            return "Detached I/O";
        case HubAttachedIOMessage::Event::AttachedIO:
            return "Attached I/O";
        case HubAttachedIOMessage::Event::AttachedVirtualIO:
            return "Attached Virtual I/O";
        case HubAttachedIOMessage::Event::Unknown:
            return "Unknown";
    }
    assert(0);
    return "N/A";
}

template <>
constexpr std::string_view EnumName(HubAttachedIOMessage::IOTypeID t)
{
    switch (t) {
        case HubAttachedIOMessage::IOTypeID::Motor:
            return "Motor";
        case HubAttachedIOMessage::IOTypeID::SystemTrainMotor:
            return "System Train Motor";
        case HubAttachedIOMessage::IOTypeID::Button:
            return "Button";
        case HubAttachedIOMessage::IOTypeID::LEDLight:
            return "LED Light";
        case HubAttachedIOMessage::IOTypeID::Voltage:
            return "Voltage";
        case HubAttachedIOMessage::IOTypeID::Current:
            return "Current";
        case HubAttachedIOMessage::IOTypeID::PiezoTone:
            return "Piezo Tone";
        case HubAttachedIOMessage::IOTypeID::RGBLight:
            return "RGBLight";
        case HubAttachedIOMessage::IOTypeID::ExternalTiltSensor:
            return "External Tilt Sensor";
        case HubAttachedIOMessage::IOTypeID::MotionSensor:
            return "Motion Sensor";
        case HubAttachedIOMessage::IOTypeID::VisionSensor:
            return "Vision Sensor";
        case HubAttachedIOMessage::IOTypeID::ExternalMotorTacho:
            return "External Motor with Tacho";
        case HubAttachedIOMessage::IOTypeID::InternalMotorTacho:
            return "Internal Motor with Tacho";
        case HubAttachedIOMessage::IOTypeID::InternalTilt:
            return "Internal Tilt";
        case HubAttachedIOMessage::IOTypeID::Unknown:
            return "Unknown";
    }
    return "N/A";
}

template <>
constexpr std::string_view EnumName(PortInfoRequestMessage::InfoType t)
{
    switch (t) {
        case PortInfoRequestMessage::InfoType::PortValue:
            return "Port Value";
        case PortInfoRequestMessage::InfoType::ModeInfo:
            return "Mode Info";
        case PortInfoRequestMessage::InfoType::PossibleCombinations:
            return "Possible Combinations";
        case PortInfoRequestMessage::InfoType::Unknown:
            return "Unknown";
    }
    return "N/A";
}

template <>
constexpr std::string_view EnumName(PortOutputCommandMessage::SubCommand t)
{
    switch (t) {
        case PortOutputCommandMessage::SubCommand::StartPower:
            return "StartPower(Power1, Power2)";
        case PortOutputCommandMessage::SubCommand::SetAccTime:
            return "SetAccTime(Time, ProfileNo)";
        case PortOutputCommandMessage::SubCommand::SetDecTime:
            return "SetDecTime(Time, ProfileNo)";
        case PortOutputCommandMessage::SubCommand::StartSpeed:
            return "StartSpeed(Speed, MaxPower, UseProfile)";
        case PortOutputCommandMessage::SubCommand::StartSpeed2:
            return "StartSpeed(Speed1, Speed2, MaxPower, UseProfile)";
        case PortOutputCommandMessage::SubCommand::StartSpeedForTime:
            return "StartSpeedForTime (Time, Speed, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::StartSpeedForTime2:
            return "StartSpeedForTime(Time, SpeedL, SpeedR, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees:
            return "StartSpeedForDegrees(Degrees, Speed, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::StartSpeedForDegrees2:
            return "StartSpeedForDegrees(Degrees, SpeedL, SpeedR, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition:
            return "GotoAbsolutePosition(AbsPos, Speed, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::GotoAbsolutePosition2:
            return "GotoAbsolutePosition(AbsPos1, AbsPos2, Speed, MaxPower, EndState, UseProfile)";
        case PortOutputCommandMessage::SubCommand::PresetEncoder:
            return "PresetEncoder(Position)";
        case PortOutputCommandMessage::SubCommand::WriteDirect:
            return "WriteDirect(Byte[0],Byte[0 + n])";
        case PortOutputCommandMessage::SubCommand::WriteDirectModeData:
            return "WriteDirectModeData(Mode, PayLoad[0] PayLoad [0 + n]";
        case PortOutputCommandMessage::SubCommand::Unknown:
            return "Unknown";
    }
    assert(0);
    return "N/A";
}

template <>
constexpr std::string_view EnumName(PortOutputCommandFeedbackMessage::FeedbackMessage t)
{
    switch (t) {
        case PortOutputCommandFeedbackMessage::FeedbackMessage::BufferEmptyInProgress:
            return "Buffer Empty + Command In Progress";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::BufferEmptyComplete:
            return "Empty/Completed";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::CommandDiscarded:
            return "Current Command(s) Discarded";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::Idle:
            return "Idle";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::BusyFull:
            return "Busy/Full";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::BufferEmptyCompleteIdle:
            return "Empty/Complete/Idle";
        case PortOutputCommandFeedbackMessage::FeedbackMessage::Unknown:
            return "Unknown";
    }
    return "N/A";
}

} // namespace Ancorage::BLE
