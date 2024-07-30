cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebEngineWidgets)

include_directories(src/include src/gui/include src/lang/include)

qt_add_executable(lexis src/main.cpp
                        src/utils.cpp
                        src/gui/start_page.cpp
                        src/gui/include/start_page.hpp
                        src/gui/lexis_dialog.cpp
                        src/gui/include/lexis_dialog.hpp
                        src/lang/completer.cpp
                        src/lang/include/completer.hpp
                        src/lang/predictor.cpp
                        src/lang/include/predictor.hpp
                        src/lang/dictionary.cpp
                        src/lang/include/dictionary.hpp
                        src/gui/visualiser.cpp
                        src/gui/include/visualiser.hpp
                        src/gui/image.cpp
                        src/gui/include/image.hpp
                        src/gui/word_card.cpp
                        src/gui/include/word_card.hpp
                        src/gui/new_lexis_dialog.cpp
                        src/gui/include/new_lexis_dialog.hpp
                        src/gui/ok_cancel_box.cpp
                        src/gui/include/ok_cancel_box.hpp)

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

