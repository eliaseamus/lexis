import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: moveDialog
  property string currentTable
  property alias model: view.model
  property alias view: view

  ColumnLayout {
    anchors.fill: parent

    TreeView {
      id: view
      Layout.preferredHeight: 300
      Layout.preferredWidth: 700
      selectionModel: ItemSelectionModel {}
      delegate: TreeViewDelegate {
        contentItem: Label {
          clip: false
          text: model.display
          elide: Text.ElideRight
          color: row === treeView.currentRow ? settings.fgColor : "black"
        }
        background : Rectangle {
          anchors.fill: parent
          color: row === treeView.currentRow ? settings.accentColor :
                 (treeView.alternatingRows && row % 2 !== 0) ? "lightgrey" : "white"
        }
      }
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
