import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: inputDialog
  width: 400
  height: 300
  property alias text: inputText.text

  ColumnLayout {
    anchors.fill: parent
    ScrollView {
      Layout.fillHeight: true
      Layout.fillWidth: true
      TextArea {
        id: inputText
        placeholderText: qsTr("Insert text")
      }
    }
    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        inputDialog.accept();
      }
      cancel:  function () {
        inputDialog.reject();
      }
    }
  }
}
