**Lexis** is entirely implemented using **Qt** framework and built using **CMake** build system.

On a high level, it comprises of:
- QML-written GUI
- `Library` storing user's words collection in **SQLite** database
- several classes providing access to web services: `Dictionary`, `Predictor` and `Pronunciation`
- `Settings`: languages being learned, interface language, user preferences

One can think of Lexis as of a web app with QML frontend and C++ backend.

<p align="center">
  <img alt="Component diagram" src="docs/uml/diagrams/component.png">
</p>

## Graphic interface
Graphic interface is written in **QML** markup language and Qt's dialect of **JavaScript** language.
Some of C++ classes are also incorporated into QML code and used directly due to [Qt's ability](https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html) to integrate C++ code into QML.

Another way for C++ and QML parts to communicate is one of the most well-known Qt's mechanisms - [signals and slots](https://doc.qt.io/qt-6/signalsandslots.html).

JavaScript is used in QML modules to implement functions operating on data. It is a sort of glue connecting GUI elements together.
You can see a QML GUI classes diagram below. Some of GUI elements are depicted in `docs/images` directory.

<p align="center">
  <img alt="GUI class diagram" src="docs/uml/diagrams/gui_class.png">
</p>

`MainWindow` contains a `StackView` to display pages and a `SideBar` on the left side.

`LibraryView` is the default page of the StackView and probably the main page of the app.
It displays user's collection and used to navigate between levels of the collection. It also has a `ToolBar` to search for existing elements and add new ones.
The collection is splitted into sections by type (words, subject group, etc) and groups of `SectionView` elements serve to visualize sections.
Each SectionView has a `GridView` to display a group of `SectionItem` elements, representing the smallest unit of the collection.

`ItemView` displays the dictionary note on a chosen word, pronunciation button, transcription and an icon. In case dictionary service doesn't provide any information on a word of interest,
the user may add their own notes using `insertMeaning` button. 

When an item is being added or modified, a `LibraryItemConfiguration` page is being pushed onto the stack, and a `ImagePicker` page containing `WebView` used to select a picture for the item.

There is also a bunch of modal dialog windows for various purposes: from configuring menu settings (`InterfaceLanguageDialog`, `SortRoleDialog`, `HelpDialog`, etc) to
performing operations on items (`DeleteDialog`, `MoveDialog`, etc).

## Data model
Library is responsible for maintaining user's words collection: storing it into SQLite database as well as exposing the collection into QML code.

<p align="center">
  <img alt="Data model class diagram" src="docs/uml/diagrams/data_model_class.png">
</p>

