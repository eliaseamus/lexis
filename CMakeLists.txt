cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network WebEngineWidgets Multimedia Sql)

include_directories(src src/db src/gui src/lang)

qt_add_executable(lexis src/main.cpp
                        src/utils.cpp
                        src/web_service.cpp
                        src/web_service.hpp
                        src/file_downloader.cpp
                        src/file_downloader.hpp
                        src/db/library.cpp
                        src/db/library.hpp
                        src/gui/start_page.cpp
                        src/gui/start_page.hpp
                        src/gui/dialog.cpp
                        src/gui/dialog.hpp
                        src/gui/image_picker.cpp
                        src/gui/image_picker.hpp
                        src/gui/image.cpp
                        src/gui/image.hpp
                        src/gui/drag_drop_image.cpp
                        src/gui/drag_drop_image.hpp
                        src/gui/icon.cpp
                        src/gui/icon.hpp
                        src/gui/pronunciation_player.cpp
                        src/gui/pronunciation_player.hpp
                        src/gui/word_card.cpp
                        src/gui/word_card.hpp
                        src/gui/new_library_item_dialog.cpp
                        src/gui/new_library_item_dialog.hpp
                        src/gui/ok_cancel_box.cpp
                        src/gui/ok_cancel_box.hpp
                        src/lang/completer.cpp
                        src/lang/completer.hpp
                        src/lang/predictor.cpp
                        src/lang/predictor.hpp
                        src/lang/dictionary.cpp
                        src/lang/dictionary.hpp
                        src/lang/pronunciation_service.cpp
                        src/lang/pronunciation_service.hpp
                        src/gui/library_view.cpp
                        src/gui/library_view.hpp
                        src/gui/grid_item.cpp
                        src/gui/grid_item.hpp)

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
  Qt6::Sql
)


