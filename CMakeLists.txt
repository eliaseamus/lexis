cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS
             Core Gui Widgets Network WebEngineWidgets Multimedia Sql
             Qml Quick QuickControls2 REQUIRED)

include_directories(src src/db src/gui src/lang)

qt_add_executable(${PROJECT_NAME}
  src/main.cpp
  src/web_service.cpp
  src/web_service.hpp
  # src/utils.cpp
  # src/file_downloader.cpp
  # src/file_downloader.hpp
  # src/db/library.cpp
  # src/db/library.hpp
  # src/gui/start_page.cpp
  # src/gui/start_page.hpp
  # src/gui/dialog.cpp
  # src/gui/dialog.hpp
  # src/gui/image_picker.cpp
  # src/gui/image_picker.hpp
  # src/gui/image.cpp
  # src/gui/image.hpp
  # src/gui/drag_drop_image.cpp
  # src/gui/drag_drop_image.hpp
  # src/gui/icon.cpp
  # src/gui/icon.hpp
  # src/gui/pronunciation_player.cpp
  # src/gui/pronunciation_player.hpp
  # src/gui/word_card.cpp
  # src/gui/word_card.hpp
  # src/gui/new_library_item_dialog.cpp
  # src/gui/new_library_item_dialog.hpp
  # src/gui/ok_cancel_box.cpp
  # src/gui/ok_cancel_box.hpp
  # src/lang/completer.cpp
  # src/lang/completer.hpp
  # src/lang/dictionary.cpp
  # src/lang/dictionary.hpp
  # src/lang/pronunciation_service.cpp
  # src/lang/pronunciation_service.hpp
  # src/gui/library_view.cpp
  # src/gui/library_view.hpp
  # src/gui/grid_item.cpp
  # src/gui/grid_item.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Widgets
  Qt6::WebEngineWidgets
  Qt6::Multimedia
  Qt6::Sql
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
)

qt_add_qml_module(${PROJECT_NAME}
  URI QLexis
  VERSION 1.0
  QML_FILES
    src/qml/Main.qml
    src/qml/StartPage.qml
    src/qml/NewLibraryItem.qml
    src/qml/ButtonsBox.qml
    src/qml/TextComplete.qml
  SOURCES
    src/lang/predictor.cpp
    src/lang/predictor.hpp
  RESOURCES
    icons/au.png
    icons/audio.png
    icons/audio-mute.png
    icons/uk.png
    icons/us.png
    qtquickcontrols2.conf
)

qt_add_resources(${PROJECT_NAME} "configuration"
    PREFIX "/"
    FILES qtquickcontrols2.conf
)

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})
add_definitions(-DDICTIONARY_API_KEY=${DICTIONARY_API_KEY})
add_definitions(-DCSE_ID=${CSE_ID})
