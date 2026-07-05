function getFgColor(bgColor) {
  var colorSum = bgColor.r + bgColor.g + bgColor.b
  var threshold = 2 * 0.93
  if (colorSum > threshold) {
    return "black"
  } else {
    return "white"
  }
}

function frequencyTierLabel(tier) {
  switch (tier) {
  case "core":
    return qsTr("Core")
  case "common":
    return qsTr("Common")
  case "intermediate":
    return qsTr("Intermediate")
  case "advanced":
    return qsTr("Advanced")
  case "rare":
    return qsTr("Rare")
  default:
    return ""
  }
}

function frequencyTierColor(tier) {
  switch (tier) {
  case "core":
    return "#2e7d32"
  case "common":
    return "#1565c0"
  case "intermediate":
    return "#ef6c00"
  case "advanced":
    return "#6a1b9a"
  case "rare":
    return "#616161"
  default:
    return "#616161"
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

