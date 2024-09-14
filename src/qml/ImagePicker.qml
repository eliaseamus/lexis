import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebView

SplitView {
  orientation: Qt.Horizontal
  property string query

  Item {
    SplitView.fillWidth: true
    WebView {
      id: searchResults
      visible: false
      anchors.fill: parent
      url: "https://cse.google.com/cse?cx=%1#gsc.tab=1&gsc.q=%2".arg(CSE_ID).arg(query)

      onLoadingChanged: (loadRequest) => {
        if (query.trim().length == 0)
          return;

        switch (loadRequest.status) {
          case WebView.LoadSucceededStatus:
            runJavaScript("
              const ids = [\"cse-header\", \"cse-footer\"];
              const classes = [\"gsc-tabsArea\", \"gsc-above-wrapper-area\"];
              ids.forEach(removeElementByID);
              classes.forEach(removeElementByClassName);

              function removeElementByID(id, index, array) {
                var element = document.getElementById(id);
                element.parentNode.removeChild(element);
              }

              function removeElementByClassName(name, index, array) {
                const elements = document.getElementsByClassName(name);
                while (elements.length > 0) {
                  elements[0].parentNode.removeChild(elements[0]);
                }
              }
            ")
            searchResults.visible = true;
            spinner.visible = false;
            break;
        }
      }
    }

    BusyIndicator {
      id: spinner
      visible: true
      anchors.centerIn: parent
    }

  }

  DropArea {
    id: dropArea
    SplitView.fillWidth: true
    SplitView.minimumWidth: main.width / 2

    Rectangle {
      id: dropBackground
      anchors.fill: parent
      color: palette.window
    }

    ColumnLayout {
      anchors.fill: parent

      Image {
        id: image
        visible: false
        width: dropArea.width / 2
        fillMode: Image.PreserveAspectFit
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.margins: 50
      }

      Label {
        visible: image.status == Image.Null
        text: "Pick an image\nand drop it in this area"
        Layout.fillWidth: true
        Layout.fillHeight: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }

      OkCancel {

      }
    }

    onEntered: {
      dropBackground.color = Qt.darker(palette.window, 1.25)
    }

    onExited: {
      dropBackground.color = palette.window
    }

    onDropped: (drop) => {
      if (!drop.hasUrls) {
        return;
      }
      image.source = drop.urls[0]
      image.visible = true
      dropBackground.color = palette.window
    }
  }

  onVisibleChanged: {
    image.source = ""
    image.visible = false
    searchResults.visible = false;
    spinner.visible = true;
    searchResults.reload()
  }

}
