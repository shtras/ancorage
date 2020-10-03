#include "GUI/MainWindow.h"
#include "XInput/XInputTest.h"
#include "BLE/BLE.h"

#include "spdlog_wrap.h"

#include "Windows_wrap.h"
#include <string>

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nShowCmd)
{
    std::list<std::shared_ptr<spdlog::sinks::sink>> sinks;
#ifdef RELEASE
    auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(
        "Logs/log.txt", 0, 0, false, static_cast<uint16_t>(10));
    sinks.push_back(dailySink);
#else
    AllocConsole();
    FILE* stream = nullptr;
    _wfreopen_s(&stream, L"CON", L"w", stdout);

    sinks.push_back(
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("Logs/log.txt", 48 * 1024, 3));
    sinks.push_back(std::make_shared<spdlog::sinks::wincolor_stdout_sink_mt>());
#endif
    auto logger = std::make_shared<spdlog::logger>(std::string{"logger"});
    for (auto& sink : sinks) {
        sink->set_level(spdlog::level::debug);
        logger->sinks().push_back(sink);
    }
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Application Started");

    Ancorage::GUI::MainWindow mw(hInst, nShowCmd);
    mw.Init();
    mw.Run();
    auto mainHwnd = mw.GetHwnd();

    Ancorage::XInput::ControllerManager m(mainHwnd);
    m.Run();

    mw.Join();
    m.Stop();
    spdlog::info("Bye");
#ifdef DEBUG
    FreeConsole();
#endif
    return 0;
}