#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T20:33:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LBChronoRace
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

RC_ICONS = LBChronoRace.ico

SOURCES += \
    chronoracedata.cpp \
    main.cpp \
    timing.cpp \
    classentry.cpp \
    competitor.cpp \
    category.cpp \
    teamclassentry.cpp \
    chronoracetable.cpp \
    lbchronorace.cpp \
    categoriesmodel.cpp \
    lbcrexception.cpp \
    startlistmodel.cpp \
    teamslistmodel.cpp \
    timingsmodel.cpp \
    crloader.cpp

HEADERS += \
    chronoracedata.h \
    timing.h \
    classentry.h \
    competitor.h \
    category.h \
    teamclassentry.h \
    chronoracetable.h \
    lbchronorace.h \
    categoriesmodel.h \
    lbcrexception.h \
    startlistmodel.h \
    teamslistmodel.h \
    timingsmodel.h \
    crloader.h

FORMS += \
    chronorace.ui \
    chronoracedata.ui \
    chronoracetable.ui

TRANSLATIONS += \
    LBChronoRace_it.ts

RESOURCES += \
    fonts.qrc \
    translations.qrc

DISTFILES += \
    LICENSE \
    README.md \
    samples/README \
    samples/mass_start/latin1/categories.csv \
    samples/mass_start/latin1/startlist.csv \
    samples/mass_start/latin1/timings.csv \
    samples/mass_start/mass_start.crd \
    samples/mass_start/utf8/categories.csv \
    samples/mass_start/utf8/startlist.csv \
    samples/mass_start/utf8/timings.csv \
    samples/relay/latin1/categories.csv \
    samples/relay/latin1/startlist.csv \
    samples/relay/latin1/timings.csv \
    samples/relay/relay.crd
