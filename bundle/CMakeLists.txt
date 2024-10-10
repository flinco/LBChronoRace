##############################################################################
# Copyright (C) 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)                    #
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

# MacOS bundle creation

#include(CMakePrintHelpers)

#add_custom_target(installer
#    COMMENT "Setting up the tree for creating a MacOS App DMG")

set_source_files_properties(${ICON_FILES} PROPERTIES
       MACOSX_PACKAGE_LOCATION "Resources")

set(deploy_tool_options_arg
    -dmg
    -hardened-runtime)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}.${PROJECT_UPDATE}
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2025 by Lorenzo Buzzi (lorenzo@buzzi.pro)"
    MACOSX_BUNDLE_GUI_IDENTIFIER lbchronorace.buzzi.pro
    MACOSX_BUNDLE_ICON_FILE LBChronoRace.ico
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_STRING})

qt_generate_deploy_app_script(
    TARGET ${CMAKE_PROJECT_NAME}
    OUTPUT_SCRIPT deploy_script
    DEPLOY_TOOL_OPTIONS ${deploy_tool_options_arg}
    #NO_COMPILER_RUNTIME
    NO_UNSUPPORTED_PLATFORM_ERROR)

# Installation
install(TARGETS ${CMAKE_PROJECT_NAME}
    BUNDLE DESTINATION .)

# The following is not required since
# the translations are embedded resources
#install(FILES ${QM_FILES}
#    DESTINATION translations)

install(SCRIPT ${deploy_script})
