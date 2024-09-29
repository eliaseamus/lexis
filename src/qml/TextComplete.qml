import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QLexis

Item {
  id: textComplete
  property string placeholder: ""
  property string text: textField.text
  Layout.preferredHeight: textField.height

  function clear() {
    textField.text = ""
  }

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

  Popup {
    id: completionsBox
    y: textField.height
    width: childrenRect.width
    visible: false
    height: completions.delegate.height * completions.model.length

    Column {
      anchors.fill: parent
      Repeater {
        id: completions
        model: []
        property int currentIndex: -1

        ItemDelegate {
          required property string modelData
          required property int index
          text: modelData
          anchors.left: parent.left
          anchors.right: parent.right
          hoverEnabled: true
          highlighted: index == completions.currentIndex
          background: Rectangle {
            visible: highlighted
            color: Material.accentColor
          }
          onHoveredChanged: {
            if (hovered)
              completions.currentIndex = index
          }
          onClicked: {
            textField.text = modelData
            textField.accepted()
            completionsBox.visible = false
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

  Keys.onPressed: (event) => {
    if (completionsBox.visible) {
      switch (event.key) {
        case Qt.Key_Escape:
          completionsBox.visible = false
          completions.currentIndex = -1
          event.accepted = true
          break
        case Qt.Key_Up:
          if (completions.currentIndex > 0) {
            completions.currentIndex--
            textField.text = completions.model[completions.currentIndex]
          }
          event.accepted = true
          break
        case Qt.Key_Down:
          if (completions.currentIndex < completions.model.length - 1) {
            completions.currentIndex++
            textField.text = completions.model[completions.currentIndex]
          }
          event.accepted = true
          break
        case Qt.Key_Return:
          textField.text = completions.model[completions.currentIndex]
          textField.accepted()
          event.accepted = true
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
          if (completions.model.length > 0) {
            completionsBox.visible = true
            completions.currentIndex = -1
          }
          event.accepted = true
          break
      }
    }
  }

}
