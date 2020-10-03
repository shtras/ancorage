#include "Utils.h"

#include "spdlog_wrap.h"

#include "Windows_wrap.h"
#include <cassert>

#include <fstream>

namespace Ancorage::Utils
{
std::wstring ReadFile(const std::string& fileName)
{
    std::wifstream t(fileName);
    if (!t.good()) {
        spdlog::error("Failed to open {}", fileName);
        return L"";
    }
    std::wstring str;
    t.seekg(0, std::ios::end);
    str.reserve(static_cast<size_t>(t.tellg()));
    t.seekg(0, std::ios::beg);

    str.assign(std::istreambuf_iterator<wchar_t>(t), std::istreambuf_iterator<wchar_t>());
    return str;
}

void LogError(std::wstring&& str)
{
    spdlog::error(str);
    MessageBox(nullptr, str.c_str(), nullptr, MB_ICONERROR);
}

float FromWparam(uint64_t p)
{
    auto res = (p / 1000.0F) - 1.0F;
    assert(res >= -1.0F && res <= 1.0F);
    return res;
}

uint64_t FromFloat(float f)
{
    assert(f >= -1.0F && f <= 1.0F);
    return static_cast<uint64_t>((f + 1.0F) * 1000.0F);
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
