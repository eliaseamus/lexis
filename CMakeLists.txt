cmake_minimum_required(VERSION 3.16)
project(lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui WebEngineWidgets)

qt_add_executable(lexis src/gui/main.cpp)

target_link_libraries(lexis PUBLIC
    Qt::Core
    Qt::Gui
    Qt::WebEngineWidgets
)

