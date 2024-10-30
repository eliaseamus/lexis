import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: deleteLanguageDialog
  property string target
  property string imageUrl
  property color backgroundColor: palette.base

  ColumnLayout {
    PrettyLabel {
      title: qsTr("Are you sure you want to<br>" +
                  "delete <b>%1</b> from your library?").arg(target)
      format: Text.RichText
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
      radius: 10
      Icon {
        width: 200
        height: 200
        anchors.centerIn: parent
        image: imageUrl
        iconRadius: 170
        iconColor: backgroundColor
        iconTitle: target
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
