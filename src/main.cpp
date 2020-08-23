#include "GUI/MainWindow.h"
#include "XInput/XInputTest.h"

#include "spdlog_wrap.h"

#include <Windows.h>
#include <string>

int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int cmdShow)
{
#ifdef RELEASE
    auto sink =
        std::make_shared<spdlog::sinks::daily_file_sink_mt>("Logs/log.txt", 0, 0, false, 10);
#else
    auto sink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("Logs/log.txt", 5 * 1024 * 1024, 3);
#endif
    sink->set_level(spdlog::level::debug);
    auto logger = std::make_shared<spdlog::logger>(std::string("logger"), sink);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Application Started");

    Ancorage::GUI::MainWindow mw(hInst, cmdShow);
    mw.Init();
    mw.Run();

    Ancorage::XInput::Test1();

    mw.Join();

    spdlog::info("Bye");
    return 0;
}