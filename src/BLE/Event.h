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
    enum class Type {
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
    std::string ToString();

protected:
    bool parse(uint8_t* buffer, size_t size);
    bool parseHeader(size_t& itr);
    virtual bool parseBody(size_t& itr) = 0;
    virtual void toString(std::stringstream& ss) = 0;

    uint16_t size_ = 0;
    Type type_ = Type::Unknown;
    std::vector<uint8_t> buffer_;
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
    void toString(std::stringstream& ss) override;

    uint8_t portId_ = 0xff;
    Event event_ = Event::Unknown;
    IOTypeID ioTypeId_ = IOTypeID::Unknown;
    uint32_t hardwareRev_ = UINT32_MAX;
    uint32_t softwareRev_ = UINT32_MAX;
    uint8_t portA_ = UINT8_MAX;
    uint8_t portB_ = UINT8_MAX;
};
} // namespace Ancorage::BLE
