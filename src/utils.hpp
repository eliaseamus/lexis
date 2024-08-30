#pragma once

#include <QString>

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x 

struct JSCode {
  QString code;
};

JSCode removeElementByID();
JSCode removeElementByClassName();

