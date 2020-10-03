#include "Utils.h"

#include "spdlog_wrap.h"

#include "Windows_wrap.h"
#include <cassert>

#include <fstream>

namespace Ancorage::Utils
{
template <>
std::optional<uint8_t> GetT(const rapidjson::WValue::ConstObject& o, const wchar_t* name)
{
    auto v = GetT<int>(o, name);
    if (!v) {
        return {};
    }
    if (*v < 0 || *v > UINT8_MAX) {
        return {};
    }
    return {static_cast<uint8_t>(*v)};
}

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

void LogError(std::wstring&& msg)
{
    spdlog::error(msg);
    MessageBox(nullptr, msg.c_str(), nullptr, MB_ICONERROR);
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
