include(CMakePrintHelpers)

add_custom_target("${CMAKE_PROJECT_NAME}Installer" ALL
                  COMMENT "Setting up the tree for the Qt Installer Framework")

#set(WIN_DEPLOY_DIR "${CMAKE_CURRENT_BINARY_DIR}/installer")
set(WIN_DEPLOY_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/data")
set(WIN_DEPLOY_SAMPLES_DIR "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace.samples/data")


set(WIN_DEPLOY_DIRS
    "config"
    "packages")
file(COPY ${WIN_DEPLOY_DIRS} DESTINATION "."
     PATTERN "*.in" EXCLUDE
     PATTERN "*.ts" EXCLUDE)

set(TargetDir          "@TargetDir@")
set(ApplicationsDirX64 "@ApplicationsDirX64@")
set(RootDir            "@RootDir@")
file(GLOB_RECURSE WIN_DEPLOY_CONFIG_XML
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     "*.xml.in")
foreach(WIN_DEPLOY_CONFIG_XML_IN ${WIN_DEPLOY_CONFIG_XML})
    cmake_path(REMOVE_EXTENSION WIN_DEPLOY_CONFIG_XML_IN LAST_ONLY
               OUTPUT_VARIABLE WIN_DEPLOY_CONFIG_XML_OUT)
    configure_file(${WIN_DEPLOY_CONFIG_XML_IN} ${WIN_DEPLOY_CONFIG_XML_OUT}
                   NEWLINE_STYLE WIN32)
endforeach()

# Installer images
file(COPY "${CMAKE_SOURCE_DIR}/icons/LBChronoRace.ico" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")
file(COPY "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-32.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")
file(COPY "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-64.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")
file(COPY "${CMAKE_SOURCE_DIR}/icons/LBChronoRace-128.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")
file(COPY "${CMAKE_SOURCE_DIR}/images/lbchronorace-preview.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")
file(COPY "${CMAKE_SOURCE_DIR}/images/lbchronorace-sample.png" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/config")

# Installer translations
set(WIN_DEPLOY_TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/pro.buzzi.lbchronorace/meta/en.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/packages/pro.buzzi.lbchronorace/meta/it.ts")
set_source_files_properties(${WIN_DEPLOY_TS_FILES}
    PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta")
qt_add_translations("${CMAKE_PROJECT_NAME}Installer"
                    TS_FILES ${WIN_DEPLOY_TS_FILES}
                    QM_FILES_OUTPUT_VARIABLE WIN_DEPLOY_QM_FILES
                    "${CMAKE_CURRENT_SOURCE_DIR}/packages/pro.buzzi.lbchronorace/meta/installscript.qs")

# License files
file(GENERATE OUTPUT "${WIN_DEPLOY_TARGET_DIR}/GPLv3.txt"
     INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0.txt"
     NEWLINE_STYLE WIN32)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta/license.txt"
     INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0.txt"
     NEWLINE_STYLE WIN32)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/packages/pro.buzzi.lbchronorace/meta/license_it.txt"
     INPUT "${CMAKE_SOURCE_DIR}/licenses/gpl-3.0_it.txt"
     NEWLINE_STYLE WIN32)

file(GLOB WIN_DEPLOY_ICONS
     LIST_DIRECTORIES false
     "${CMAKE_SOURCE_DIR}/icons/*.ico")
file(COPY ${WIN_DEPLOY_ICONS} DESTINATION ${WIN_DEPLOY_TARGET_DIR})

# Copy samples (moving to Windows newline style)
file(MAKE_DIRECTORY "${WIN_DEPLOY_SAMPLES_DIR}/samples")
file(GLOB_RECURSE WIN_DEPLOY_SAMPLES_CONTENTS
     LIST_DIRECTORIES true RELATIVE "${CMAKE_SOURCE_DIR}/samples"
     "${CMAKE_SOURCE_DIR}/samples/*")
foreach(SAMPLE_CONTENT ${WIN_DEPLOY_SAMPLES_CONTENTS})
    cmake_path(ABSOLUTE_PATH SAMPLE_CONTENT BASE_DIRECTORY "${CMAKE_SOURCE_DIR}/samples"
               OUTPUT_VARIABLE SAMPLE_CONTENT_IN)
    cmake_path(ABSOLUTE_PATH SAMPLE_CONTENT BASE_DIRECTORY "${WIN_DEPLOY_SAMPLES_DIR}/samples"
               OUTPUT_VARIABLE SAMPLE_CONTENT_OUT)
    if(IS_DIRECTORY ${SAMPLE_CONTENT_IN})
        file(MAKE_DIRECTORY ${SAMPLE_CONTENT_OUT})
    else()
        cmake_path(GET SAMPLE_CONTENT EXTENSION LAST_ONLY EXTENSION_CRD)
        if("${EXTENSION_CRD}" STREQUAL ".crd")
            file(COPY_FILE ${SAMPLE_CONTENT_IN} ${SAMPLE_CONTENT_OUT})
        else()
            file(GENERATE OUTPUT ${SAMPLE_CONTENT_OUT}
                 INPUT ${SAMPLE_CONTENT_IN}
                 NEWLINE_STYLE WIN32)
        endif()
    endif()
endforeach()

add_custom_command(TARGET "${CMAKE_PROJECT_NAME}Installer" POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>" ${WIN_DEPLOY_TARGET_DIR}
    DEPENDS "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
    COMMENT "Copy executable into the installer directory")
