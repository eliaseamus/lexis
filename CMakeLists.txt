cmake_minimum_required(VERSION 3.16)
project(lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebEngineWidgets)

include_directories(src/include src/gui/include src/dict/include)

qt_add_executable(lexis src/main.cpp
                        src/utils.cpp
                        src/gui/start_page.cpp
                        src/gui/include/start_page.hpp
                        src/dict/completer.cpp
                        src/dict/include/completer.hpp
                        src/dict/predictor.cpp
                        src/dict/include/predictor.hpp
                        src/dict/dictionary.cpp
                        src/dict/include/dictionary.hpp
                        src/gui/visualiser.cpp
                        src/gui/include/visualiser.hpp
                        src/gui/image.cpp
                        src/gui/include/image.hpp
                        src/gui/word_card.cpp
                        src/gui/include/word_card.hpp)

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})
add_definitions(-DDICTIONARY_API_KEY=${DICTIONARY_API_KEY})
add_definitions(-DCSE_ID=${CSE_ID})

target_link_libraries(lexis PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Network
    Qt::Widgets
    Qt::WebEngineWidgets
)

