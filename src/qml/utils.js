function getFgColor(bgColor) {
  var colorSum = bgColor.r + bgColor.g + bgColor.b
  var threshold = 2 * 0.93
  if (colorSum > threshold) {
    return "black"
  } else {
    return "white"
  }
}

function getFullLanguageName(language) {
  var res = "";
  if (language === "en") {
    return qsTr("English");
  } else if (language === "es") {
    res = qsTr("Spanish");
  } else if (language === "de") {
    res = qsTr("German");
  } else if (language === "fr") {
    res = qsTr("French");
  } else if (language === "ru") {
    res = qsTr("Russian");
  } else if (language === "it") {
    res = qsTr("Italian");
  } else if (language === "pl") {
    res = qsTr("Polish");
  } else if (language === "uk") {
    res = qsTr("Ukranian");
  } else if (language === "tr") {
    res = qsTr("Turkish");
  }
  return res;
}

