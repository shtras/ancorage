#include "Profile.h"
#include "Utils/Utils.h"

#include "rapidjson/document.h"

#include <fstream>

namespace Ancorage::ControlPlus
{
bool Profile::Load(const std::string& fileName)
{
    std::string str;
    std::ifstream f(fileName);
    if (f.good()) {
        str = Utils::ReadFile(fileName);
    }
    rapidjson::Document d;
    const rapidjson::ParseResult res = d.Parse(str);
    if (res.IsError()) {
        Utils::LogError(L"Failed loading profile");
        return false;
    }

    // Test init here LOL

    hubs_[L"Buggy"] = std::make_unique<HubHandler>(L"90842b5480f3#8&207f92e2", L"Buggy");
    hubs_[L"Truck"] = std::make_unique<HubHandler>(L"90842b596c22#8&1572164", L"Truck");

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
}

void Profile::ButtonUp(uint8_t b)
{
}
} // namespace Ancorage::ControlPlus
