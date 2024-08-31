import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
  id: newItem

  ColumnLayout {
    id: metaData
    anchors.fill: parent
    // anchors.top: parent.top
    // anchors.left: parent.left
    // anchors.right: parent.right

    ComboBox {
      id: type
      Layout.fillWidth: true
      model: ["Subject group", "Book", "Article", "Movie", "Series", "Album", "Song"]
    }

    TextField {
      id: title
      Layout.fillWidth: true
      placeholderText: qsTr("Title")
    }

    RowLayout {
      visible: type.currentText !== "Subject group"
      TextField {
        id: author
        Layout.fillWidth: true
        placeholderText: qsTr("Author")
      }
      TextField {
        id: year
        Layout.fillWidth: true
        placeholderText: qsTr("Year")
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
      Material.background: Material.Red
      onClicked: stackView.pop()
    }
    Button {
      id: cancel
      text: "Cancel"
      onClicked: stackView.pop()
    }
  }
}
