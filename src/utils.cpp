#include "utils.hpp"

JSCode removeElementByID() {
  return {"                                        \
    function removeElementByID(id, index, array) { \
      var element = document.getElementById(id);   \
      element.parentNode.removeChild(element);     \
    }                                              \
  "};
}

JSCode removeElementByClassName() {
  return {"                                                   \
    function removeElementByClassName(name, index, array) {   \
      const elements = document.getElementsByClassName(name); \
      while (elements.length > 0) {                           \
        elements[0].parentNode.removeChild(elements[0]);      \
      }                                                       \
    }                                                         \
  "};
}

