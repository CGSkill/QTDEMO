#-------------------------------------------------
#
# Project created by QtCreator 2016-03-21T11:54:02
#
#-------------------------------------------------

QT       += core gui \
                serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SerialTest_fc
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    serial_data_thread.cpp

HEADERS  += widget.h \
    serial_data_thread.h

FORMS    += widget.ui

DISTFILES +=
