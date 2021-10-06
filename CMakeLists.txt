# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

cmake_minimum_required(VERSION 3.20)
project(LBChronoRace
    VERSION 3.0.2
    DESCRIPTION "Timings for Races"
    HOMEPAGE_URL "http://github.com/flinco/LBChronoRace"
    LANGUAGES CXX)

string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)

add_compile_definitions(LBCHRONORACE_NAME=\"${PROJECT_NAME}\")
add_compile_definitions(LBCHRONORACE_VERSION=\"${PROJECT_VERSION}\")
add_subdirectory(translations)

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
    classentry.cpp
    classentry.h
    competitor.cpp
    competitor.h
    crloader.cpp
    crloader.h
    crtablemodel.h
    csvrankingprinter.h
    csvrankingprinter.cpp
    lbchronorace.cpp
    lbchronorace.h
    lbcrexception.cpp
    lbcrexception.h
    pdfrankingprinter.h
    pdfrankingprinter.cpp
    rankingsbuilder.cpp
    rankingsbuilder.h
    rankingprinter.h
    rankingprinter.cpp
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
    txtrankingprinter.cpp
)

set(TS_FILES
    translations/LBChronoRace_en.ts
    translations/LBChronoRace_it.ts
)

# Only call lrelease on each translation files to create qm files.
# This command assumes that the ts files already exists (manually created).
#qt_add_translation(QM_FILES ${CMAKE_SOURCE_DIR}/translations ${TS_FILES})
# Call lupdate to generate (or update already existing) ts files from source
# code (.cpp and .ui) code AND generate qm files. We can call it with a source
# directory OR individual files to search for strings to translate. The ts files
# are generated in the source directory and the qm files are created in the
# build directory, as it should, because we don't want to pollute our source
# directory with generated binary files.
#
# Recall that the ts files are edited by translators, while qm files are just
# binary representation of the ts file for perfomance and size optimization.
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

# The qm files are generated in the build tree, but the qrc file is inside the
# source directory and the path to resources are relative to the location of
# the qrc file itself. We use configure_file() to copy the qrc file in the build
# directory such that it can find the qm translations files. The qrc file is
# copied if it doesn't exist in the destination or if it is modified.
configure_file(translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)

# We have to reference the translations.qrc copy in the build directory, not the
# original file in the source directory. We also add qm files to the target to
# create a dependency that will force rebuild in case the translation are
# updated.
set(PROJECT_RESOURCES
    icons.qrc
    fonts-mono.qrc
    fonts-sans.qrc
    fonts-serif.qrc
    ${CMAKE_BINARY_DIR}/translations.qrc
)

if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_SOURCE_DIR}/icons/LBChronoRace.rc")
    qt_add_executable(${CMAKE_PROJECT_NAME} WIN32
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
        ${QM_FILES}
        ${APP_ICON_RESOURCE_WINDOWS})

    # Create the installer tree (in Release mode only)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        add_subdirectory(installer)
    endif()
else()
    qt_add_executable(${CMAKE_PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
        ${QM_FILES})
endif()


target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
