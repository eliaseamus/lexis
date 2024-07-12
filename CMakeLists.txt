cmake_minimum_required(VERSION 3.16)
project(lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

include_directories(src/include src/gui/include src/dict/include)

qt_add_executable(lexis src/gui/main.cpp
                        src/gui/start_page.cpp
                        src/dict/completer.cpp
                        src/dict/include/completer.hpp
                        src/dict/predictor.cpp
                        src/dict/include/predictor.hpp)

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})

target_link_libraries(lexis PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
)

