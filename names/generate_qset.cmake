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

# Read the text file line by line into a list
# (INPUT_FILE is passed via command line with -D)
file(STRINGS "${INPUT_FILE}" LINES)

set(RESULT "    /* This file is AUTO-GENERATED. Do not edit. */\n")

# Process each line
foreach(LINE IN LISTS LINES)
    # Convert the string to lowercase before wrapping it
    string(TOLOWER "${LINE}" LINE_LOWER)

    # Remove leading/trailing whitespace and carriage returns
    string(STRIP "${LINE_LOWER}" LINE_LOWER)

    # If the line is not empty, format it as a QStringLiteral
    if(NOT LINE_LOWER STREQUAL "")
        string(APPEND RESULT "    QStringLiteral(\"${LINE_LOWER}\"),\n")
    endif()
endforeach()

# Write the formatted strings to the header file
# (OUTPUT_FILE is passed via command line with -D)
file(WRITE "${OUTPUT_FILE}" "${RESULT}")
