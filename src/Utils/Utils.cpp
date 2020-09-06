#include "Utils.h"

#include "spdlog_wrap.h"

#include <Windows.h>
#include <assert.h>

namespace Ancorage::Utils
{
void LogError(std::wstring&& str)
{
    spdlog::error(str);
    MessageBox(nullptr, str.c_str(), nullptr, MB_ICONERROR);
}

float FromWparam(uint64_t p)
{
    auto res = (p / 1000.0f) - 1.0f;
    assert(res >= -1.0f && res <= 1.0f);
    return res;
}

uint64_t FromFloat(float f)
{
    assert(f >= -1.0f && f <= 1.0f);
    return static_cast<uint64_t>((f + 1.0f) * 1000.0f);
}

Semaphore::Semaphore(int count /* = 0*/)
    : count_(count)
{
}

void Semaphore::notify()
{
    std::unique_lock<std::mutex> lock(mtx_);
    ++count_;
    cv_.notify_one();
}

void Semaphore::wait()
{
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return count_ > 0; });
    --count_;
}
} // namespace Ancorage::Utils
