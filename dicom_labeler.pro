#-------------------------------------------------
#
# Project created by QtCreator 2014-04-05T22:49:01
#
#-------------------------------------------------

QT       += core webkit network

#QT       -= gui

greaterThan(QT_MAJOR_VERSION, 4): {
  QT       +=  webkitwidgets
}

TARGET = dicom_labeler
CONFIG   += qt console
CONFIG   -= app_bundle

contains(CONFIG, static): {
  QTPLUGIN += qjpeg qgif qsvg qmng qico qtiff
  DEFINES  += STATIC_PLUGINS
}

SOURCES += main.cpp \
    dicomlabeler.cpp \
    dicomprocessor.cpp \
    templaterenderer.cpp

HEADERS += \
    dicomlabeler.h \
    dicomprocessor.h \
    templaterenderer.h


LIBS += -ldcmjpls -lCharLS -ldcmjpeg -lijg8 -lijg12 -lijg16  -ldcmimage -ltiff -ldcmimgle -ldcmdata -loflog -lofstd -lz
