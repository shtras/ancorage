#include "Profile.h"
#include "Utils/Utils.h"

#include "spdlog_wrap.h"
#include "rapidjson_wrap.h"

#include <fstream>

namespace Ancorage::ControlPlus
{
bool Profile::Load(const std::string& fileName)
{
    std::wstring str;
    std::ifstream f(fileName);
    if (f.good()) {
        str = Utils::ReadFile(fileName);
    }
    rapidjson::GenericDocument<rapidjson::UTF16<>> d;
    const rapidjson::ParseResult res = d.Parse(str);
    if (res.IsError()) {
        Utils::LogError(L"Failed loading profile");
        return false;
    }
    auto hubsO = Utils::GetT<rapidjson::WValue::ConstArray>(d, L"hubs");
    if (!hubsO) {
        spdlog::error("hubs not found");
        return false;
    }
    const auto& hubsArr = *hubsO;
    for (rapidjson::SizeType i = 0; i < hubsArr.Size(); ++i) {
        auto hubO = Utils::GetT<rapidjson::WValue::ConstObject>(hubsArr[i]);
        if (!hubO) {
            return false;
        }
        auto hub = std::make_unique<Hub>();
        if (!hub->Parse(*hubO)) {
            return false;
        }
        hubs_[hub->GetName()] = std::move(hub);
    }

    return true;
}

std::list<std::wstring> Profile::GetHubs()
{
    std::list<std::wstring> res;
    for (const auto& p : hubs_) {
        res.push_back(p.second->GetName());
    }
    return res;
}

void Profile::Connect(const std::wstring& id)
{
    if (hubs_.count(id) == 0) {
        Utils::LogError(L"Hub not found");
        return;
    }
    hubs_.at(id)->Connect();
}

void Profile::Disconnect(const std::wstring& id)
{
    if (hubs_.count(id) == 0) {
        Utils::LogError(L"Hub not found");
        return;
    }
    hubs_.at(id)->Disconnect();
}

void Profile::ButtonDown(uint8_t b)
{
    (void)b;
}

void Profile::ButtonUp(uint8_t b)
{
    (void)b;
}
} // namespace Ancorage::ControlPlus
