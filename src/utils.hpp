#pragma once

#include <QUrl>

#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

namespace lexis {

QByteArray readFile(const QUrl& url);

}
