##############################################################################
# Copyright (C) 2026 by Lorenzo Buzzi (lorenzo@buzzi.pro)                    #
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

# Find all .list files in the current directory
file(GLOB LIST_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.list")
set(OUT_FILES "")

foreach(LIST_FILE IN LISTS LIST_FILES)
    # Get the filename without extension (e.g., "males" from "males.list")
    get_filename_component(NAME "${LIST_FILE}" NAME_WE)

    set(IN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${LIST_FILE}")
    set(OUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated_${NAME}.inc")

    # This runs the script IMMEDIATELY during CMake configuration
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -D "INPUT_FILE=${IN_FILE}"
            -D "OUTPUT_FILE=${OUT_FILE}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/generate_qset.cmake"
        RESULT_VARIABLE gen_result)

    if(NOT gen_result EQUAL 0)
        message(FATAL_ERROR "Failed to generate ${OUT_FILE}")
    endif()

    # This tells CMake to re-run configuration if the .txt files change
    set_source_files_properties("${OUT_FILE}" PROPERTIES GENERATED TRUE HEADER_FILE_ONLY TRUE)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${IN_FILE}")

    # Collect the path of the generated file
    list(APPEND OUT_FILES "${OUT_FILE}")
endforeach()

# Export the list of generated files to the parent scope (root CMakeLists.txt)
set(GENERATED_NAMES_INC ${OUT_FILES} PARENT_SCOPE)
