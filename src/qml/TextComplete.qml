import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Item {
  id: textComplete
  property string placeholder: ""
  Layout.preferredHeight: textField.height

  TextField {
    id: textField
    placeholderText: placeholder

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    onTextEdited: {
      completions.currentIndex = -1
      predictor.request(text)
    }
  }

  Rectangle {
    id: completionsBox
    visible: false
    radius: 5

    anchors.top: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    height: childrenRect.height

    border.width: 2
    border.color: Material.accentColor
    color: palette.window
    Column {
      anchors.top: parent.top
      anchors.left: parent.left
      anchors.right: parent.right
      Repeater {
        id: completions
        property int currentIndex: -1

        ItemDelegate {
          id: completion
          required property string modelData
          required property int index
          text: modelData
          anchors.left: parent.left
          anchors.right: parent.right
          hoverEnabled: true
          highlighted: index == completions.currentIndex
          background: Rectangle {
            property bool isTop: index == 0
            property bool isBottom: index === completions.model.length - 1
            property int kRadius: 5
            visible: highlighted || hovered
            color: Material.accentColor
            topLeftRadius: isTop ? kRadius: 0
            topRightRadius: isTop ? kRadius : 0
            bottomLeftRadius: isBottom ? kRadius: 0
            bottomRightRadius: isBottom ? kRadius : 0
          }
        }
      }
    }
  }

  Predictor {
    id: predictor
  }

  Connections {
    target: predictor

    function onPredictionsReceived(predictions) {
      completions.model = predictions
      completionsBox.visible = completions.model.length > 0 ? true : false
    }
  }

  onFocusChanged: {
    if (!textComplete.focus) {
      completionsBox.visible = false
      completions.currentIndex = -1
    }
  }

  Keys.onPressed: function onPressed(event) {
    if (completionsBox.visible) {
      switch (event.key) {
        case Qt.Key_Escape:
          completionsBox.visible = false
          completions.currentIndex = -1
          event.accepted = true
          break
        case Qt.Key_Up:
          completions.currentIndex--

          if (completions.currentIndex < 0)
            completions.currentIndex = completions.model.length - 1

          textField.text = completions.model[completions.currentIndex]
          event.accepted = true
          break
        case Qt.Key_Down:
          completions.currentIndex++

          if (completions.currentIndex > completions.model.length - 1)
              completions.currentIndex = 0

          textField.text = completions.model[completions.currentIndex]
          event.accepted = true
          break
        case Qt.Key_Return:
          textField.text = completions.model[completions.currentIndex]
          textField.accepted()
          event.accepted = true;
          completionsBox.visible = false
          break
      }
    } else {
      switch (event.key) {
        case Qt.Key_Escape:
          textField.focus = false
          event.accepted = true
          break
        case Qt.Key_Down:
          completionsBox.visible = completions.model.length > 0 ? true : false
          event.accepted = true
          break
      }
    }
  }

}
