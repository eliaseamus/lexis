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
  property var translationListsCache: ({})
  property int currentIndex: 0
  property int score: 0
  property int wrongCount: 0
  property int loadTotal: 0
  property int loadDone: 0
  property int loadProgress: 0
  property int questionsPresented: 0
  property string phase: "loading"
  property string errorMessage: ""
  property var currentOptions: []
  property string correctAnswer: ""
  property string currentPromptText: ""
  property bool currentQuestionReverse: false
  property int selectedOption: -1
  property var pendingResolve: null
  property var lookupQueue: []
  property bool lookupBusy: false
  property bool playWhenReady: false
  property bool audioLoading: false
  property int audioRetryCount: 0
  readonly property int maxAudioRetries: 3

  property var currentWord: currentIndex >= 0 && currentIndex < questions.length
                            ? questions[currentIndex].word : null
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
        title: qsTr("Loading question…")
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

      PrettyLabel {
        visible: phase === "error"
        Layout.alignment: Qt.AlignHCenter
        title: qsTr("Quiz unavailable")
      }

      Label {
        visible: phase === "error"
        Layout.alignment: Qt.AlignHCenter
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
        color: settings.fgColor
        font.pointSize: 16
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
            font.pointSize: currentQuestionReverse ? 28 : 36
            text: currentQuestionReverse ? currentPromptText : (currentWord ? currentWord.title : "")
          }

          RowLayout {
            Layout.alignment: Qt.AlignHCenter
            visible: !currentQuestionReverse && currentWord !== null
            BusyIndicator {
              visible: audioLoading
              Layout.preferredWidth: 32
              Layout.preferredHeight: 32
            }
            RoundButton {
              visible: !audioLoading
              icon.source: "qrc:/qt/qml/QLexis/icons/audio.png"
              icon.color: cardFgColor
              Material.background: cardColor
              enabled: currentWord !== null
              onClicked: playPronunciation()
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
            text: currentQuestionReverse ? qsTr("Choose the word:") : qsTr("Choose the translation:")
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

    onErrorOccurred: (error, errorString) => {
      if (playWhenReady) {
        retryPronunciation()
      }
    }

    onPlaybackStateChanged: {
      if (playbackState === MediaPlayer.PlayingState) {
        audioRetryCount = 0
        playbackCheckTimer.stop()
      }
    }
  }

  Timer {
    id: playbackCheckTimer
    interval: 1000
    onTriggered: {
      if (!playWhenReady || !currentWord) {
        return
      }
      if (pronunciation.playbackState !== MediaPlayer.PlayingState && pronunciation.position === 0) {
        retryPronunciation()
      }
    }
  }

  Connections {
    target: library

    function onAudioReady(itemId, url) {
      if (!currentWord || itemId !== currentWord.itemId) {
        return
      }
      audioLoading = false
      if (url.toString().length === 0) {
        if (playWhenReady) {
          retryPronunciation()
        }
        return
      }
      pronunciation.source = url
      if (!playWhenReady) {
        return
      }
      pronunciation.play()
      playbackCheckTimer.restart()
    }
  }

  Dictionary {
    id: dictionary
  }

  Connections {
    target: dictionary

    function onDefinitionsReady(definitions) {
      finishLookup(translationsFromDefinitions(definitions))
    }

    function onErrorOccured(error) {
      finishLookup([])
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

  function parseCachedTranslations(text) {
    if (!text || text.length === 0) {
      return []
    }
    const seen = {}
    const result = []
    const lines = text.split("\n")
    for (let i = 0; i < lines.length; i++) {
      const line = lines[i].trim()
      if (line.length > 0 && !seen[line]) {
        seen[line] = true
        result.push(line)
      }
    }
    return result
  }

  function joinCachedTranslations(translations) {
    return translations.join("\n")
  }

  function translationsFromDefinitions(definitions) {
    const seen = {}
    const result = []
    for (let i = 0; i < definitions.length; i++) {
      const translations = definitions[i].translations
      for (let j = 0; j < translations.length; j++) {
        const text = stripHtml(translations[j].text).trim()
        if (text.length > 0 && !seen[text]) {
          seen[text] = true
          result.push(text)
        }
      }
    }
    return result
  }

  function translationsFromMeaning(meaning) {
    if (!meaning || meaning.length === 0) {
      return []
    }
    const line = firstLine(meaning)
    return line.length > 0 ? [line] : []
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

  function formatOptionText(translations) {
    if (!translations || translations.length === 0) {
      return ""
    }
    return translations.slice(0, 5).join(", ")
  }

  function storeTranslations(word, translations) {
    translationListsCache[word.itemId] = translations
    if (translations.length > 0) {
      const joined = joinCachedTranslations(translations)
      library.updateCachedTranslation(word.itemId, joined)
      word.cachedTranslation = joined
    }
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

    if (translationListsCache[word.itemId] !== undefined) {
      item.callback(translationListsCache[word.itemId])
      processLookupQueue()
      return
    }

    const fromMeaning = translationsFromMeaning(word.meaning)
    if (fromMeaning.length > 0) {
      translationListsCache[word.itemId] = fromMeaning
      item.callback(fromMeaning)
      processLookupQueue()
      return
    }

    const fromCachedTranslation = parseCachedTranslations(word.cachedTranslation)
    if (fromCachedTranslation.length > 0) {
      translationListsCache[word.itemId] = fromCachedTranslation
      item.callback(fromCachedTranslation)
      processLookupQueue()
      return
    }

    lookupBusy = true
    pendingResolve = (translations) => {
      storeTranslations(word, translations)
      item.callback(translations)
      lookupBusy = false
      pendingResolve = null
      processLookupQueue()
    }
    dictionary.get(word.title)
  }

  function finishLookup(translations) {
    if (!pendingResolve) {
      return
    }
    pendingResolve(translations)
  }

  function updateLoadProgress() {
    loadProgress = loadTotal > 0 ? Math.round(100 * loadDone / loadTotal) : 0
  }

  function resolveWords(words, callback) {
    if (words.length === 0) {
      callback({})
      return
    }

    loadTotal = words.length
    loadDone = 0
    updateLoadProgress()

    const answers = {}
    let remaining = words.length
    words.forEach((word) => {
      enqueueLookup(word, (translations) => {
        answers[word.itemId] = translations
        loadDone++
        updateLoadProgress()
        remaining--
        if (remaining === 0) {
          callback(answers)
        }
      })
    })
  }

  function pickDistractorWords(word, count) {
    const others = scopeWords.filter((entry) => entry.itemId !== word.itemId)
    return shuffle(others).slice(0, count)
  }

  function uniqueOptionCount(options) {
    const seen = {}
    for (let i = 0; i < options.length; i++) {
      seen[options[i]] = true
    }
    return Object.keys(seen).length
  }

  function buildQuestionOptions(wordTranslations, distractorTranslations, targetOptionCount) {
    const correct = formatOptionText(wordTranslations)
    const options = [correct]
    for (let i = 0; i < distractorTranslations.length && options.length < targetOptionCount; i++) {
      const text = formatOptionText(distractorTranslations[i])
      if (text.length > 0) {
        options.push(text)
      }
    }
    return { correctAnswer: correct, options: options }
  }

  function prepareForwardQuestionOptions(word, attempt) {
    if (attempt > 10) {
      currentIndex++
      showQuestion()
      return
    }

    const targetOptionCount = Math.min(4, scopeWords.length)
    const distractorCount = targetOptionCount - 1
    const distractorWords = pickDistractorWords(word, distractorCount)
    if (distractorWords.length < distractorCount) {
      currentIndex++
      showQuestion()
      return
    }

    const wordsNeeded = [word].concat(distractorWords)
    resolveWords(wordsNeeded, (answers) => {
      const wordTranslations = answers[word.itemId] || []
      if (wordTranslations.length === 0) {
        prepareForwardQuestionOptions(word, attempt + 1)
        return
      }

      const distractorTranslations = distractorWords.map((entry) => answers[entry.itemId] || [])
      const built = buildQuestionOptions(wordTranslations, distractorTranslations, targetOptionCount)
      if (built.options.length < targetOptionCount || uniqueOptionCount(built.options) < targetOptionCount) {
        prepareForwardQuestionOptions(word, attempt + 1)
        return
      }

      currentQuestionReverse = false
      currentPromptText = ""
      correctAnswer = built.correctAnswer
      currentOptions = shuffle(built.options)
      questionsPresented++
      phase = "question"
      prefetchAudio()
    })
  }

  function prepareReverseQuestionOptions(word, attempt) {
    if (attempt > 10) {
      currentIndex++
      showQuestion()
      return
    }

    const targetOptionCount = Math.min(4, scopeWords.length)
    const distractorCount = targetOptionCount - 1
    const distractorWords = pickDistractorWords(word, distractorCount)
    if (distractorWords.length < distractorCount) {
      currentIndex++
      showQuestion()
      return
    }

    resolveWords([word], (answers) => {
      const wordTranslations = answers[word.itemId] || []
      if (wordTranslations.length === 0) {
        prepareReverseQuestionOptions(word, attempt + 1)
        return
      }

      const options = [word.title]
      for (let i = 0; i < distractorWords.length; i++) {
        options.push(distractorWords[i].title)
      }
      if (options.length < targetOptionCount || uniqueOptionCount(options) < targetOptionCount) {
        prepareReverseQuestionOptions(word, attempt + 1)
        return
      }

      currentQuestionReverse = true
      currentPromptText = formatOptionText(wordTranslations)
      correctAnswer = word.title
      currentOptions = shuffle(options)
      questionsPresented++
      phase = "question"
    })
  }

  function prefetchAudio() {
    if (!currentWord || currentQuestionReverse) {
      audioLoading = false
      return
    }

    const cachedUrl = pronunciation.source.toString()
    if (cachedUrl.length > 0) {
      audioLoading = false
      return
    }

    const url = library.readAudio(currentWord.itemId).toString()
    if (url.length > 0) {
      pronunciation.source = url
      audioLoading = false
    } else {
      audioLoading = true
    }
  }

  function playPronunciation() {
    if (!currentWord) {
      return
    }
    playWhenReady = true
    const cachedUrl = pronunciation.source.toString()
    if (cachedUrl.length > 0) {
      pronunciation.play()
      playbackCheckTimer.restart()
      return
    }

    const url = library.readAudio(currentWord.itemId).toString()
    if (url.length > 0) {
      pronunciation.source = url
      audioLoading = false
      pronunciation.play()
      playbackCheckTimer.restart()
      return
    }

    audioLoading = true
  }

  function retryPronunciation() {
    if (!currentWord || !playWhenReady) {
      return
    }
    if (audioRetryCount >= maxAudioRetries) {
      audioLoading = false
      playWhenReady = false
      return
    }
    audioRetryCount++
    pronunciation.source = ""
    audioLoading = true
    library.refreshAudio(currentWord.itemId)
  }

  function showQuestion() {
    selectedOption = -1
    currentOptions = []
    audioRetryCount = 0
    playWhenReady = false
    audioLoading = false
    playbackCheckTimer.stop()
    pronunciation.stop()
    pronunciation.source = ""

    if (currentIndex >= questions.length) {
      if (questionsPresented === 0) {
        phase = "error"
        errorMessage = qsTr("Could not load enough translations for a quiz in this scope.")
      } else {
        phase = "done"
      }
      return
    }

    phase = "loading"
    currentQuestionReverse = questions[currentIndex].reverse
    if (currentQuestionReverse) {
      prepareReverseQuestionOptions(questions[currentIndex].word, 0)
    } else {
      prepareForwardQuestionOptions(questions[currentIndex].word, 0)
    }
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
    questionsPresented = 0
    loadTotal = 0
    loadDone = 0
    loadProgress = 0
    selectedOption = -1
    translationListsCache = {}
    lookupQueue = []
    lookupBusy = false
    pendingResolve = null
    currentOptions = []
    phase = "loading"
    errorMessage = ""

    scopeWords = library.wordsInScope(scopeRootId)
    if (scopeWords.length < 2) {
      phase = "error"
      errorMessage = scopeWords.length === 0
                   ? qsTr("This scope has no words to quiz.")
                   : qsTr("Need at least two words in this scope to start a quiz.")
      return
    }

    questions = shuffle(scopeWords).slice(0, Math.min(10, scopeWords.length)).map((word) => ({
      word: word,
      reverse: Math.random() >= 0.5
    }))
    showQuestion()
  }
}
