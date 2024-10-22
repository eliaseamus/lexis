import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: deleteLanguageDialog
  property string target
  property string imageSource
  property color backgroundColor: palette.base

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Are you sure you want to\ndelete %1 from your library?").arg(target)
      horizontalAlignment: Qt.AlignHCenter
      Layout.alignment: Qt.AlignHCenter
      Layout.topMargin: 10
      Layout.bottomMargin: 20
    }
    Rectangle {
      Layout.preferredHeight: 200
      Layout.preferredWidth: 300
      Layout.alignment: Qt.AlignCenter
      color: backgroundColor
      Image {
        id: image
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.margins: 30
        fillMode: Image.PreserveAspectFit
        source: imageSource
        visible: imageSource.length > 0
      }
      TextIcon {
        id: textIcon
        width: 150
        height: 150
        anchors.centerIn: parent
        anchors.margins: 30
        iconRadius: 180
        iconColor: backgroundColor
        iconTitle: target
        visible: !image.visible
      }
    }
    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        deleteLanguageDialog.accept()
      }
      cancel:  function () {
        deleteLanguageDialog.reject()
      }
    }
  }

}
