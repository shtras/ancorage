#pragma once

#include "rapidjson_wrap.h"
#include "spdlog_wrap.h"

#include <string>
#include <condition_variable>
#include <mutex>
#include <optional>

namespace Ancorage::Utils
{
template <typename T>
std::optional<T> GetT(const rapidjson::WValue::ValueType& o, const wchar_t* name)
{
    if (!o.HasMember(name) || !o[name].Is<T>()) {
        return {};
    }
    return o[name].Get<T>();
}

template <typename T>
std::optional<T> GetT(const rapidjson::WValue::ConstObject& o, const wchar_t* name)
{
    if (!o.HasMember(name) || !o[name].Is<T>()) {
        return {};
    }
    return o[name].Get<T>();
}

template <>
std::optional<int8_t> GetT(const rapidjson::WValue::ConstObject& o, const wchar_t* name);

template <>
std::optional<uint8_t> GetT(const rapidjson::WValue::ConstObject& o, const wchar_t* name);

template <typename T>
std::optional<T> GetT(const rapidjson::WValue::ValueType& o)
{
    if (!o.Is<T>()) {
        spdlog::error("Json value is unexpected");
        return {};
    }
    return o.Get<T>();
}

template <typename E>
using enable_enum_t = std::enable_if_t<std::is_enum_v<E>, std::underlying_type_t<E>>;

template <typename E>
constexpr enable_enum_t<E> enum_value(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template <typename E, typename T = int>
constexpr std::enable_if_t<std::is_enum_v<E> && std::is_integral_v<T>, E> to_enum(T value) noexcept
{
    return static_cast<E>(value);
}

std::wstring ReadFile(const std::string& fileName);

void LogError(std::wstring&& msg);

float FromWparam(uint64_t p);

uint64_t FromFloat(float f);

class Semaphore
{
public:
    explicit Semaphore(int count = 0);
    void notify();
    void wait();

private:
    std::mutex mtx_;
    std::condition_variable cv_;
    int count_;
};
} // namespace Ancorage::Utils
