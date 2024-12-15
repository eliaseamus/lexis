**Lexis** is entirely implemented using **Qt** framework and built with **CMake** build system.

On a high level, it comprises of:
- QML-written GUI
- `Library` storing user's words collection in **SQLite** database
- several classes providing access to web services: `Dictionary`, `Predictor` and `Pronunciation`
- `Settings`: languages being learned, interface language, user preferences

One can think of Lexis as of a web app with QML frontend and C++ backend.

<p align="center">
  <img alt="Component diagram" src="uml/diagrams/component.png">
</p>

## Graphic interface
Graphic interface is written in **QML** markup language and Qt's dialect of **JavaScript** language.
Some of C++ classes are also incorporated into QML code and used directly due to [Qt support](https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html) for integration of C++ types into QML.

Another way for C++ and QML parts to communicate is one of the most well-known Qt mechanisms - [signals and slots](https://doc.qt.io/qt-6/signalsandslots.html).

JavaScript is used in QML modules to implement functions operating on data. It is a sort of glue connecting GUI elements together.
You can see a QML GUI classes diagram below. Some of GUI elements are depicted in `docs/images` directory.

<p align="center">
  <img alt="GUI class diagram" src="uml/diagrams/gui_class.png">
</p>

`MainWindow` contains a [StackView](https://doc.qt.io/qt-5/qml-qtquick-controls2-stackview.html) to display pages and a `SideBar` on the left side.

`LibraryView` is the default page of the StackView and probably the main page of the app.
It displays user's collection and used to navigate between levels of the collection. It also has a `ToolBar` to search for existing elements and add new ones.
The collection is splitted into sections by type (words, subject group, etc) and groups of `SectionView` elements serve to visualize sections.
Each SectionView has a [GridView](https://doc.qt.io/qt-6/qml-qtquick-gridview.html) to display `SectionItems`, representing the smallest unit of the collection.

`ItemView` displays the dictionary definitions of a chosen word, pronunciation button, transcription and an icon. In case dictionary service doesn't provide any information on a word of interest,
the user may add their own notes on it using `insertMeaning` button.

<p align="center">
  <img alt="Open word card" src="uml/diagrams/open_item.png">
</p>

When an item is being added or modified, a `LibraryItemConfiguration` page is pushed onto the stack, and a `ImagePicker` page is used to select a picture for the item.
ImagePicker showcases pictures to choose from via a [WebView](https://doc.qt.io/qt-6/qml-qtwebview-webview.html) displaying search results of the [Google Programmable Search Engine](https://programmablesearchengine.google.com/about/).

There is also a bunch of modal dialog windows for various purposes: from configuring menu settings (`InterfaceLanguageDialog`, `SortRoleDialog`, `HelpDialog`, etc) to
performing operations on items (`DeleteDialog`, `MoveDialog`, etc).

## Data model
Library is responsible for maintaining user's words collection: storing it into SQLite database as well as exposing the collection into QML code.

<p align="center">
  <img alt="Data model class diagram" src="uml/diagrams/data_model_class.png">
</p>

Library performs all operations on items that involve storage access, including: read, update, delete, move.
Since library items may have children, which may also have children in their turn, etc, a tree structure is used for storing collection data.
Thus, the root table for the data inherits its name from the language it is dedicated to: for instance, `en` - for English, `es` - for Spanish, and so on.
All the successor elements' tables obtain their name by concatenating their parent's name and their index within the parent table (like `es_3_11`).
When a group of elements is being deleted, dropping for its table together with all its child tables follows.

When providing the collection data to QML, it is sorted into groups of `LibrarySection` by type. To enable sorting and filtering for the sections, a `LibraryItemProxyModel`
extending [QSortFilterProxyModel](https://doc.qt.io/qt-6/qsortfilterproxymodel.html) was added. It serves as a wrapper for the actual `LibraryItemModel` possessing the collection of `LibraryItems`.

One more chore of the Library is to request audio from `Pronunciation` service when a new word is added into the collection, or the existing word's title is changed.
Audio fetch is an asynchronous operation, since it may take some time, and we don't want to freeze GUI during that. Library stores the last requested audio file and provides it to `ItemView` page on demand: when the audio file was not uploaded yet at the moment of LibraryItem's creation.

<p align="center">
  <img alt="Play pronunciation" src="uml/diagrams/play_pronunciation.png">
</p>

## Web services
Lexis actively fetches data from the internet using classes implementing `WebService` interface. This interface utilizes [QNetworkAccessManager](https://doc.qt.io/qt-6/qnetworkaccessmanager.html) to make REST API requests. The derived classes override `onFinished` slot method to handle responses.

<p align="center">
  <img alt="Web classes" src="uml/diagrams/web_class.png">
</p>

`Predictor` is the first of such classes, and it is used for text autocomplete. When a user adds a new item into the collection and types the word into the text field, Predictor
provides a list of suggestions ("predictions") displayed below the text field. An actual web service API generating predictions list is [Yandex Predictor API](https://yandex.com/dev/predictor/).

<p align="center">
  <img alt="New item" src="uml/diagrams/new_item.png">
</p>

`Dictionary` provides definitions of the word by the means of [Yandex Dictionary API](https://yandex.com/dev/dictionary) when a user opens the `ItemView` page. This service utilizes
`DictionaryCache`, which implements the Least Recently Used (LRU) principle to reduce the number of REST API requests. Dictionary also takes into account current interface language,
which means that a change of the interface language will cause a cache miss when access the item that was added to the cache before the language change.

Presently, there are two text-to-speech services that implement `TTSService` interface: `ElevenLabs` and `PlayHT`. They match with corresponding projects: [Eleven labs](https://elevenlabs.io/)
and [Play HT](https://play.ht/). They are called alternately by `Pronunciation` class when a request to generate a pronunciation audio file is being processed to distribute the number of
requests over services equally.

## Settings
Lexis settings are manipulated with the help of the respective classes on both [QML](https://doc.qt.io/qt-6/qml-qt-labs-settings-settings.html) and [C++](https://doc.qt.io/qt-6/qsettings.html) 
sides. These classes provide Lexis with persistent platform-independent application settings.
