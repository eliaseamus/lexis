cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebEngineWidgets Multimedia)

include_directories(src/include src/gui/include src/lang/include)

qt_add_executable(lexis src/main.cpp
                        src/utils.cpp
                        src/gui/start_page.cpp
                        src/gui/include/start_page.hpp
                        src/gui/lexis_dialog.cpp
                        src/gui/include/lexis_dialog.hpp
                        src/lang/completer.cpp
                        src/lang/include/completer.hpp
                        src/lang/web_service.cpp
                        src/lang/include/web_service.hpp
                        src/lang/predictor.cpp
                        src/lang/include/predictor.hpp
                        src/lang/dictionary.cpp
                        src/lang/include/dictionary.hpp
                        src/lang/pronunciation_service.cpp
                        src/lang/include/pronunciation_service.hpp
                        src/gui/visualiser.cpp
                        src/gui/include/visualiser.hpp
                        src/gui/image.cpp
                        src/gui/include/image.hpp
                        src/gui/drag_drop_image.cpp
                        src/gui/include/drag_drop_image.hpp
                        src/gui/icon.cpp
                        src/gui/include/icon.hpp
                        src/gui/pronunciation_player.cpp
                        src/gui/include/pronunciation_player.hpp
                        src/gui/word_card.cpp
                        src/gui/include/word_card.hpp
                        src/gui/new_lexis_dialog.cpp
                        src/gui/include/new_lexis_dialog.hpp
                        src/gui/ok_cancel_box.cpp
                        src/gui/include/ok_cancel_box.hpp)

set(resource_files "icons/au.png"
                   "icons/audio.png"
                   "icons/audio-mute.png"
                   "icons/uk.png"
                   "icons/us.png")

qt_add_resources(lexis "lexis" PREFIX "/" FILES ${resource_files})

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})
add_definitions(-DDICTIONARY_API_KEY=${DICTIONARY_API_KEY})
add_definitions(-DCSE_ID=${CSE_ID})

target_link_libraries(lexis PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Widgets
  Qt6::WebEngineWidgets
  Qt6::Multimedia
)

