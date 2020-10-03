set(GUI_SOURCES "GUI/MainWindow.cpp")
set(GUI_RESOURCES "Resources/menu_stuff.rc")
set(GUI_INCLUDES "GUI/IDs.h" "GUI/MainWindow.h")
set(GLOBAL_INCLUDES "spdlog_wrap.h")
source_group("Resources" FILES ${GUI_RESOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${GUI_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${GUI_INCLUDES})
add_library(GUI ${GUI_SOURCES} ${GUI_INCLUDES})


set(ENGINE_SOURCES 
	"BLE/BLE.cpp" 
	"BLE/Message.cpp" 
	"XInput/XInputTest.cpp" 
	"Utils/Utils.cpp" 
	"ControlPlus/Profile.cpp" 
	"ControlPlus/Hub.cpp" 
	"ControlPlus/Port.cpp")

set(ENGINE_INCLUDES 
	"BLE/BLE.h" 
	"BLE/EventNames.h" 
	"BLE/Message.h" 
	"XInput/XInputTest.h" 
	"Utils/Utils.h" 
	"ControlPlus/Profile.h" 
	"ControlPlus/Hub.h" 
	"ControlPlus/Port.h")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${ENGINE_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${ENGINE_INCLUDES})
add_library(Engine ${ENGINE_SOURCES} ${ENGINE_INCLUDES})

target_link_libraries(Engine Xinput SetupAPI BluetoothApis)
add_executable(Ancorage WIN32 main.cpp ${GUI_RESOURCES} ${GLOBAL_INCLUDES})
target_link_libraries(Ancorage GUI Engine Threads::Threads)
