import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: moveDialog
  property string currentTable

  ColumnLayout {

    TreeView {

    }

    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        moveDialog.accept()
      }
      cancel:  function () {
        moveDialog.reject()
      }
    }
  }

}
