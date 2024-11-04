import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: helpDialog
  property var shortcutNames: [
    qsTr("Back"),
    qsTr("Search"),
    qsTr("New item"),
    qsTr("Cancel selection"),
    qsTr("Quit"),
    qsTr("Interface language"),
    qsTr("Sort rule"),
    qsTr("Color"),
    qsTr("Help")
  ]

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: -5
    anchors.topMargin: 15
    spacing: 15
    Repeater {
      model: ["Esc", "Ctrl + F", "Ctrl + N", "Esc", "Ctrl + Q",
              "Alt + 1", "Alt + 2", "Alt + 3", "Alt + 4"]
      InfoLine {
        required property string modelData
        required property int index
        infoTitle: modelData
        infoText: helpDialog.shortcutNames[index]
      }
    }

    PrettyButton {
      text: qsTr("Ok")
      Layout.alignment: Qt.AlignHCenter
      onClicked: {
        helpDialog.accept()
      }
    }

  }

}
