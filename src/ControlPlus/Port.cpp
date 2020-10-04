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
        auto m = BLE::MessageFactory::CreateStartSpeedPortOutputCommandMessage(
            id_, static_cast<int8_t>(start ? a.value : a.zeroValue), 100, start ? 1 : 2);
        ble_->SendBTMessage(m);
    } else if (a.type == Action::Type::Absolute) {
        auto m = BLE::MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
            id_, start ? a.value - position_ : a.zeroValue - position_, 60, 60, 126);
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
    switch (m->GetType()) {
        case BLE::Message::Type::HubAttachedIO:
            if ((static_cast<BLE::HubAttachedIOMessage*>(m.get()))->GetEvent() ==
                BLE::HubAttachedIOMessage::Event::AttachedIO) {
                onConnect();
            }
            break;
        case BLE::Message::Type::PortInfo:
            numModes_ = (static_cast<BLE::PortInfoMessage*>(m.get()))->GetModeCount();
            initSem_.notify();
            break;
        case BLE::Message::Type::PortModeInfo:
            initSem_.notify();
            break;
        case BLE::Message::Type::PortValueSingle:
            if (!initialized_) {
                position_ = static_cast<int>(
                    static_cast<BLE::PortValueSingleMessage*>(m.get())->GetValue());
                initSem_.notify();
            }
            break;
    }
}

void Port::onConnect()
{
    connectT_ = std::thread(&Port::connectProc, this);
}

void Port::connectProc()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    ble_->SendBTMessage(BLE::MessageFactory::CreatePortInfoRequestMessage(
        id_, BLE::PortInfoRequestMessage::InfoType::ModeInfo));
    initSem_.wait();
    for (decltype(numModes_) i = 0; i < numModes_; ++i) {
        ble_->SendBTMessage(BLE::MessageFactory::CreatePortModeInfoRequestMessage(
            id_, i, BLE::PortModeInfoRequestMessage::InfoType::Raw));
        initSem_.wait();
    }
    if (type_ == Type::Servo || type_ == Type::Stepper) {
        ble_->SendBTMessage(
            BLE::MessageFactory::CreatePortInputFormatSetupSingleMessage(id_, 3, 5, true));
        spdlog::info("Connection sequence completed");
        initSem_.wait();

        ble_->SendBTMessage(BLE::MessageFactory::CreateGotoAbsolutePositionPortOutputCommandMessage(
            id_, -position_, 10, 60, 127));
    }

    initialized_ = true;
}

} // namespace Ancorage::ControlPlus
