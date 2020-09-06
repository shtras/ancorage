#pragma once

#include <string>
#include <condition_variable>
#include <mutex>

namespace Ancorage::Utils
{
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
