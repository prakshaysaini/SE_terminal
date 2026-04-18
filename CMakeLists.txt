cmake_minimum_required(VERSION 3.16)
project(SE_terminal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)

add_executable(SE_terminal
    main.cpp
    MainWindow.cpp
    LocalLLMService.cpp
    TerminalUI.cpp
    TerminalHighlighter.cpp
    DataAccessLayer.cpp
)

target_link_libraries(SE_terminal PRIVATE Qt6::Widgets Qt6::Network Qt6::Sql)

add_executable(test_runner
    test_runner.cpp
    TestSuite.cpp
    DataAccessLayer.cpp
)

target_link_libraries(test_runner PRIVATE Qt6::Core Qt6::Sql)
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake_clean.cmake
    COMMENT "Cleaning all build files and generated artifacts..."
)