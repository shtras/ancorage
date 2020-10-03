#pragma once

#include "Hub.h"

#include <string>
#include <map>
#include <list>
#include <memory>
#include <set>

namespace Ancorage::ControlPlus
{
class Profile
{
public:
    bool Load(const std::string& fileName);
    std::list<std::wstring> GetHubs();

    void Connect(const std::wstring& id);
    void Disconnect(const std::wstring& id);

    void ButtonDown(uint8_t b);
    void ButtonUp(uint8_t b);

private:
    std::map<std::wstring, std::unique_ptr<Hub>> hubs_;
    std::set<uint8_t> pressedKeys_;
};
} // namespace Ancorage::ControlPlus
