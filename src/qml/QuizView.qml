import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QLexis
import "utils.js" as Utils

Pane {
  id: quizView
  property int scopeRootId: 0
  property string scopeTitle: ""
  property var scopeWords: []
  property var questions: []
  property var answerCache: ({})
  property int currentIndex: 0
  property int score: 0
  property int wrongCount: 0
  property int loadTotal: 0
  property int loadDone: 0
  property int loadProgress: 0
  property string phase: "loading"
  property string errorMessage: ""
  property var currentOptions: []
  property string correctAnswer: ""
  property int selectedOption: -1
  property var pendingResolve: null
  property var lookupQueue: []
  property bool lookupBusy: false

  property var currentWord: currentIndex >= 0 && currentIndex < questions.length
                            ? questions[currentIndex] : null
  property color cardColor: {
    if (!currentWord || !currentWord.color || currentWord.color.length === 0) {
      return settings.accentColor
    }
    return currentWord.color
  }
  property color cardFgColor: Utils.getFgColor(cardColor)

  ColumnLayout {
    anchors.fill: parent
    anchors.margins: 20

    RowLayout {
      Layout.fillWidth: true
      Layout.rightMargin: sideBar.width
      RoundButton {
        icon.source: "qrc:/qt/qml/QLexis/icons/back.png"
        icon.color: settings.fgColor
        Material.background: settings.accentColor
        onClicked: popStack()
        ToolTip {
          visible: parent.hovered
          text: qsTr("Back")
        }
      }
      Item { Layout.fillWidth: true }
      Label {
        visible: phase === "question" || phase === "feedback"
        color: settings.fgColor
        text: qsTr("Question %1 of %2").arg(currentIndex + 1).arg(questions.length)
      }
      Label {
        visible: phase === "question" || phase === "feedback"
        Layout.leftMargin: 20
        color: settings.fgColor
        text: qsTr("Score: %1").arg(score)
      }
    }

    Item { Layout.fillHeight: true }

    ColumnLayout {
      Layout.fillWidth: true
      Layout.alignment: Qt.AlignHCenter
      Layout.rightMargin: sideBar.width
      Layout.maximumWidth: 640
      spacing: 16

      PrettyLabel {
        visible: phase === "loading"
        Layout.alignment: Qt.AlignHCenter
        title: qsTr("Loading quiz…")
      }

      Label {
        visible: phase === "loading"
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        color: settings.fgColor
        text: qsTr("%1%").arg(loadProgress)
      }

      ProgressBar {
        visible: phase === "loading"
        Layout.fillWidth: true
        from: 0
        to: 100
        value: loadProgress
      }

      Label {
        visible: phase === "error"
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: settings.fgColor
        text: errorMessage
      }

      PrettyButton {
        visible: phase === "error"
        Layout.alignment: Qt.AlignHCenter
        text: qsTr("Back to library")
        onClicked: popStack()
      }

      Rectangle {
        visible: phase === "question" || phase === "feedback"
        Layout.fillWidth: true
        color: cardColor
        radius: 12
        implicitHeight: cardLayout.implicitHeight + 32

        ColumnLayout {
          id: cardLayout
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.top: parent.top
          anchors.margins: 16
          spacing: 12

          Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: cardFgColor
            font.pointSize: 36
            text: currentWord ? currentWord.title : ""
          }

          RowLayout {
            Layout.alignment: Qt.AlignHCenter
            RoundButton {
              icon.source: "qrc:/qt/qml/QLexis/icons/audio.png"
              icon.color: cardFgColor
              Material.background: cardColor
              enabled: currentWord !== null
              onClicked: {
                pronunciation.source = library.readAudio(currentWord.itemId)
                pronunciation.play()
              }
              ToolTip {
                visible: parent.hovered
                text: qsTr("Pronunciation")
              }
            }
          }

          Label {
            Layout.fillWidth: true
            Layout.topMargin: 8
            color: cardFgColor
            text: qsTr("Choose the translation:")
          }

          Repeater {
            model: currentOptions
            Button {
              id: optionButton
              required property int index
              required property string modelData
              Layout.fillWidth: true
              flat: true
              enabled: phase === "question"

              readonly property color fillColor: {
                if (phase === "feedback" && selectedOption >= 0) {
                  if (modelData === correctAnswer) {
                    return "#2e7d32"
                  }
                  if (index === selectedOption) {
                    return "#c62828"
                  }
                }
                return cardFgColor === "black"
                       ? Qt.darker(cardColor, 1.12)
                       : Qt.lighter(cardColor, 1.18)
              }

              readonly property color labelColor: {
                if (phase === "feedback" && (modelData === correctAnswer || index === selectedOption)) {
                  return "white"
                }
                return cardFgColor
              }

              background: Rectangle {
                radius: 8
                color: optionButton.fillColor
                border.color: cardFgColor === "black"
                              ? Qt.darker(cardColor, 1.25)
                              : Qt.lighter(cardColor, 1.3)
                border.width: 1
              }

              contentItem: Label {
                text: optionButton.modelData
                color: optionButton.labelColor
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                padding: 8
              }

              onClicked: selectOption(index)
            }
          }
        }
      }

      ColumnLayout {
        visible: phase === "done"
        Layout.alignment: Qt.AlignHCenter
        spacing: 16

        PrettyLabel {
          Layout.alignment: Qt.AlignHCenter
          title: qsTr("Quiz complete!")
          font.pointSize: 28
        }

        Label {
          Layout.alignment: Qt.AlignHCenter
          color: settings.fgColor
          text: qsTr("You got %1 out of %2 correct.").arg(score).arg(score + wrongCount)
        }

        ColumnLayout {
          Layout.alignment: Qt.AlignHCenter
          Layout.fillWidth: true
          Layout.topMargin: 8
          spacing: 8

          InfoLine {
            Layout.fillWidth: true
            infoTitle: qsTr("Correct:")
            infoText: String(score)
          }
          InfoLine {
            Layout.fillWidth: true
            infoTitle: qsTr("Wrong:")
            infoText: String(wrongCount)
          }
          InfoLine {
            Layout.fillWidth: true
            infoTitle: qsTr("Accuracy:")
            infoText: (score + wrongCount) > 0
                      ? qsTr("%1%").arg(Math.round(100 * score / (score + wrongCount)))
                      : qsTr("0%")
          }
        }

        RowLayout {
          Layout.alignment: Qt.AlignHCenter
          spacing: 12
          PrettyButton {
            text: qsTr("Try again")
            onClicked: init(scopeRootId, scopeTitle)
          }
          PrettyButton {
            text: qsTr("Back to library")
            onClicked: popStack()
          }
        }
      }
    }

    Item { Layout.fillHeight: true }
  }

  MediaPlayer {
    id: pronunciation
    audioOutput: AudioOutput {}
  }

  Dictionary {
    id: dictionary
  }

  Connections {
    target: dictionary

    function onDefinitionsReady(definitions) {
      var text = ""
      for (let i = 0; i < definitions.length && text.length === 0; i++) {
        const translations = definitions[i].translations
        for (let j = 0; j < translations.length; j++) {
          if (translations[j].text.length > 0) {
            text = translations[j].text
            break
          }
        }
      }
      finishLookup(text)
    }

    function onErrorOccured(error) {
      finishLookup("")
    }
  }

  function stripHtml(text) {
    return text.replace(/<[^>]*>/g, "").replace(/&nbsp;/g, " ").trim()
  }

  function firstLine(text) {
    const plain = stripHtml(text)
    const idx = plain.indexOf("\n")
    return idx >= 0 ? plain.substring(0, idx).trim() : plain
  }

  function answerFromMeaning(meaning) {
    if (!meaning || meaning.length === 0) {
      return ""
    }
    return firstLine(meaning)
  }

  function shuffle(array) {
    const copy = array.slice()
    for (let i = copy.length - 1; i > 0; i--) {
      const j = Math.floor(Math.random() * (i + 1))
      const tmp = copy[i]
      copy[i] = copy[j]
      copy[j] = tmp
    }
    return copy
  }

  function cachedAnswer(word) {
    const text = answerCache[word.itemId]
    return text !== undefined ? text : ""
  }

  function translationPool() {
    const answers = []
    const seen = {}
    scopeWords.forEach((word) => {
      const text = cachedAnswer(word)
      if (text.length > 0 && !seen[text]) {
        seen[text] = true
        answers.push(text)
      }
    })
    return answers
  }

  function pickDistractors(correctText, pool, count) {
    const candidates = pool.filter((text) => text.length > 0 && text !== correctText)
    return shuffle(candidates).slice(0, count)
  }

  function enqueueLookup(word, callback) {
    lookupQueue.push({ word: word, callback: callback })
    processLookupQueue()
  }

  function processLookupQueue() {
    if (lookupBusy || lookupQueue.length === 0) {
      return
    }

    const item = lookupQueue.shift()
    const word = item.word

    if (answerCache[word.itemId] !== undefined) {
      item.callback(answerCache[word.itemId])
      processLookupQueue()
      return
    }

    const fromMeaning = answerFromMeaning(word.meaning)
    if (fromMeaning.length > 0) {
      answerCache[word.itemId] = fromMeaning
      item.callback(fromMeaning)
      processLookupQueue()
      return
    }

    lookupBusy = true
    pendingResolve = (text) => {
      answerCache[word.itemId] = text
      item.callback(text)
      lookupBusy = false
      pendingResolve = null
      processLookupQueue()
    }
    dictionary.get(word.title)
  }

  function finishLookup(text) {
    if (!pendingResolve) {
      return
    }
    pendingResolve(text)
  }

  function updateLoadProgress() {
    loadProgress = loadTotal > 0 ? Math.round(100 * loadDone / loadTotal) : 0
  }

  function preloadTranslations(words, callback) {
    if (words.length === 0) {
      callback()
      return
    }

    loadTotal = words.length
    loadDone = 0
    updateLoadProgress()

    let remaining = words.length
    words.forEach((word) => {
      enqueueLookup(word, () => {
        loadDone++
        updateLoadProgress()
        remaining--
        if (remaining === 0) {
          callback()
        }
      })
    })
  }

  function showQuestion() {
    selectedOption = -1
    currentOptions = []

    if (currentIndex >= questions.length) {
      phase = "done"
      return
    }

    const word = questions[currentIndex]
    const answer = cachedAnswer(word)
    if (answer.length === 0) {
      currentIndex++
      showQuestion()
      return
    }

    correctAnswer = answer
    const pool = translationPool()
    const distractors = pickDistractors(answer, pool, 3)
    if (distractors.length < 3) {
      currentIndex++
      showQuestion()
      return
    }

    currentOptions = shuffle([answer].concat(distractors))
    phase = "question"
  }

  function selectOption(index) {
    if (phase !== "question") {
      return
    }
    selectedOption = index
    if (currentOptions[index] === correctAnswer) {
      score++
    } else {
      wrongCount++
    }
    phase = "feedback"
    feedbackTimer.restart()
  }

  Timer {
    id: feedbackTimer
    interval: 1200
    onTriggered: {
      currentIndex++
      showQuestion()
    }
  }

  function init(rootId, title) {
    scopeRootId = rootId
    scopeTitle = title
    currentIndex = 0
    score = 0
    wrongCount = 0
    loadTotal = 0
    loadDone = 0
    loadProgress = 0
    selectedOption = -1
    answerCache = {}
    lookupQueue = []
    lookupBusy = false
    pendingResolve = null
    currentOptions = []
    phase = "loading"
    errorMessage = ""

    scopeWords = library.wordsInScope(scopeRootId)
    if (scopeWords.length < 4) {
      phase = "error"
      errorMessage = qsTr("Need at least 4 words in this scope to start a quiz.")
      return
    }

    preloadTranslations(scopeWords, () => {
      const pool = translationPool()
      if (pool.length < 4) {
        phase = "error"
        errorMessage = qsTr("Need at least 4 words with translations in this scope.")
        return
      }

      const validWords = scopeWords.filter((word) => cachedAnswer(word).length > 0)
      questions = shuffle(validWords).slice(0, Math.min(10, validWords.length))
      showQuestion()
    })
  }
}
