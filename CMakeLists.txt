cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS
             Core Gui Widgets Network WebEngineWidgets Multimedia Sql
             Qml Quick QuickControls2 WebView LinguistTools REQUIRED)

include_directories(src src/db src/gui src/lang src/model)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru)

qt_add_executable(${PROJECT_NAME}
  src/main.cpp
  src/web_service.cpp
  src/web_service.hpp
  src/utils.hpp
  # src/file_downloader.cpp
  # src/file_downloader.hpp
  # src/gui/pronunciation_player.cpp
  # src/gui/pronunciation_player.hpp
  # src/gui/word_card.cpp
  # src/gui/word_card.hpp
  # src/lang/dictionary.cpp
  # src/lang/dictionary.hpp
  # src/lang/pronunciation_service.cpp
  # src/lang/pronunciation_service.hpp
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
  Qt6::WebView
)

qt_add_qml_module(${PROJECT_NAME}
  URI QLexis
  VERSION 1.0
  QML_FILES
    src/qml/DeleteDialog.qml
    src/qml/ImagePicker.qml
    src/qml/ImageWithEffect.qml
    src/qml/InterfaceLanguageDialog.qml
    src/qml/Main.qml
    src/qml/LibraryItemConfiguration.qml
    src/qml/OkCancel.qml
    src/qml/PrettyLabel.qml
    src/qml/RoundImage.qml
    src/qml/SectionView.qml
    src/qml/SelectLanguageDialog.qml
    src/qml/SideBar.qml
    src/qml/SortRoleDialog.qml
    src/qml/StartPage.qml
    src/qml/TextComplete.qml
    src/qml/TextIcon.qml
    src/qml/utils.js
  SOURCES
    src/app_manager.hpp
    src/app_manager.cpp
    src/app_settings.hpp
    src/app_settings.cpp
    src/db/library.cpp
    src/db/library.hpp
    src/lang/predictor.cpp
    src/lang/predictor.hpp
    src/model/library_item.hpp
    src/model/library_item.cpp
    src/model/library_item_model.hpp
    src/model/library_item_model.cpp
    src/model/library_item_proxy_model.hpp
    src/model/library_item_proxy_model.cpp
    src/model/library_section.hpp
    src/model/library_section.cpp
    src/model/section_type.hpp
    src/model/section_type.cpp
  RESOURCES
    src/qml/icons/audio.png
    src/qml/icons/audio-mute.png
    src/qml/icons/color.png
    src/qml/icons/language.png
    src/qml/icons/plus.png
    src/qml/icons/search.png
    src/qml/icons/sort.png
    src/qml/icons/sort-ascending.png
    src/qml/icons/sort-descending.png
    src/qml/icons/flags/de.png
    src/qml/icons/flags/en.png
    src/qml/icons/flags/es.png
    src/qml/icons/flags/fr.png
    src/qml/icons/flags/it.png
    src/qml/icons/flags/pl.png
    src/qml/icons/flags/ru.png
    src/qml/icons/flags/tr.png
    src/qml/icons/flags/uk.png
    qtquickcontrols2.conf
)

qt_add_resources(${PROJECT_NAME} "configuration"
    PREFIX "/"
    FILES qtquickcontrols2.conf
)

qt_add_translations(${PROJECT_NAME}
    TS_FILE_DIR tr)

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})
add_definitions(-DDICTIONARY_API_KEY=${DICTIONARY_API_KEY})
add_definitions(-DCSE_ID=${CSE_ID})
