cmake_minimum_required(VERSION 3.16)
project(SE_terminal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(SE_terminal
    main.cpp
    LocalLLMService.cpp
    TerminalUI.cpp
)

target_link_libraries(SE_terminal PRIVATE Qt6::Widgets Qt6::Network)