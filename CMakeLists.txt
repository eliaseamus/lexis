cmake_minimum_required(VERSION 3.16)
project(Lexis LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS
             Core Gui Widgets Network WebEngineWidgets Multimedia Sql
             Qml Quick QuickControls2 WebView LinguistTools REQUIRED)

include_directories(src src/db src/gui src/model src/web)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/Lexis.rc")

qt_add_executable(${PROJECT_NAME}
  src/web/dictionary.cpp
  src/web/dictionary.hpp
  src/web/predictor.cpp
  src/web/predictor.hpp
  src/web/pronunciation.cpp
  src/web/pronunciation.hpp
  src/web/web_service.cpp
  src/web/web_service.hpp
  src/main.cpp
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
  src/model/tree_item.hpp
  src/model/tree_item.cpp
  src/model/tree_model.hpp
  src/model/tree_model.cpp
  src/utils.hpp
  src/utils.cpp
  src/app_manager.hpp
  src/app_manager.cpp
  src/app_settings.hpp
  src/app_settings.cpp
  src/db/library.cpp
  src/db/library.hpp
  ${app_icon_resource_windows}
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
    src/qml/HelpDialog.qml
    src/qml/Icon.qml
    src/qml/ImagePicker.qml
    src/qml/ImageWithEffect.qml
    src/qml/InfoLine.qml
    src/qml/InputDialog.qml
    src/qml/InterfaceLanguageDialog.qml
    src/qml/ItemView.qml
    src/qml/LexisBox.qml
    src/qml/LibraryItemConfiguration.qml
    src/qml/LibraryView.qml
    src/qml/Main.qml
    src/qml/MoveDialog.qml
    src/qml/NetworkErrorScreen.qml
    src/qml/OkCancel.qml
    src/qml/PrettyButton.qml
    src/qml/PrettyLabel.qml
    src/qml/RoundImage.qml
    src/qml/SectionItem.qml
    src/qml/SectionItemMenu.qml
    src/qml/SectionView.qml
    src/qml/SelectLanguageDialog.qml
    src/qml/SideBar.qml
    src/qml/SortRoleDialog.qml
    src/qml/TextComplete.qml
    src/qml/TimeInfoDialog.qml
    src/qml/ToolBar.qml
    src/qml/utils.js
  RESOURCES
    icons/audio.png
    icons/back.png
    icons/check.png
    icons/color.png
    icons/internet-error.png
    icons/language.png
    icons/Lexis.png
    icons/plus.png
    icons/question.png
    icons/search.png
    icons/sort.png
    icons/sort-ascending.png
    icons/sort-descending.png
    icons/flags/de.png
    icons/flags/en.png
    icons/flags/es.png
    icons/flags/fr.png
    icons/flags/it.png
    icons/flags/pl.png
    icons/flags/ru.png
    icons/flags/tr.png
    icons/flags/uk.png
    qtquickcontrols2.conf
)

qt_add_resources(${PROJECT_NAME} "configuration"
    PREFIX "/"
    FILES qtquickcontrols2.conf
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_translations(${PROJECT_NAME}
    TS_FILE_DIR tr)

add_definitions(-DPREDICTOR_API_KEY=${PREDICTOR_API_KEY})
add_definitions(-DDICTIONARY_API_KEY=${DICTIONARY_API_KEY})
add_definitions(-DCSE_ID=${CSE_ID})
add_definitions(-DELEVEN_LABS_API_KEY=${ELEVEN_LABS_API_KEY})
add_definitions(-DPLAY_HT_USER=${PLAY_HT_USER})
add_definitions(-DPLAY_HT_API_KEY=${PLAY_HT_API_KEY})
