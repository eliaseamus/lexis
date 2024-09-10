import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
  id: newItem

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

      TextComplete {
        id: year
        Layout.fillWidth: true
        placeholder: qsTr("Year")
      }
    }

    Image {
      Layout.fillHeight: true
      Layout.fillWidth: true
    }
  }

  RowLayout {
    id: buttonBox
    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter
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
