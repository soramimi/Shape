#-------------------------------------------------
#
# Project created by QtCreator 2018-06-11T17:56:23
#
#-------------------------------------------------

QT       += core gui widgets xml

TARGET = Shape
TEMPLATE = app
CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

win32:DEFINES += _USE_MATH_DEFINES

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    ShapeView.cpp \
    Shape.cpp

HEADERS += \
        MainWindow.h \
    ShapeView.h \
    Shape.h

FORMS += \
        MainWindow.ui
