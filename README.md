<p align="center">
 <a href="https://www.qt.io/download-open-source">
   <img alt="Framework" src="https://img.shields.io/badge/Framework-Qt-green?style=for-the-badge">
 </a>
 <a href="https://cmake.org/">
   <img alt="Build system" src="https://img.shields.io/badge/Build_system-CMake-red?style=for-the-badge">
 </a>
 <a href="https://en.cppreference.com/w/cpp/23">
   <img alt="CPP standard" src="https://img.shields.io/badge/C%2B%2B_standard-23-lightblue?style=for-the-badge">
 </a>
 <a href="https://github.com/eliaseamus/lexis/blob/main/LICENSE">
   <img alt="License" src="https://img.shields.io/badge/License-GPL--3.0-blue?style=for-the-badge">
 </a>
</p>

<p align="center">
  <img alt="Logo" src="/icons/Lexis.png" width="256" height="256">
</p>

**Lexis** is a vocabulary-builder desktop app, which helps you to organize a library of words to learn.
Using **Lexis**, you can create your own collections of words, grouping them as you wish.

## Example of use
Lets pretend you're reading some book and encounter an unknown word *ingenious*.
You'd like to learn it, but instead of writing it down to some piece of paper or a note-taking app, you can add it to your word
collection in **Lexis**. This way you'll get a dictionary definition of the word with a picture associated with
its meaning, plus the transcription with an audiable pronunciation - pretty much handy, isn't it?
Using Lexis, you can combine words into groups by subject, create an entry in your collection for any source you learn the new word from:
it can be a book or a song, or something else.

1. Since the word *ingenious* may be used to describe a person, you can add it to the *Personality* subject group.

<p align="center">
  <img alt="Collection" src="docs/images/library_view.png">
</p>

2. Having opened the group, you can create a new item by pressing a *New item* button at the right bottom corner and type
the desired word in the language you're learning:

<p align="center">
  <img alt="New word" src="docs/images/predictor.png">
</p>

3. Pick an image best associated with the meaning of this word and drag it to the specified area. In case search results don't
reflect the meaning of the word too good, you can type a different word or phrase in any language in the bottom bar and search again:

<p align="center">
  <img alt="Pick image" src="docs/images/image_picker.png">
</p>

4. Assign a background color to make a new item more distinguishable from the rest of the collection:

<p align="center">
  <img alt="Assign color" src="docs/images/library_item_configuration.png">
</p>

5. You can see a new word added into the collection:

<p align="center">
  <img alt="New word" src="docs/images/section_item.png">
</p>

6. Open it, and voilà!

<p align="center">
  <img alt="Word card" src="docs/images/item_view.png">
</p>

In case the dictionary doesn't provide any meaning for the word or phrase, you may add it yourself into the library.
You may also revise the groups added into the collection by moving them or their contents around the library.

## Supported languages
There are nine languages available to learn using **Lexis**:
- english
- spanish
- german
- french
- russian
- italian
- polish
- ukranian
- turkish

Also, two languages can be used as interface language: english and russian.

## Target platform
So far, build configuration for Linux and Windows platforms has been done. A build for MacOS is also achievable due to Qt's cross-platform support.

## Dependencies
**Lexis** uses several third-party services which you need to get API_KEY to:
- [Yandex predictor](https://yandex.com/dev/predictor/)
- [Yandex dictionary](https://yandex.com/dev/dictionary)
- [Google Programmable Search Engine](https://programmablesearchengine.google.com/about/)
- [Eleven labs](https://elevenlabs.io/)
- [PlayHT](https://play.ht/)

These keys are currently used as compile definitions, which makes them necessary to build an executable program file.
