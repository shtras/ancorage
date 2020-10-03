#include "Port.h"

#include "Utils/Utils.h"

namespace Ancorage::ControlPlus
{
bool Port::Parse(const rapidjson::WValue::ConstObject& v)
{
    auto idO = Utils::GetT<int>(v, L"id");
    if (!idO) {
        return false;
    }
    if (*idO < 0 || *idO > UINT8_MAX) {
        spdlog::error("Port ID out of bounds");
        return false;
    }
    id_ = static_cast<uint8_t>(*idO);
    auto typeO = Utils::GetT<std::wstring>(v, L"type");
    if (!typeO) {
        return false;
    }
    if (*typeO == L"engine") {
        type_ = Type::Motor;
    } else if (*typeO == L"stepper") {
        type_ = Type::Stepper;
    } else if (*typeO == L"servo") {
        type_ = Type::Servo;
    } else {
        spdlog::error(L"Unknown type: {}", *typeO);
        return false;
    }
    return true;
}

uint8_t Port::GetId() const
{
    return id_;
}
} // namespace Ancorage::ControlPlus
