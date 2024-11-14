import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
  id: moveDialog
  property list<int> ids
  property string sourceTable
  property alias model: view.model
  property alias view: view

  ColumnLayout {
    anchors.fill: parent

    Rectangle {
      Layout.topMargin: 10
      Layout.preferredHeight: childrenRect.height + 2
      Layout.preferredWidth: childrenRect.width + 2
      border.color: "lightgrey"
      TreeView {
        id: view
        anchors.centerIn: parent
        height: contentItem.height
        width: contentItem.width
        selectionModel: ItemSelectionModel {}
        delegate: TreeViewDelegate {
          contentItem: Label {
            clip: false
            text: {
              if (model.display === sourceTable) {
                view.selectionModel.setCurrentIndex(treeView.index(row, 0),
                                                    ItemSelectionModel.Select);
              }
              return column > 0 ? "" : model.display
            }
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
    }

    OkCancel {
      Layout.topMargin: 10
      okay: function () {
        const targetTable = view.model.columnData(view.selectionModel.currentIndex, 1);
        if (sourceTable !== targetTable) {
          libraryView.moveItems(ids, sourceTable, targetTable);
        } else {
          libraryView.clearSelectedItems();
        }
        moveDialog.accept()
      }
      cancel:  function () {
        moveDialog.reject()
      }
    }
  }

}
