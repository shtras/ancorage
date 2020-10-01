#pragma once
#include <cstdint>
#include <vector>
#include <memory>
#include <sstream>

namespace Ancorage::BLE
{
class Message
{
public:
    enum class Type : uint32_t {
        HubProperties = 0x01,
        HubActions = 0x02,
        HubAlerts = 0x03,
        HubAttachedIO = 0x04,
        GenericError = 0x05,
        HWNetworkCommands = 0x06,
        FWUpdateGoIntoBootMode = 0x10,
        FWUpdateLockMemory = 0x11,
        FWUpdateLockStatusRequest = 0x12,
        FWLockStatus = 0x13,
        PortInfoRequest = 0x21,
        PortModeInfoRequest = 0x22,
        PortInputFormatSetupSingle = 0x41,
        PortInputFormatSetupCombined = 0x42,
        PortInfo = 0x43,
        PortModeInfo = 0x44,
        PortValueSingle = 0x45,
        PortValueCombined = 0x46,
        PortInputFormatSingle = 0x47,
        PortInputFormatCombined = 0x48,
        VirtualPortSetup = 0x61,
        PortOutputCommand = 0x81,
        PortOutputCommandFeedback = 0x82,
        Unknown = 0xff
    };

    virtual ~Message() = default;
    static std::unique_ptr<Message> Parse(uint8_t* buffer, size_t size);

    static bool IsType(uint8_t b);
    std::string ToString() const;
    std::vector<uint8_t> ToBuffer() const;

protected:
    bool parse(uint8_t* buffer, size_t size);
    bool parseHeader(size_t& itr);
    virtual bool parseBody(size_t&) = 0;
    virtual void toString(std::stringstream&) const = 0;
    virtual void toBuffer(std::vector<uint8_t>&) const
    {
    }

    uint16_t size_ = 0;
    Type type_ = Type::Unknown;
    std::vector<uint8_t> buffer_;
};

class HubActionsMessage : public Message
{
public:
    enum class ActionType {
        SwitchOff = 0x01,
        Disconnect = 0x02,
        VCCPortControlOn = 0x03,
        VCCPortControlOff = 0x04,
        ActivateBusyInd = 0x05,
        ResetBusyInd = 0x06,
        Shutdown = 0x2f,
        WillSwitchOff = 0x30,
        WillDisconnect = 0x31,
        WillGoIntoBoot = 0x32,
        Unknown = 0xff,
    };
    HubActionsMessage();

public:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    ActionType actionType_ = ActionType::Unknown;
};

class GenericErrorMessage : public Message
{
public:
    enum class Code {
        AKC = 0x01,
        MACK = 0x02,
        BufferOverflow = 0x03,
        Timeout = 0x04,
        CommandNotRecognized = 0x05,
        InvalidUse = 0x06,
        Overcurrent = 0x07,
        InternalError = 0x08,
        Unknown = 0xff
    };
    GenericErrorMessage();

private:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;

    Type commandType_ = Type::Unknown;
    Code code_ = Code::Unknown;
};

class HubAttachedIOMessage : public Message
{
public:
    enum class Event {
        DetachedIO = 0x00,
        AttachedIO = 0x01,
        AttachedVirtualIO = 0x02,
        Unknown = 0xff
    };
    enum class IOTypeID {
        Motor = 0x01,
        SystemTrainMotor = 0x02,
        Button = 0x05,
        LEDLight = 0x08,
        Voltage = 0x14,
        Current = 0x15,
        PiezoTone = 0x16,
        RGBLight = 0x17,
        ExternalTiltSensor = 0x22,
        MotionSensor = 0x23,
        VisionSensor = 0x25,
        ExternalMotorTacho = 0x26,
        InternalMotorTacho = 0x27,
        InternalTilt = 0x28,
        Unknown = 0xff
    };
    HubAttachedIOMessage();

private:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;

    uint8_t portId_ = 0xff;
    Event event_ = Event::Unknown;
    IOTypeID ioTypeId_ = IOTypeID::Unknown;
    uint32_t hardwareRev_ = UINT32_MAX;
    uint32_t softwareRev_ = UINT32_MAX;
    uint8_t portA_ = UINT8_MAX;
    uint8_t portB_ = UINT8_MAX;
};

class PortInputFormatSetupSingle : public Message
{
public:
    PortInputFormatSetupSingle();

    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    uint8_t portId_ = UINT8_MAX;
    uint8_t mode_ = UINT8_MAX;
    uint32_t deltaInterval_ = UINT32_MAX;
    bool notificationEnabled_ = false;
};

class PortInfoRequestMessage : public Message
{
public:
    enum class InfoType : uint8_t {
        PortValue = 0x00,
        ModeInfo = 0x01,
        PossibleCombinations = 0x02,
        Unknown = 0xff
    };

    PortInfoRequestMessage();

    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    uint8_t portId_ = UINT8_MAX;
    InfoType infoType_ = InfoType::Unknown;
};

class PortValueSingleMessage : public Message
{
public:
    PortValueSingleMessage();

private:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;

    uint8_t portId_ = UINT8_MAX;
    uint32_t value_ = 0;
};

class PortOutputCommandMessage : public Message
{
public:
    enum class SubCommand {
        StartPower = 0x02,
        SetAccTime = 0x05,
        SetDecTime = 0x06,
        StartSpeed = 0x07,
        StartSpeed2 = 0x08,
        StartSpeedForTime = 0x09,
        StartSpeedForTime2 = 0x0a,
        StartSpeedForDegrees = 0x0b,
        StartSpeedForDegrees2 = 0x0c,
        GotoAbsolutePosition = 0x0d,
        GotoAbsolutePosition2 = 0x0e,
        PresetEncoder = 0x14,
        WriteDirect = 0x50,
        WriteDirectModeData = 0x51,
        Unknown = 0xff
    };
    PortOutputCommandMessage();

public:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;
    virtual bool parseSubCommand(size_t& itr) = 0;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    uint8_t portId_ = 0xff;
    uint8_t startupCompletion_ = 0;
    SubCommand subCommand_ = SubCommand::Unknown;
};

class GotoAbsolutePositionPortOutputCommandMessage : public PortOutputCommandMessage
{
public:
    GotoAbsolutePositionPortOutputCommandMessage();

public:
    void toString(std::stringstream& ss) const override;
    bool parseSubCommand(size_t& itr) override;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    int32_t pos_ = 0;
    int8_t speed_ = 0;
    int8_t power_ = 0;
    int8_t endState_ = 0;
};

class WriteDirectModeDataPortOutputCommandMessage : public PortOutputCommandMessage
{
public:
    WriteDirectModeDataPortOutputCommandMessage();

public:
    void toString(std::stringstream& ss) const override;
    bool parseSubCommand(size_t& itr) override;
    void toBuffer(std::vector<uint8_t>& buf) const override;

    uint8_t mode_ = 0xff;
    std::vector<uint8_t> payload_;
};

class PortOutputCommandFeedbackMessage : public Message
{
public:
    enum class FeedbackMessage {
        BufferEmptyInProgress = 0x01,
        BufferEmptyComplete = 0x02,
        CommandDiscarded = 0x04,
        Idle = 0x08,
        BusyFull = 0x10,
        Unknown = 0xff
    };
    PortOutputCommandFeedbackMessage();

private:
    bool parseBody(size_t& itr) override;
    void toString(std::stringstream& ss) const override;

    uint8_t portId1_ = 0xff;
    FeedbackMessage message1_ = FeedbackMessage::Unknown;
    uint8_t portId2_ = 0xff;
    FeedbackMessage message2_ = FeedbackMessage::Unknown;
    uint8_t portId3_ = 0xff;
    FeedbackMessage message3_ = FeedbackMessage::Unknown;
};

} // namespace Ancorage::BLE
