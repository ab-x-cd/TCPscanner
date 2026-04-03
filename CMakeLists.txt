cmake_minimum_required(VERSION 3.10)
project(TCPScanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration (optionnel)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Network QUIET)

# Déterminer les sources selon la disponibilité de Qt6
set(TCP_SCANNER_SOURCES
	main.cpp
	src/model/ScannerModel.cpp
	src/controller/ScannerController.cpp
)

if(Qt6_FOUND)
	list(APPEND TCP_SCANNER_SOURCES src/view/QtView.cpp)
	message(STATUS "Qt6 found: GUI support enabled")
else()
	message(STATUS "Qt6 not found: Building CLI-only version")
endif()

add_executable(tcp_scanner ${TCP_SCANNER_SOURCES})

target_include_directories(tcp_scanner PRIVATE
	src/controller
	src/model
	src/view
)

if(Qt6_FOUND)
	target_compile_definitions(tcp_scanner PRIVATE QT_FOUND)
endif()

if(Qt6_FOUND)
	target_link_libraries(tcp_scanner PRIVATE
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
		Qt6::Network
	)
endif()

if (WIN32)
	target_link_libraries(tcp_scanner PRIVATE ws2_32)
endif()

enable_testing()

# Test de base - compilation et exécution
add_test(NAME basic_functionality COMMAND tcp_scanner 127.0.0.1 22 22)

# Test avec IP invalide (devrait échouer)
add_test(NAME invalid_ip_test COMMAND tcp_scanner 999.999.999.999 80 80)
set_tests_properties(invalid_ip_test PROPERTIES WILL_FAIL TRUE)

# Test avec ports invalides (devrait échouer)
add_test(NAME invalid_ports_test COMMAND tcp_scanner 127.0.0.1 80 20)
set_tests_properties(invalid_ports_test PROPERTIES WILL_FAIL TRUE)

# Test avec plage de ports
add_test(NAME port_range_test COMMAND tcp_scanner 127.0.0.1 80 85)

# Test avec port unique
add_test(NAME single_port_test COMMAND tcp_scanner 127.0.0.1 443 443)

# Test de performance - plusieurs ports
add_test(NAME performance_test COMMAND tcp_scanner 127.0.0.1 12340 12349)