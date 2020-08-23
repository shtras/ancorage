#include "Utils.h"

#include "spdlog_wrap.h"

#include <Windows.h>

namespace Ancorage::Utils
{
void LogError(std::wstring&& str)
{
    spdlog::error(str);
    MessageBox(nullptr, str.c_str(), nullptr, MB_ICONERROR);
}
} // namespace Ancorage::Utils
