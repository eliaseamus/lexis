import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Pane {
  property string title
  property string imageUrl
  property color itemColor

  ColumnLayout {
    anchors.fill: parent
    Item {Layout.fillHeight: true}
    BusyIndicator {
      id: spinner
      visible: true
      Layout.alignment: Qt.AlignCenter
    }
    RowLayout {
      id: body
      visible: false
      Layout.fillWidth: true
      Layout.fillHeight: true
      ColumnLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: main.width / 2 - sideBar.width
        RowLayout {
          Layout.fillWidth: true
          Layout.leftMargin: 40
          Layout.rightMargin: 40
          Text {
            text: title
            font.pointSize: 40
          }
          Item {Layout.fillWidth: true}
          PrettyLabel {
            id: transcription
            visible: false
            Layout.alignment: Qt.AlignVCenter
            font.pointSize: 20
          }
        }
        Item {Layout.fillHeight: true}
      }
      Rectangle {
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.preferredWidth: main.width / 2
        radius: 10
        color: itemColor
        Icon {
          anchors.fill: parent
          anchors.centerIn: parent
          anchors.margins: 50
          image: imageUrl
          iconRadius: 600
          iconColor: itemColor
          iconTitle: title
        }
      }
    }
    Item {Layout.fillHeight: true}
    RoundButton {
      id: back
      icon.source: "icons/back.png"
      icon.color: settings.fgColor
      Layout.alignment: Qt.AlignBottom
      Material.background: settings.accentColor
      onClicked: popStack()

      ToolTip {
        visible: back.hovered
        text: qsTr("Back")
      }
    }
  }

  Connections {
    target: dictionary

    function onDefinitionsReady(definitions) {
      definitions.forEach((definition) => {
        if (definition.transcription.length > 0) {
          transcription.title = definition.transcription;
          transcription.visible = true;
        }
      });
      spinner.visible = false;
      body.visible = true;
    }
  }

  Dictionary {
    id: dictionary
  }

  function init() {
    spinner.visible = true;
    body.visible = false;
    transcription.visible = false;
    dictionary.request(title)
  }

}
