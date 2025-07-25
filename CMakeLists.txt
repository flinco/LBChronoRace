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
    VERSION 2025
    DESCRIPTION "Timings and Rankings for Races"
    HOMEPAGE_URL "http://github.com/flinco/LBChronoRace"
    LANGUAGES CXX)
set(PROJECT_UPDATE 2)
string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)
qt_standard_project_setup(REQUIRES 6.8
    I18N_TRANSLATED_LANGUAGES it)

set(PROJECT_ORGANIZATION "LoBuSoft")
set(PROJECT_VERSION_STRING "${PROJECT_VERSION}u${PROJECT_UPDATE}")
if(${PROJECT_UPDATE} EQUAL 0)
    set(LBCHRONORACE_VERSION_STRING "${PROJECT_VERSION}")
else()
    set(LBCHRONORACE_VERSION_STRING "${PROJECT_VERSION} update ${PROJECT_UPDATE}")
endif()

set(HEADER_FILES
    categoriesmodel.hpp
    category.hpp
    cattypedelegate.hpp
    chronoracedata.hpp
    chronoracelogo.hpp
    chronoracetable.hpp
    chronoracetimings.hpp
    classentry.hpp
    clubdelegate.hpp
    competitor.hpp
    compsexdelegate.hpp
    crhelper.hpp
    crloader.hpp
    crtablemodel.hpp
    csvrankingprinter.hpp
    lbchronorace.hpp
    lbcrexception.hpp
    livetable.hpp
    livetablefilterproxymodel.hpp
    multiselectcombobox.hpp
    pdfrankingprinter.hpp
    ranking.hpp
    rankingcatsdelegate.hpp
    rankingsbuilder.hpp
    rankingprinter.hpp
    rankingsmodel.hpp
    rankingtypedelegate.hpp
    recentraces.hpp
    startlistmodel.hpp
    teamclassentry.hpp
    teamslistmodel.hpp
    timespandialog.hpp
    timing.hpp
    timingsmodel.hpp
    timingstatusdelegate.hpp
    txtrankingprinter.hpp
    wizards/newracewizard.hpp
    wizards/rankingswizard.hpp
    wizards/rankingswizardformat.hpp
    wizards/rankingswizardmode.hpp
    wizards/rankingswizardselection.hpp)

set(SOURCE_FILES
    main.cpp
    categoriesmodel.cpp
    category.cpp
    cattypedelegate.cpp
    chronoracedata.cpp
    chronoracelogo.cpp
    chronoracetable.cpp
    chronoracetimings.cpp
    classentry.cpp
    clubdelegate.cpp
    competitor.cpp
    compsexdelegate.cpp
    crhelper.cpp
    crloader.cpp
    csvrankingprinter.cpp
    lbchronorace.cpp
    lbcrexception.cpp
    livetable.cpp
    livetablefilterproxymodel.cpp
    multiselectcombobox.cpp
    pdfrankingprinter.cpp
    ranking.cpp
    rankingcatsdelegate.cpp
    rankingsbuilder.cpp
    rankingprinter.cpp
    rankingsmodel.cpp
    rankingtypedelegate.cpp
    recentraces.cpp
    startlistmodel.cpp
    teamclassentry.cpp
    teamslistmodel.cpp
    timespandialog.cpp
    timing.cpp
    timingsmodel.cpp
    timingstatusdelegate.cpp
    txtrankingprinter.cpp
    wizards/newracewizard.cpp
    wizards/rankingswizard.cpp
    wizards/rankingswizardformat.cpp
    wizards/rankingswizardmode.cpp
    wizards/rankingswizardselection.cpp)

set(UI_FILES
    chronorace.ui
    chronoracedata.ui
    chronoracetable.ui
    chronoracetimings.ui
    livetable.ui)

set(PROJECT_SOURCES
    ${HEADER_FILES}
    ${SOURCE_FILES}
    ${UI_FILES})

# Translations
set(TS_FILES
    translations/lbchronorace_en.ts
    translations/lbchronorace_it.ts)

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

# Images (as standard resource)
list(APPEND PROJECT_SOURCES images.qrc)

if(WIN32)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/crversion.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/crversion.rc
        @ONLY)

    set(LBCHRONORACE_BIN ${CMAKE_PROJECT_NAME}) # Used also by the Windows Installer CMake code

    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/icons/LBChronoRace.rc")
    set(APP_VERSION_RESOURCE_WINDOWS "${CMAKE_CURRENT_BINARY_DIR}/crversion.rc")
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
    if(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
        set_source_files_properties(${APP_VERSION_RESOURCE_WINDOWS} PROPERTIES COMPILE_DEFINITIONS QT_QML_DEBUG)
    endif()

    qt_add_executable(${CMAKE_PROJECT_NAME} WIN32
        ${PROJECT_SOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
        ${APP_VERSION_RESOURCE_WINDOWS}
        ${TS_FILES}
        ${FONT_FILES}
        ${PROJECT_FONTS}
        ${ICON_FILES}
        ${PROJECT_ICONS})

    set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE)

    # Create the installer tree
    add_subdirectory(installer)
elseif(APPLE)
    set(LBCHRONORACE_BIN lbchronorace)

    qt_add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE
        ${PROJECT_SOURCES}
        ${TS_FILES}
        ${FONT_FILES}
        ${PROJECT_FONTS}
        ${ICON_FILES}
        ${PROJECT_ICONS})
else()
    set(LBCHRONORACE_BIN lbchronorace)

    qt_add_executable(${CMAKE_PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${TS_FILES}
        ${FONT_FILES}
        ${PROJECT_FONTS}
        ${ICON_FILES}
        ${PROJECT_ICONS})
endif()
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    LBCHRONORACE_ORGANIZATION=\"${PROJECT_ORGANIZATION}\"
    LBCHRONORACE_NAME=\"${PROJECT_NAME}\"
    LBCHRONORACE_VERSION=\"${LBCHRONORACE_VERSION_STRING}\")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_20)

# The name of the executable binary
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${LBCHRONORACE_BIN})

# Translations
qt_add_lupdate(TS_FILES ${TS_FILES}
    SOURCES ${PROJECT_SOURCES})
qt_add_lrelease(TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES)
qt_add_resources(${CMAKE_PROJECT_NAME} "translations"
    PREFIX "/i18n"
    BASE ${CMAKE_BINARY_DIR}
    FILES ${QM_FILES})

# Linked Qt libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
