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
        icon.source: "qrc:/QLexis/icons/language.png"
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
        icon.source: "qrc:/QLexis/icons/sort.png"
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
        icon.source: "qrc:/QLexis/icons/color.png"
        ToolTip {
          visible: appColor.hovered
          text: qsTr("Color")
        }
        onClicked: colorDialog.open()
      }
      RoundButton {
        id: help
        flat: true
        icon.source: "qrc:/QLexis/icons/question.png"
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
        icon.source: "qrc:/QLexis/icons/plus.png"
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
            source: modelData ? "qrc:/QLexis/icons/flags/%1.png".arg(modelData) : ""
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
                    deleteLanguageDialog.target =
                      "%1 ".arg(Utils.getFullLanguageName(modelData).toLowerCase()) +
                      qsTr("language");
                    deleteLanguageDialog.imageUrl = "qrc:/QLexis/icons/flags/%1.png".arg(modelData);
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
      library.dropTableRecursively(language);
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
