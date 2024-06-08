##############################################################################
# Copyright (C) 2021 by Lorenzo Buzzi (lorenzo@buzzi.pro)                    #
#                                                                            #
# This program is free software: you can redistribute it and/or modify       #
# it under the terms of the GNU General Public License as published by       #
# the Free Software Foundation, either version 3 of the License, or          #
# (at your option) any later version.                                        #
#                                                                            #
# This program is distributed in the hope that it will be useful,            #
# but WITHOUT ANY WARRANTY; without even the implied warranty of             #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               #
# GNU General Public License for more details.                               #
#                                                                            #
# You should have received a copy of the GNU General Public License          #
# along with this program. If not, see <https://www.gnu.org/licenses/>.      #
##############################################################################

# Created by and for Qt Creator.
# This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

# Main project file
#include(CMakePrintHelpers)

cmake_minimum_required(VERSION 3.20)
project(LBChronoRace
    VERSION 2024
    DESCRIPTION "Timings and Rankings for Races"
    HOMEPAGE_URL "http://github.com/flinco/LBChronoRace"
    LANGUAGES CXX)
set(PROJECT_UPDATE 0)
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)

add_compile_definitions(LBCHRONORACE_NAME=\"${PROJECT_NAME}\")
if(${PROJECT_UPDATE} EQUAL 0)
    add_compile_definitions(LBCHRONORACE_VERSION=\"${PROJECT_VERSION}\")
else()
    add_compile_definitions(LBCHRONORACE_VERSION=\"${PROJECT_VERSION}\ update\ ${PROJECT_UPDATE}\")
endif()


set(PROJECT_SOURCES
    main.cpp
    categoriesmodel.cpp
    categoriesmodel.h
    category.cpp
    category.h
    chronorace.ui
    chronoracedata.cpp
    chronoracedata.h
    chronoracedata.ui
    chronoracelogo.cpp
    chronoracelogo.h
    chronoracetable.cpp
    chronoracetable.h
    chronoracetable.ui
    chronoracetimings.cpp
    chronoracetimings.h
    chronoracetimings.ui
    classentry.cpp
    classentry.h
    competitor.cpp
    competitor.h
    crloader.cpp
    crloader.h
    crtablemodel.h
    csvrankingprinter.cpp
    csvrankingprinter.h
    lbchronorace.cpp
    lbchronorace.h
    lbcrexception.cpp
    lbcrexception.h
    pdfrankingprinter.cpp
    pdfrankingprinter.h
    rankingsbuilder.cpp
    rankingsbuilder.h
    rankingprinter.cpp
    rankingprinter.h
    rankingswizard.cpp
    rankingswizard.h
    rankingswizardformat.cpp
    rankingswizardformat.h
    rankingswizardmode.cpp
    rankingswizardmode.h
    rankingswizardselection.cpp
    rankingswizardselection.h
    startlistmodel.cpp
    startlistmodel.h
    teamclassentry.cpp
    teamclassentry.h
    teamslistmodel.cpp
    teamslistmodel.h
    timing.cpp
    timing.h
    timingsmodel.cpp
    timingsmodel.h
    txtrankingprinter.h
    txtrankingprinter.cpp)

# Translations
set(TS_FILES
    translations/${CMAKE_PROJECT_NAME}_en.ts
    translations/${CMAKE_PROJECT_NAME}_it.ts)

# Fonts (as big resources)
set(FONT_FILES
    fonts/LiberationMono-Regular.ttf
    fonts/LiberationSans-Italic.ttf
    fonts/LiberationMono-Bold.ttf
    fonts/LiberationMono-Italic.ttf
    fonts/LiberationMono-BoldItalic.ttf
    fonts/LiberationSans-Bold.ttf
    fonts/LiberationSerif-Bold.ttf
    fonts/LiberationSerif-BoldItalic.ttf
    fonts/LiberationSans-Regular.ttf
    fonts/LiberationSerif-Regular.ttf
    fonts/LiberationSerif-Italic.ttf
    fonts/LiberationSans-BoldItalic.ttf)
qt_add_big_resources(PROJECT_FONTS fonts-mono.qrc fonts-sans.qrc fonts-serif.qrc)

# Have fonts-mono.qrc, fonts-sans.qrc, and fonts-serif.qrc treated as a source file by Qt Creator
list(APPEND PROJECT_SOURCES fonts-mono.qrc fonts-sans.qrc fonts-serif.qrc)
set_property(SOURCE fonts-mono.qrc PROPERTY SKIP_AUTORCC ON)
set_property(SOURCE fonts-sans.qrc PROPERTY SKIP_AUTORCC ON)
set_property(SOURCE fonts-serif.qrc PROPERTY SKIP_AUTORCC ON)

# Icons (as big resource)
set(ICON_FILES
    icons/LBChronoRace.ico
    icons/LBChronoRaceCRD.ico)
qt_add_big_resources(PROJECT_ICONS icons.qrc)

# Have icons.qrc treated as a source file by Qt Creator
list(APPEND PROJECT_SOURCES icons.qrc)
set_property(SOURCE icons.qrc PROPERTY SKIP_AUTORCC ON)

# Google Material Icons (as standard resource)
list(APPEND PROJECT_SOURCES materialicons.qrc)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/icons/LBChronoRace.rc")
    qt_add_executable(${CMAKE_PROJECT_NAME} WIN32
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
        ${TS_FILES}
        ${FONT_FILES}
        ${PROJECT_FONTS}
        ${ICON_FILES}
        ${PROJECT_ICONS})

    # Create the installer tree
    add_subdirectory(installer)
else()
    qt_add_executable(${CMAKE_PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${TS_FILES}
        ${FONT_FILES}
        ${PROJECT_FONTS}
        ${ICON_FILES}
        ${PROJECT_ICONS})
endif()

# Translations
qt_add_lupdate(${CMAKE_PROJECT_NAME} TS_FILES ${TS_FILES}
    SOURCES ${PROJECT_SOURCES})
qt_add_lrelease(${CMAKE_PROJECT_NAME}
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES)
qt_add_resources(${CMAKE_PROJECT_NAME} "translations"
    PREFIX "/i18n"
    BASE ${CMAKE_BINARY_DIR}
    FILES ${QM_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
