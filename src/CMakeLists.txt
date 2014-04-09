INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}
${CMAKE_SOURCE_DIR}/scribus
)

SET(SCRIBUS_EPUBEXPORT_PLUGIN_UI_SRC
  epubexportdialog.ui
)

SET(SCRIBUS_EPUBEXPORT_PLUGIN_MOC_CLASSES
  epubexportplugin.h
  epubexportdialog.h
  epubexport.h
  module/epubexportEpub.h
  module/epubexportImage.h
  module/epubexportStructure.h
  module/epubexportScribusDoc.h
  module/epubexportCssParser.h
  module/epubexportStyle.h
  module/epubexportText.h
  module/epubexportXhtml.h
)

SET(SCRIBUS_EPUBEXPORT_PLUGIN_SOURCES
  epubexportplugin.cpp
  epubexportdialog.cpp
  epubexport.cpp
  module/epubexportEpub.cpp
  module/epubexportImage.cpp
  module/epubexportStructure.cpp
  module/epubexportScribusDoc.cpp
  module/epubexportCssParser.cpp
  module/epubexportStyle.cpp
  module/epubexportText.cpp
  module/epubexportXhtml.cpp
)

SET(SCRIBUS_EPUBEXPORT_PLUGIN "epubexportplugin")

QT4_WRAP_UI(SCRIBUS_EPUBEXPORT_PLUGIN_UI_SOURCES ${SCRIBUS_EPUBEXPORT_PLUGIN_UI_SRC} )
QT4_WRAP_CPP(SCRIBUS_EPUBEXPORT_PLUGIN_MOC_SOURCES ${SCRIBUS_EPUBEXPORT_PLUGIN_MOC_CLASSES})

ADD_LIBRARY(${SCRIBUS_EPUBEXPORT_PLUGIN} MODULE ${SCRIBUS_EPUBEXPORT_PLUGIN_SOURCES} ${SCRIBUS_EPUBEXPORT_PLUGIN_MOC_SOURCES} ${SCRIBUS_EPUBEXPORT_PLUGIN_UI_CPP} ${SCRIBUS_EPUBEXPORT_PLUGIN_UI_SOURCES})

TARGET_LINK_LIBRARIES(${SCRIBUS_EPUBEXPORT_PLUGIN} ${PLUGIN_LIBRARIES})

INSTALL(TARGETS ${SCRIBUS_EPUBEXPORT_PLUGIN}
  LIBRARY
  DESTINATION ${PLUGINDIR}
  PERMISSIONS ${PLUGIN_PERMISSIONS}
)

ADD_DEPENDENCIES(${SCRIBUS_EPUBEXPORT_PLUGIN} ${EXE_NAME})
