cmake_minimum_required(VERSION 3.5)

project(LBChronoRace LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES LBChronoRace_it.ts)

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
        lbchronorace.cpp
        lbchronorace.h
        lbcrexception.cpp
        lbcrexception.h
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
        ${TS_FILES}
)

set(PROJECT_RESOURCES
        icons.qrc
        fonts.qrc
        translations.qrc
        distfiles.qrc
)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/icons.qrc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LBChronoRace
        ${PROJECT_SOURCES}
        ${PROJECT_RESOURCES}
        ${APP_ICON_RESOURCE_WINDOWS}
    )

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(LBChronoRace SHARED
            ${PROJECT_SOURCES}
            ${PROJECT_RESOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    else()
        add_executable(LBChronoRace
            ${PROJECT_SOURCES}
            ${PROJECT_RESOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(LBChronoRace PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
