cmake_minimum_required(VERSION 3.16)
project(rtf_editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(rtf_editor
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
)

target_link_libraries(rtf_editor
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)