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
    color: palette.base.darker(1.1)
    ColumnLayout {
      anchors.fill: parent
      RoundButton {
        id: interfaceLanguage
        flat: true
        icon.source: "icons/language.png"
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
        icon.source: "icons/sort.png"
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
        icon.source: "icons/color.png"
        ToolTip {
          visible: appColor.hovered
          text: qsTr("Color")
        }
        onClicked: colorDialog.open()
      }
      Item {
        Layout.fillHeight: true
      }
      RoundButton {
        id: newLanguage
        visible: settings.languages.length < 9
        flat: true
        icon.source: "icons/plus.png"
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
          Layout.leftMargin: 2
          Layout.bottomMargin: 15
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
          Rectangle {
            anchors.centerIn: parent
            width: 40
            height: 40
            radius: 40
            color: "transparent"
            RoundImage {
              anchors.centerIn: parent
              anchors.fill: parent
              source: modelData ? "icons/flags/%1.png".arg(modelData) : ""
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
                    if (settings.currentLanguage !== modelData) {
                      settings.currentLanguage = modelData
                      library.changeLanguage(modelData)
                      stackView.pop(null)
                      startPage.refresh()
                    }
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
                      deleteLanguageDialog.imageSource = "icons/flags/%1.png".arg(modelData);
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
  }

  Connections {
    target: deleteLanguageDialog

    function onAccepted() {
      var language = languageToDelete;
      library.deleteLanguage(language);
      var index = settings.languages.indexOf(language);
      if (index !== -1) {
       settings.languages.splice(index, 1);
      }
      if (language === settings.currentLanguage) {
        settings.currentLanguage = ""
        startPage.refresh();
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
}
