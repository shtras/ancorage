#include "Port.h"

#include "BLE/MessageFactory.h"

#include "Utils/Utils.h"

#include <thread>

namespace Ancorage::ControlPlus
{
Port::Port(BLE::BLEManager* ble)
    : ble_(ble)
{
}

Port::~Port()
{
    if (connectT_.joinable()) {
        connectT_.join();
    }
}

bool Port::Parse(const rapidjson::WValue::ConstObject& v)
{
    auto idO = Utils::GetT<uint8_t>(v, L"id");
    if (!idO) {
        return false;
    }
    id_ = *idO;
    auto typeO = Utils::GetT<std::wstring>(v, L"type");
    if (!typeO) {
        return false;
    }
    if (*typeO == L"motor") {
        type_ = Type::Motor;
    } else if (*typeO == L"stepper") {
        type_ = Type::Stepper;
    } else if (*typeO == L"servo") {
        type_ = Type::Servo;
    } else {
        spdlog::error(L"Unknown type: {}", *typeO);
        return false;
    }

    auto mappingsO = Utils::GetT<rapidjson::WValue::ConstArray>(v, L"mappings");
    if (!mappingsO) {
        spdlog::error("Missing mappings");
        return false;
    }
    const auto& mappingsArr = *mappingsO;
    for (rapidjson::SizeType i = 0; i < mappingsArr.Size(); ++i) {
        auto mappingO = Utils::GetT<rapidjson::WValue::ConstObject>(mappingsArr[i]);
        if (!mappingO) {
            return false;
        }
        const auto& mapping = *mappingO;
        Action a;
        auto actionTypeO = Utils::GetT<std::wstring>(mapping, L"type");
        if (!actionTypeO) {
            spdlog::error("Missing mapping type");
            return false;
        }
        const std::wstring& actionType = *actionTypeO;
        if (actionType == L"value") {
            a.type = Action::Type::Value;
        } else if (actionType == L"stepForward") {
            a.type = Action::Type::Forward;
        } else if (actionType == L"stepBackward") {
            a.type = Action::Type::Backward;
        } else if (actionType == L"position") {
            a.type = Action::Type::Absolute;
        } else {
            spdlog::error(L"Unknown mapping action type: {}", actionType);
            return false;
        }

        auto fromO = Utils::GetT<rapidjson::WValue::ConstObject>(mapping, L"from");
        if (!fromO) {
            spdlog::error("Missing trigger info");
            return false;
        }
        const auto& from = *fromO;
        auto bO = Utils::GetT<std::wstring>(from, L"button");
        if (!bO) {
            spdlog::error(L"Missing button value");
            return false;
        }
        std::wstring bStr = *bO;
        if (bStr.size() != 1 || bStr[0] > UINT8_MAX) {
            spdlog::error("Incorrect button value");
            return false;
        }
        auto b = static_cast<uint8_t>(bStr[0]);
        auto buttonTypeO = Utils::GetT<std::wstring>(from, L"type");
        if (!buttonTypeO) {
            spdlog::error(L"Missing trigger type");
            return false;
        }
        const std::wstring& buttonType = *buttonTypeO;

        if (type_ == Type::Motor) {
            auto onO = Utils::GetT<int8_t>(mapping, L"on_value");
            if (!onO) {
                spdlog::error(L"Missing/incorrect on_value");
                return false;
            }
            a.value = *onO;
            if (buttonType == L"continuous") {
                auto offO = Utils::GetT<int8_t>(mapping, L"off_value");
                if (!offO) {
                    spdlog::error(L"Missing/incorrect off_value");
                    return false;
                }
                a.zeroValue = *offO;
            }
        } else {
            auto onO = Utils::GetT<int>(mapping, L"on_value");
            if (!onO) {
                spdlog::error(L"Missing/incorrect on_value");
                return false;
            }
            a.value = *onO;
            if (buttonType == L"continuous") {
                auto offO = Utils::GetT<int>(mapping, L"off_value");
                if (!offO) {
                    spdlog::error(L"Missing/incorrect off_value");
                    return false;
                }
                a.zeroValue = *offO;
            }
        }

        if (buttonType == L"single") {
            singleActions_[b] = a;
        } else if (buttonType == L"continuous") {
            continuousActions_[b] = a;
        } else {
            spdlog::error(L"Incorrect trigger type: {}", buttonType);
            return false;
        }
    }
    return true;
}

uint8_t Port::GetId() const
{
    return id_;
}

void Port::ButtonDown(uint8_t b)
{
    if (!initialized_) {
        return;
    }
    if (continuousActions_.count(b) > 0) {
        executeAction(continuousActions_.at(b), true);
    }
    if (singleActions_.count(b) > 0) {
        executeAction(singleActions_.at(b), true);
    }
}

void Port::ButtonUp(uint8_t b)
{
    if (!initialized_) {
        return;
    }
    if (continuousActions_.count(b) > 0) {
        executeAction(continuousActions_.at(b), false);
    }
}

void Port::executeAction(const Action& a, bool start)
{
    if (a.type == Action::Type::Value) {
        if (start) {
            ble_->SendBTMessage(BLE::MessageFactory::CreateStartSpeedPortOutputCommandMessage(
                id_, static_cast<int8_t>(a.value), 100, 1));
        } else {
            ble_->SendBTMessage(BLE::MessageFactory::CreateDirectModeStartPowerMessage(id_, 0));
        }
    } else if (a.type == Action::Type::Absolute) {
        auto m = BLE::MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
            id_, start ? a.value - initialPos_ : a.zeroValue - initialPos_, 40, 80, 126);
        ble_->SendBTMessage(m);
    } else if (a.type == Action::Type::Forward || a.type == Action::Type::Backward) {
        if (a.type == Action::Type::Forward) {
            position_ += a.value;
        } else {
            position_ -= a.value;
        }
        auto m = BLE::MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
            id_, position_, 60, 60, 126);
        ble_->SendBTMessage(m);
    }
}

void Port::OnMessage(const std::unique_ptr<BLE::Message>& m)
{
    spdlog::debug("Received message on port {}: {}", id_, m->ToString());
    if (!initialized_) {
        onInitializingMessage(m);
        return;
    }
}

void Port::OnConnect()
{
    if (connectT_.joinable()) {
        connectT_.join();
    }
    connectT_ = std::thread(&Port::connectProc, this);
}

void Port::onInitializingMessage(const std::unique_ptr<BLE::Message>& m)
{
    switch (m->GetType()) {
        case BLE::Message::Type::PortInfo:
            numModes_ = (dynamic_cast<BLE::PortInfoMessage*>(m.get()))->GetModeCount();
            break;
        case BLE::Message::Type::PortModeInfo: {
            auto portModeM = dynamic_cast<BLE::PortModeInfoMessage*>(m.get());
            auto n = portModeM->GetName();
            if (n == "APOS") {
                absPosMode_ = portModeM->GetMode();
            }
        } break;
        case BLE::Message::Type::PortValueSingle:
            initialPos_ =
                static_cast<int>(dynamic_cast<BLE::PortValueSingleMessage*>(m.get())->GetValue());
            position_ = initialPos_;
            break;
    }
    if (m->GetType() == nextInitMessage_) {
        if (m->GetType() == BLE::Message::Type::PortOutputCommandFeedback) {
            auto feedbackM = dynamic_cast<BLE::PortOutputCommandFeedbackMessage*>(m.get());
            if (!BLE::PortOutputCommandFeedbackMessage::IsIdle(
                    feedbackM->GetFeedbackMessage(id_))) {
                return;
            }
        }
        initSem_.notify();
    }
}

void Port::connectProc()
{
    ble_->SendBTMessage(BLE::MessageFactory::CreatePortInfoRequestMessage(
        id_, BLE::PortInfoRequestMessage::InfoType::ModeInfo));
    nextInitMessage_ = BLE::Message::Type::PortInfo;
    if (!initSem_.wait_for()) {
        spdlog::error("Port initialization failed");
        return;
    }
    for (decltype(numModes_) i = 0; i < numModes_; ++i) {
        ble_->SendBTMessage(BLE::MessageFactory::CreatePortModeInfoRequestMessage(
            id_, i, BLE::PortModeInfoRequestMessage::InfoType::Name));
        nextInitMessage_ = BLE::Message::Type::PortModeInfo;
        initSem_.wait();
    }
    if (type_ == Type::Servo || type_ == Type::Stepper) {
        if (absPosMode_ == UINT8_MAX) {
            spdlog::error("Motor doesn't support absolute position mode");
            return;
        }
        ble_->SendBTMessage(BLE::MessageFactory::CreatePortInputFormatSetupSingleMessage(
            id_, absPosMode_, 5, true));
        nextInitMessage_ = BLE::Message::Type::PortValueSingle;
        initSem_.wait();

        ble_->SendBTMessage(BLE::MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
            id_, -initialPos_, 10, 60, 127));
        nextInitMessage_ = BLE::Message::Type::PortOutputCommandFeedback;
        initSem_.wait();

        ble_->SendBTMessage(BLE::MessageFactory::CreateDirectModePresetEncoderMessage(id_, 0));
        initialPos_ = 0;
        position_ = 0;
    }

    spdlog::info("Connection sequence completed");
    initialized_ = true;
}

} // namespace Ancorage::ControlPlus
