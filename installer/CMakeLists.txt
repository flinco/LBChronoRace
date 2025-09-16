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

# Windows installer creation (with Qt Installer Framework)

#include(CMakePrintHelpers)

add_custom_target(installer
    COMMENT "Setting up the tree for the Qt Installer Framework")

# External tools
get_target_property(QMAKE_EXECUTABLE Qt::qmake IMPORTED_LOCATION)
get_filename_component(_QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)

# Look for 'windeployqt'
find_program(WINDEPLOYQT_EXECUTABLE
    NAMES windeployqt
    HINTS "${_QT_BIN_DIR}"
    REQUIRED
    DOC "The Windows deployment tool")

# Look for 'binarycreator'
find_program(QTIFW_BINARYCREATOR_EXECUTABLE
    NAMES binarycreator
    HINTS "C:/Qt/Tools/QtInstallerFramework/4.10"
    PATH_SUFFIXES "bin"
    REQUIRED
    DOC "Qt Installer Framework command line client")

mark_as_advanced(WINDEPLOYQT_EXECUTABLE QTIFW_BINARYCREATOR_EXECUTABLE)

# Convenience path variables
set(WIN_DEPLOY_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/data")
set(WIN_DEPLOY_SAMPLES_DIR "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace.samples/data")
set(WIN_DEPLOY_DIRS "config" "packages")

# Cleanup
foreach(WIN_DEPLOY_DIR ${WIN_DEPLOY_DIRS})
    file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/${WIN_DEPLOY_DIR}")
endforeach()

# Create the basic tree
file(COPY ${WIN_DEPLOY_DIRS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
    PATTERN "*.in" EXCLUDE
    PATTERN "*.ts" EXCLUDE)

# Copy the XMLs: replacing the version and date placeholders but preserve other ones
set(at "@" CACHE STRING "QTIFW @ placeholder")
file(GLOB_RECURSE WIN_DEPLOY_CONFIG_XML RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.xml.in")
foreach(IN_FILE ${WIN_DEPLOY_CONFIG_XML})
    cmake_path(REMOVE_EXTENSION IN_FILE LAST_ONLY OUTPUT_VARIABLE OUT_FILE)
    configure_file(${IN_FILE} ${OUT_FILE} NEWLINE_STYLE WIN32)
endforeach()
unset(at CACHE)

# Installer images and icons
file(COPY
    "${CMAKE_SOURCE_DIR}/icons/LBChronoRace.ico"
    "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-32.png"
    "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-64.png"
    "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-128.png"
    "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-256.png"
    "${CMAKE_SOURCE_DIR}/images/lbchronorace-preview.png"
    "${CMAKE_SOURCE_DIR}/images/lbchronorace-sample.png"
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")

# Installer translations
set(WIN_DEPLOY_TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/pro.buzzi.lbchronorace/meta/en.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/pro.buzzi.lbchronorace/meta/it.ts")
qt_add_translations(installer
    TS_FILES ${WIN_DEPLOY_TS_FILES}
    QM_FILES_OUTPUT_VARIABLE WIN_DEPLOY_QM_FILES)

foreach(QM_FILE ${WIN_DEPLOY_QM_FILES})
    get_filename_component(QM_NAME "${QM_FILE}" NAME)
    set(QM_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta/${QM_NAME}")

    add_custom_command(
        OUTPUT ${QM_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy "${QM_FILE}" "${QM_OUTPUT}"
        DEPENDS ${QM_FILE}
        COMMENT "Copying translation file: ${QM_NAME}")
    list(APPEND INSTALLED_QM_FILES ${QM_OUTPUT})
endforeach()

add_custom_target(install_qms DEPENDS ${INSTALLED_QM_FILES})
add_dependencies(installer install_qms)

# Installer license files (moving to Windows newline style)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta/license.txt"
    INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0.txt"
    NEWLINE_STYLE WIN32)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta/license_it.txt"
    INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0_it.txt"
    NEWLINE_STYLE WIN32)

# Application license file (moving to Windows newline style)
file(GENERATE OUTPUT "${WIN_DEPLOY_TARGET_DIR}/GPLv3.txt"
    INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0.txt"
    NEWLINE_STYLE WIN32)

# Application icons
file(GLOB WIN_DEPLOY_ICONS LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/icons/*.ico")
file(COPY ${WIN_DEPLOY_ICONS} DESTINATION ${WIN_DEPLOY_TARGET_DIR})

# Data samples
file(MAKE_DIRECTORY "${WIN_DEPLOY_SAMPLES_DIR}/samples")
file(GLOB_RECURSE WIN_DEPLOY_SAMPLES_CONTENTS
    LIST_DIRECTORIES true RELATIVE "${CMAKE_SOURCE_DIR}/samples"
    "${CMAKE_SOURCE_DIR}/samples/*")
foreach(SAMPLE_CONTENT ${WIN_DEPLOY_SAMPLES_CONTENTS})
    cmake_path(ABSOLUTE_PATH SAMPLE_CONTENT BASE_DIRECTORY "${CMAKE_SOURCE_DIR}/samples"
        OUTPUT_VARIABLE IN_PATH)
    cmake_path(ABSOLUTE_PATH SAMPLE_CONTENT BASE_DIRECTORY "${WIN_DEPLOY_SAMPLES_DIR}/samples"
        OUTPUT_VARIABLE OUT_PATH)

    if(IS_DIRECTORY ${IN_PATH})
        file(MAKE_DIRECTORY ${OUT_PATH})
    else()
        cmake_path(GET SAMPLE_CONTENT EXTENSION LAST_ONLY EXT)
        if("${EXT}" STREQUAL ".crd")
            file(COPY_FILE ${IN_PATH} ${OUT_PATH})
        else()
            file(GENERATE OUTPUT ${OUT_PATH} INPUT ${IN_PATH} NEWLINE_STYLE WIN32)
        endif()
    endif()
endforeach()

# Application executable (copy)
add_custom_command(TARGET installer PRE_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${LBCHRONORACE_BIN}>" ${WIN_DEPLOY_TARGET_DIR}
    DEPENDS "$<TARGET_FILE:${LBCHRONORACE_BIN}>"
    COMMENT "Copy executable into the installer directory"
    USES_TERMINAL)

# Populate the installer tree with the runtime dependencies
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
    set(WINDEPLOYQT_ARGS "--debug")
else()
    set(WINDEPLOYQT_ARGS "--release")
endif()

cmake_path(GET CMAKE_CXX_COMPILER PARENT_PATH _GXX_BIN_DIR)
add_custom_command(TARGET installer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env PATH="${_GXX_BIN_DIR}" ${WINDEPLOYQT_EXECUTABLE}
        --qmldir ${CMAKE_SOURCE_DIR}
        ${WINDEPLOYQT_ARGS}
        --dir ${WIN_DEPLOY_TARGET_DIR}
        --no-compiler-runtime
        "${WIN_DEPLOY_TARGET_DIR}/$<TARGET_FILE_NAME:${LBCHRONORACE_BIN}>"
    COMMENT "Deploying ${CMAKE_PROJECT_NAME}..."
    USES_TERMINAL)

# Build the installer
# consider building an on-line installer or examine the possibility to use --hybrid
add_custom_command(TARGET installer POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E env ${QTIFW_BINARYCREATOR_EXECUTABLE}
        -c "config/config.xml"
        -p "packages"
        -v "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Installer${CMAKE_EXECUTABLE_SUFFIX}"
    COMMENT "Crearting ${CMAKE_PROJECT_NAME} installer"
    USES_TERMINAL)

# Build the launcher that set AllUsers
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/launcher.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/launcher.rc
    @ONLY)

set(INSTALLER_BINARY "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Installer${CMAKE_EXECUTABLE_SUFFIX}")

# Embed the installer in the launcher executable
set(INSTALLER_RC "${CMAKE_CURRENT_BINARY_DIR}/installer.rc")
file(WRITE ${INSTALLER_RC} "#include \"launcher.hpp\"\n\nIDR_INSTALLER_BIN RCDATA \"${INSTALLER_BINARY}\"\n")

# Create the launcher executable
set(APP_ICON_RESOURCE_LAUNCHER "${CMAKE_SOURCE_DIR}/icons/LBChronoRace.rc")
set(APP_VERSION_RESOURCE_LAUNCHER "${CMAKE_CURRENT_BINARY_DIR}/launcher.rc")
if(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEBUG")
set_source_files_properties(${APP_VERSION_RESOURCE_LAUNCHER} PROPERTIES COMPILE_DEFINITIONS QT_QML_DEBUG)
endif()
add_executable(launcher WIN32 EXCLUDE_FROM_ALL
    "${CMAKE_SOURCE_DIR}/installer/launcher.cpp"
    ${APP_ICON_RESOURCE_LAUNCHER}
    ${APP_VERSION_RESOURCE_LAUNCHER}
    ${INSTALLER_RC})
set_target_properties(launcher PROPERTIES
    WIN32_EXECUTABLE TRUE
    RUNTIME_OUTPUT_NAME "${CMAKE_PROJECT_NAME}Installer-${PROJECT_VERSION}u${PROJECT_UPDATE}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
target_include_directories(launcher PRIVATE ${CMAKE_BINARY_DIR})
add_dependencies(launcher installer)
