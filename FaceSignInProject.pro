#-------------------------------------------------
#
# Project created by QtCreator 2020-05-29T10:01:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FaceSignInProject
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
INCLUDEPATH += D:/opencv/opencv3.4-qt-install/install/include
INCLUDEPATH += D:/opencv/opencv3.4-qt-install/install/include/opencv
INCLUDEPATH += D:/opencv/opencv3.4-qt-install/install/include/opencv2
LIBS += D:/opencv/opencv3.4-qt-install/install/x86/mingw/lib/libopencv_*.a
