import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QLexis
import "utils.js" as Utils

Rectangle {
  id: newItem
  property list<string> displayedTypes // translated type names
  property list<string> types
  property int typesNum
  property bool editMode
  property int itemID
  property int currentType
  property string title
  property string image
  property color backgroundColor: "whitesmoke"
  property string meaning

  function init() {
    editMode = true
    type.currentIndex = currentType
    titleItem.setText(title)
    imageItem.source = image
    imageItem.visible = image.length > 0
    cover.color = backgroundColor
  }

  function clear() {
    editMode = false
    type.currentIndex = 0
    titleItem.clear()
    imageItem.source = ""
    imageItem.visible = false
    cover.color = "whitesmoke"
    customColor.Material.background = "white";
    customColor.icon.color = "black";
  }

  ColumnLayout {
    id: metaData
    anchors.fill: parent
    anchors.margins: 20

    LexisBox {
      id: type
      currentIndex: currentType
      Layout.fillWidth: true
      model: displayedTypes.slice(0, typesNum)
    }

    TextComplete {
      id: titleItem
      text: title
      Layout.fillWidth: true
      placeholder: qsTr("Title")
    }

    RowLayout {
      Layout.alignment: Qt.AlignHCenter
      Layout.fillWidth: true
      Layout.topMargin: 20
      spacing: 20
      Repeater {
        id: backgroundColors
        model: ["whitesmoke", "#a8e6cf", "#dcedc1", "#ffd3b6", "#ffaaa5",
                "#ff8b94", "#1b85b8", "#c3cb71", "#ffd4e5", "#e4ddd7",
                "#d4ffea", "#eecbff", "#feffa3", "#dbdcff", "#e3dfa4"]
        RoundButton {
          required property string modelData
          Material.background: modelData
          onClicked: cover.color = modelData
        }
      }

      RoundButton {
        id: customColor
        icon.source: "qrc:/qt/qml/QLexis/icons/plus.png"
        icon.color: "black"
        Material.background: "white"
        onClicked: colorDialog.open()

        ColorDialog {
          id: colorDialog
          onAccepted: {
            customColor.Material.background = selectedColor;
            customColor.icon.color = Utils.getFgColor(selectedColor);
            cover.color = selectedColor;
          }
        }
      }
    }

    Rectangle {
      id: cover
      color: backgroundColor
      Layout.fillHeight: true
      Layout.fillWidth: true
      Layout.topMargin: 20
      Layout.bottomMargin: 20
      radius: 20

      Image {
        id: imageItem
        visible: source.toString().length > 0
        source: image
        fillMode: Image.PreserveAspectFit
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.margins: 20

        MouseArea {
          anchors.fill: parent
          cursorShape: Qt.PointingHandCursor
          onClicked: (mouse) => {
            if (mouse.button === Qt.LeftButton) {
              imagePicker.init(image);
              stackView.push(imagePicker)
            }
          }
        }
      }

      PrettyButton {
        id: pickImage
        visible: !imageItem.visible
        text: qsTr("Pick an image")
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: -sideBar.width / 2

        onClicked: {
          if (imagePicker.hasQuery) {
            imagePicker.init("");
            stackView.push(imagePicker);
          }
        }

        ToolTip {
          visible: pickImage.hovered && !imagePicker.hasQuery
          text: qsTr("Insert title")
        }
      }

    }

    OkCancel {
      Layout.rightMargin: sideBar.width
      okTooltipVisible: !imagePicker.hasQuery
      okTooltipText: qsTr("Insert title")
      okay: function () {
        if (imagePicker.hasQuery) {
          if (editMode) {
            library.updateItem(newDbRecord, currentType);
          } else {
            library.addItem(newDbRecord);
          }
          libraryView.refresh();
          popStack();
          newItem.clear();
        }
      }
      cancel: function () {
        popStack();
        newItem.clear();
      }
    }
  }

  LibraryItem {
    id: newDbRecord
    itemID: newItem.itemID
    title: titleItem.text
    type: sectionTypeManager.librarySectionType(types[type.currentIndex])
    imageUrl: imageItem.source
    color: cover.color
    meaning: newItem.meaning
  }

  ImagePicker {
    id: imagePicker
    query: titleItem.text
    property bool hasQuery: query.trim().length > 0
    visible: false
  }

  Connections {
    target: imagePicker

    function onImagePicked(url) {
      imageItem.source = url
      imageItem.visible = true
    }
  }
}
