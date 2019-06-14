#-------------------------------------------------
#
# Project created by QtCreator 2019-04-17T14:37:46
#
#-------------------------------------------------

QT       += core gui
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Touring_simulator
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../src/qt_headers

SOURCES += \
    headers/city_graph.cc \
    headers/id_map.cc \
    headers/io.cc \
    headers/log.cc \
    headers/path.cc \
    headers/save_at_exit.cc \
    headers/time_format.cc \
    headers/traveller.cc \
    main.cpp \
    qt_headers/widget.cpp \
    qt_headers/mylabel.cpp \
    qt_headers/mymap.cpp \
    qt_headers/simulator.cpp

HEADERS += \
    qt_headers/time_table_widget_item.h \
    qt_headers/widget.h \
    headers/city_graph.h \
    headers/id_map.h \
    headers/io.h \
    headers/log.h \
    headers/path.h \
    headers/save_at_exit.h \
    headers/time_format.h \
    headers/traveller.h \
    headers/user_type.h \
    qt_headers/mylabel.h \
    qt_headers/simulator.h \
    qt_headers/mymap.h

FORMS += \
    qt_headers/widget.ui

DISTFILES += \
    ../data/car_extract_with_id.txt \
    ../data/city_id.txt \
    ../data/flight_extract_with_id.txt \
    ../data/train_extract_with_id.txt \
    ../data/train_seat_type_id.txt \
    ../data/transport_type_id.txt

RESOURCES += \
    material/material.qrc
