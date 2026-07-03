import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtQuick.Effects
import "utils.js" as Utils

Item {
  id: sideBar
  width: 55
  property string languageToDelete
  Rectangle {
    anchors.fill: parent
    color: "lightgrey"
    ColumnLayout {
      anchors.fill: parent
      RoundButton {
        id: interfaceLanguage
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/language.png"
        ToolTip {
          visible: interfaceLanguage.hovered
          text: qsTr("Interface language")
        }
        onClicked: {
          languageDialog.init()
          languageDialog.open()
        }
      }
      RoundButton {
        id: sortRole
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/sort.png"
        ToolTip {
          visible: sortRole.hovered
          text: qsTr("Sort rule")
        }
        onClicked: {
          sortRoleDialog.init()
          sortRoleDialog.open()
        }
      }
      RoundButton {
        id: appColor
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/color.png"
        ToolTip {
          visible: appColor.hovered
          text: qsTr("Color")
        }
        onClicked: colorDialog.open()
      }
      RoundButton {
        id: libraryData
        visible: Qt.platform.os !== "android"
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/settings.png"
        ToolTip {
          visible: libraryData.hovered
          text: qsTr("Library data")
        }
        onClicked: libraryDataMenu.open()
      }
      RoundButton {
        id: help
        visible: Qt.platform.os !== "android"
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/question.png"
        ToolTip {
          visible: help.hovered
          text: qsTr("Help")
        }
        onClicked: helpDialog.open()
      }
      Item {
        Layout.fillHeight: true
      }
      RoundButton {
        id: newLanguage
        visible: settings.languages.length < 9
        flat: true
        icon.source: "qrc:/qt/qml/QLexis/icons/plus.png"
        ToolTip {
          visible: newLanguage.hovered
          text: qsTr("New language")
        }
        onClicked: {
          selectLanguageDialog.init(true)
          selectLanguageDialog.open()
        }
      }
      Repeater {
        model: settings.languages
        Rectangle {
          Layout.bottomMargin: 15
          Layout.alignment: Qt.AlignCenter
          width: 50
          height: 50
          radius: 50
          color: "transparent"
          border.color: {
            return settings.currentLanguage === modelData ?
                   settings.accentColor :
                   "transparent";
          }
          border.width: 20
          RoundImage {
            anchors.centerIn: parent
            imageFillMode: Image.Stretch
            anchors.fill: parent
            anchors.margins: 5
            source: modelData ? "qrc:/qt/qml/QLexis/icons/flags/%1.png".arg(modelData) : ""
            ToolTip {
              visible: mouseArea.containsMouse
              text: Utils.getFullLanguageName(modelData)
            }
            MouseArea {
              id: mouseArea
              anchors.fill: parent
              hoverEnabled: true
              cursorShape: Qt.PointingHandCursor
              acceptedButtons: Qt.LeftButton | Qt.RightButton
              onClicked: (mouse) => {
                if (mouse.button === Qt.LeftButton) {
                  settings.currentLanguage = modelData
                  libraryView.changeLanguage(modelData)
                  stackView.pop(null)
                } else if (mouse.button === Qt.RightButton) {
                  contextMenu.popup()
                }
              }
              Menu {
                id: contextMenu
                MenuItem {
                  text: qsTr("Delete")
                  onTriggered: {
                    languageToDelete = modelData;
                    var target = [];
                    target["title"] = "%1 ".arg(Utils.getFullLanguageName(modelData)
                                                     .toLowerCase()) + qsTr("language");
                    target["imageUrl"] = "qrc:/qt/qml/QLexis/icons/flags/%1.png".arg(modelData);
                    deleteLanguageDialog.targets = [target];
                    deleteLanguageDialog.open();
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  Menu {
    id: libraryDataMenu
    x: sideBar.width
    y: libraryData.y

    MenuItem {
      enabled: settings.currentLanguage.length > 0
      text: qsTr("Export language…")
      onTriggered: {
        exportLanguageDialog.currentFile = "%1.lexis".arg(settings.currentLanguage)
        exportLanguageDialog.open()
      }
    }
    MenuItem {
      text: qsTr("Import language…")
      onTriggered: importLanguageDialog.open()
    }
    MenuSeparator {}
    MenuItem {
      text: qsTr("Backup database…")
      onTriggered: backupDatabaseDialog.open()
    }
    MenuItem {
      text: qsTr("Restore database…")
      onTriggered: restoreConfirmDialog.open()
    }
  }

  FileDialog {
    id: exportLanguageDialog
    title: qsTr("Export language")
    fileMode: FileDialog.SaveFile
    defaultSuffix: "lexis"
    nameFilters: [qsTr("Lexis archive (*.lexis)")]
    onAccepted: {
      if (library.exportLanguage(settings.currentLanguage, selectedFile)) {
        dataMessageDialog.message = qsTr("Language exported successfully.")
      } else {
        dataMessageDialog.message = qsTr("Failed to export language.")
      }
      dataMessageDialog.open()
    }
  }

  FileDialog {
    id: importLanguageDialog
    title: qsTr("Import language")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("Lexis archive (*.lexis)")]
    onAccepted: {
      pendingImportFile = selectedFile
      importConfirmDialog.open()
    }
  }

  FileDialog {
    id: backupDatabaseDialog
    title: qsTr("Backup database")
    fileMode: FileDialog.SaveFile
    defaultSuffix: "db"
    nameFilters: [qsTr("SQLite database (*.db)")]
    onAccepted: {
      if (library.backupDatabase(selectedFile)) {
        dataMessageDialog.message = qsTr("Database backup created successfully.")
      } else {
        dataMessageDialog.message = qsTr("Failed to backup database.")
      }
      dataMessageDialog.open()
    }
  }

  FileDialog {
    id: restoreDatabaseDialog
    title: qsTr("Restore database")
    fileMode: FileDialog.OpenFile
    nameFilters: [qsTr("SQLite database (*.db)")]
    onAccepted: {
      if (library.restoreDatabase(selectedFile)) {
        dataMessageDialog.message = qsTr("Database restored successfully.")
      } else {
        dataMessageDialog.message = qsTr("Failed to restore database.")
      }
      dataMessageDialog.open()
    }
  }

  Dialog {
    id: dataMessageDialog
    property string message: ""
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    standardButtons: Dialog.Ok
    title: qsTr("Lexis")
    Label {
      text: dataMessageDialog.message
      wrapMode: Text.WordWrap
      width: Math.min(360, main.width - sideBar.width - 80)
    }
  }

  Dialog {
    id: importConfirmDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    title: qsTr("Import language")
    standardButtons: Dialog.Yes | Dialog.No
    Label {
      text: qsTr("Import will replace all items for the language contained in the archive. Continue?")
      wrapMode: Text.WordWrap
      width: Math.min(360, main.width - sideBar.width - 80)
    }
    onAccepted: {
      if (library.importLanguage(pendingImportFile)) {
        dataMessageDialog.message = qsTr("Language imported successfully.")
      } else {
        dataMessageDialog.message = qsTr("Failed to import language.")
      }
      dataMessageDialog.open()
    }
  }

  Dialog {
    id: restoreConfirmDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    title: qsTr("Restore database")
    standardButtons: Dialog.Yes | Dialog.No
    Label {
      text: qsTr("Restore will replace the entire local library database. Continue?")
      wrapMode: Text.WordWrap
      width: Math.min(360, main.width - sideBar.width - 80)
    }
    onAccepted: restoreDatabaseDialog.open()
  }

  property url pendingImportFile
  property url pendingRestoreFile

  Connections {
    target: library

    function onLanguageImported(language) {
      if (settings.languages.indexOf(language) === -1) {
        settings.languages.push(language)
      }
      settings.currentLanguage = language
      libraryView.changeLanguage(language)
    }

    function onDatabaseRestored() {
      syncLanguagesFromDatabase()
      libraryView.changeLanguage(settings.currentLanguage)
    }
  }

  function syncLanguagesFromDatabase() {
    settings.languages = library.registeredLanguages()
    if (settings.languages.indexOf(settings.currentLanguage) === -1) {
      settings.currentLanguage = settings.languages.length > 0 ? settings.languages[0] : ""
    }
  }

  InterfaceLanguageDialog {
    id: languageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  SortRoleDialog {
    id: sortRoleDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    onAccepted: {
      library.sections.forEach((section) => {
        section.sort(settings.sortRole)
      })
    }
  }

  ColorDialog {
    id: colorDialog
    selectedColor: settings.accentColor
    onAccepted: {
      settings.accentColor = selectedColor
      settings.fgColor = Utils.getFgColor(selectedColor)
    }
  }

  HelpDialog {
    id: helpDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  SelectLanguageDialog {
    id: selectLanguageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
  }

  DeleteDialog {
    id: deleteLanguageDialog
    x: (main.width - width) / 2
    y: (main.height - height) / 2
    parent: ApplicationWindow.overlay
    onAccepted: {
      var language = sideBar.languageToDelete;
      library.deleteLanguage(language);
      var index = settings.languages.indexOf(language);
      if (index !== -1) {
       settings.languages.splice(index, 1);
      }
      if (language === settings.currentLanguage) {
        settings.currentLanguage = ""
        libraryView.refresh();
      }
    }
  }

  Shortcut {
    sequence: "Alt+1"
    onActivated: {
      languageDialog.init()
      languageDialog.open()
    }
  }

  Shortcut {
    sequence: "Alt+2"
    onActivated: {
      sortRoleDialog.init()
      sortRoleDialog.open()
    }
  }

  Shortcut {
    sequence: "Alt+3"
    onActivated: colorDialog.open()
  }

  Shortcut {
    sequence: "Alt+4"
    onActivated: helpDialog.open()
  }
}
