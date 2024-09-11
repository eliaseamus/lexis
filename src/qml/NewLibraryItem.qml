import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
  id: newItem
  property color backgroundColor: "white"
  color: backgroundColor

  ColumnLayout {
    id: metaData
    anchors.fill: parent
    anchors.topMargin: 50
    anchors.leftMargin: 50
    anchors.rightMargin: 50

    ComboBox {
      id: type
      Layout.fillWidth: true
      model: ["Subject group", "Book", "Article", "Movie", "Series", "Album", "Song"]
    }

    TextComplete {
      id: title
      Layout.fillWidth: true
      placeholder: qsTr("Title")
    }

    RowLayout {
      visible: type.currentText !== "Subject group"

      TextComplete {
        id: author
        Layout.fillWidth: true
        placeholder: qsTr("Author")
      }

      TextField {
        id: year
        placeholderText: qsTr("Year")
        validator: IntValidator {bottom: 0; top: 9999;}
      }

      CheckBox {
        id: bc
        visible: type.currentText === "Book"
        text: qsTr("BC")
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Before Christ")
      }
    }

    RowLayout {
      Layout.alignment: Qt.AlignHCenter
      Layout.fillWidth: true
      Layout.topMargin: 20
      spacing: 30
      Repeater {
        id: backgroundColors
        model: ["#ffffff", "#a8e6cf", "#dcedc1", "#ffd3b6", "#ffaaa5",
                "#ff8b94", "#1b85b8", "#c3cb71", "#ffd4e5",
                "#d4ffea", "#eecbff", "#feffa3", "#dbdcff"]
        RoundButton {
          required property string modelData
          Material.background: modelData
          onClicked: {
            backgroundColor = modelData
          }
        }
      }
    }

    Item {
      id: spacer
      Layout.fillHeight: true
    }

    Button {
      id: pickImage
      text: "Pick an image"
      Material.background: Material.accentColor
      Layout.alignment: Qt.AlignHCenter
    }

    Item {
      id: spacer2
      Layout.fillHeight: true
    }

    RowLayout {
      id: buttonBox
      Layout.alignment: Qt.AlignHCenter
      Button {
        id: ok
        text: "Ok"
        Material.background: Material.accentColor
        onClicked: stackView.pop()
      }
      Button {
        id: cancel
        text: "Cancel"
        onClicked: stackView.pop()
      }
    }

  }
}
