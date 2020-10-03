#include "Port.h"

#include "Utils/Utils.h"

namespace Ancorage::ControlPlus
{
Port::Port(BLE::BLEManager* ble)
    : ble_(ble)
{
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
    if (continuousActions_.count(b) > 0) {
        executeAction(continuousActions_.at(b), true);
    }
    if (singleActions_.count(b) > 0) {
        executeAction(singleActions_.at(b), true);
    }
}

void Port::ButtonUp(uint8_t b)
{
    if (continuousActions_.count(b) > 0) {
        executeAction(continuousActions_.at(b), false);
    }
}

void Port::executeAction(const Action& a, bool start)
{
    if (a.type == Action::Type::Value) {
        auto m = std::make_shared<BLE::WriteDirectModeDataPortOutputCommandMessage>();
        m->portId_ = id_;
        m->startupCompletion_ = 0x11;
        m->mode_ = 0;
        m->payload_.push_back(static_cast<int8_t>(start ? a.value : a.zeroValue));
        ble_->SendBTMessage(m);
    } else if (a.type == Action::Type::Absolute) {
        auto m = std::make_shared<BLE::GotoAbsolutePositionPortOutputCommandMessage>();
        m->portId_ = id_;
        m->startupCompletion_ = 0x11;
        m->pos_ = start ? a.value : a.zeroValue;
        m->speed_ = 60;
        m->power_ = 60;
        m->endState_ = 126;
        ble_->SendBTMessage(m);
    } else if (a.type == Action::Type::Forward || a.type == Action::Type::Backward) {
        if (a.type == Action::Type::Forward) {
            position_ += a.value;
        } else {
            position_ -= a.value;
        }
        auto m = std::make_shared<BLE::GotoAbsolutePositionPortOutputCommandMessage>();
        m->portId_ = id_;
        m->startupCompletion_ = 0x11;
        m->pos_ = position_;
        m->speed_ = 60;
        m->power_ = 60;
        m->endState_ = 126;
        ble_->SendBTMessage(m);
    }
}

void Port::OnConnect()
{
}

} // namespace Ancorage::ControlPlus
