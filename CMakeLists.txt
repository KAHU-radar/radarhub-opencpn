# ---------------------------------------------------------------------------
# Frontend2 Author:   jongough aka. Jon Gough Copyright: License:  wxWidgets
# License
# ---------------------------------------------------------------------------
# - Copyright (C)  by Egil Moeller
# - Copyright (C)  by Jon Gough (Template repo)                           *
# - License:     GPLv3+
# - This program is free software; you can redistribute it and/or modify  *
# - it under the terms of the GNU General Public License as published by  *
# - the Free Software Foundation; either version 2 of the License, or     *
# - (at your option) any later version.                                   *
# - *
# - This program is distributed in the hope that it will be useful,       *
# - but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# - MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# - GNU General Public License for more details.                          *
# - *
# - You should have received a copy of the GNU General Public License     *
# - along with this program; if not, write to the                         *
# - xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,                                       *
# - xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                       *
# -----------------------------------------------------------------------------
# Frontend2 Author: Jon Gough Crowdsource(TP) v1.0.314.0 24/10/2024 DD/MM/YYYY
# format buildCI:yes useODAPI:yes validateJSON:yes
# ---------------------------------------------------------------------------
# -- When changing this file do NOT change the order of the sections.    -- ##
# -- Make changes only between the section blocks identified             -- ##
# -- The whole configuration process relies on this sequence for success.-- ##
#
# -- Modify section below to include all the details for your plugin.    -- ##

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.1)
if(COMMAND cmake_policy)
  if (POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
  endif (POLICY CMP0043)
  cmake_policy(SET CMP0048 NEW)
  if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
  endif (POLICY CMP0076)
  if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
  endif (POLICY CMP0077)
endif (COMMAND cmake_policy)

include(ExternalProject)

# set for vscode/vscodium
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# define plugin name, owner and versions
set(VERBOSE_NAME "KAHU Radar Hub")
set(COMMON_NAME "KAHU Radar Hub")
set(TITLE_NAME "radarhub")
set(PACKAGE_CONTACT "Egil Moeller")
set(PACKAGE "radarhub")
set(SHORT_DESCRIPTION
    "Contribute AIS and ARPA targets to crowdsourcing for marine safety"
)
set(LONG_DESCRIPTION
    "Upload AIS and ARPA targets (or any NMEA) to an internet server. Upload can be continuous, over intermittent internet, or scheduled, and tracks can be downsampled to fit your bandwidth."
)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "196")
set(VERSION_TWEAK "0")
set(VERSION_DATE "11/12/2024") # DD/MM/YYYY format
set(OCPN_MIN_VERSION "ov58")

set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "18")
set(TP_COMMENT "  * Release using CI")

set(PARENT "opencpn")

# Use to set up a local webserver with git installed. Default is github.com. Now
# set in pluginconfigure.cmake set(GIT_REPOSITORY_SERVER "github.com")

# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod") # Standard Repos
set(BETA "beta") # Standard Repos
set(ALPHA "alpha") # Standard Repos

# Set if  Cloudsmith Base Repository name does not match your Git Repository
# name. set(CLOUDSMITH_BASE_REPOSITORY "${GIT_REPOSITORY_NAME}")
# set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Ricks's customization
set(CLOUDSMITH_BASE_REPOSITORY "radarhub") # without the pi

# Set the following to use OpenCPN cloudsmith repositories instead of own
# repositories
set(CLOUDSMITH_USER "egil-moller")
# set(CLOUDSMITH_USER "jon-gough")

set(XML_INFO_URL
    "https://opencpn.org/OpenCPN/plugins/radarhub.html"
)
set(XML_SUMMARY ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION ${LONG_DESCRIPTION})

# ----- Modify section above to include all the details for your plugin
#
# ----- Modify section below if there are special requirements for the plugin
#

option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Use local version of GLU library requires libs/glu directory
set(USE_LOCAL_GLU FALSE)
# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY") # not operational

option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE
      "Debug" # "Release"
      CACHE STRING "Choose the type of build" FORCE
  )
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(
  CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo"
                                  "MinSizeRel"
)
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")

# ----- Statements below used to setup standard variables required by the CMAKE
# ----- process -- Do not remove --

project(${PACKAGE})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ========================================================

include("PluginSetup")

# ========================================================
# ----- Above sets up standard variables required by CMAKE - Do not remove

# ----- Modify section below if there are special requirements for the plugin

set(CMAKE_VERBOSE_MAKEFILE "Activate verbose mode for make files" ON)

option(Plugin_CXX11 "Use c++11" OFF)

# ----- Modify section above if there are special requirements for the plugin

# ----- Do not change next section - needed to configure build process
# ===========================================================

include("PluginConfigure")

# ===========================================================
# ----- Do not change section above - needed to configure build process

# ----- Change below to match project requirements for source, headers, etc.

add_definitions(-DUSE_S57)

# ---- Section Below - Add your project SET(SRCS and SET(HDRS  etc..

set(SRCS
    src/radarhub_pi.cpp
    src/routecache.cpp
    src/connector.cpp
    src/sqlutils.cpp
    src/socketutils.cpp
    src/avroutils.cpp
    src/preferences.cpp
)

set(HDRS
    include/radarhub_pi.h
    include/routecache.h
    include/connector.h
    include/sqlutils.h
    include/socketutils.h
    include/avroutils.h
    src/preferences.h # Yes, really, this is a wxglade thing... grr
)

add_definitions(-DPLUGIN_USE_SVG)

# ----- Section above - Add your project SET(SRCS and SET(HDRS  etc.


if(WIN32)
    target_compile_definitions(${PACKAGE_NAME} PRIVATE _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

    set(LIBPREFIX "")
    set(LIBEXT "libstatic.lib")
    set(LIBDIR "/RelWithDebInfo")
    set(ZLIB "zlibstatic.lib")
    set(LZMALIB "lzma.lib")
    set(JANSSONLIB "jansson.lib")
    set(AVROLIB "avro.lib")
else()
    set(LIBPREFIX "lib")
    set(LIBEXT ".a")
    set(LIBDIR "")
    set(ZLIB "libz.a")
    set(LZMALIB "liblzma.a")
    set(JANSSONLIB "libjansson.a")
    set(AVROLIB "libavro.a")
endif()

if(WIN32)
    set(SEP ";")
else()
    set(SEP ":")
endif()

# For liblzma, if we want to enable this, we need gettext, and on
# macos universal, we need a non-brew, universal version of gettext...
set(ENABLE_NLS OFF)

set(ENV{PKG_CONFIG_PATH} "${CMAKE_BINARY_DIR}/libs/jansson${SEP}${CMAKE_BINARY_DIR}/libs/avro/lang/c/src${SEP}${CMAKE_BINARY_DIR}/libs/zlib${SEP}${CMAKE_BINARY_DIR}/libs/liblzma${SEP}$ENV{PKG_CONFIG_PATH}" CACHE STRING "Paths for pkg-config")

message(STATUS "PKGS $ENV{PKG_CONFIG_PATH}")

set(BUILD_SHARED_LIBS OFF)

set(ORIG_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(ORIG_SKIP_INSTALL_ALL ${SKIP_INSTALL_ALL})
set(ORIG_INSTALL_INC_DIR ${INSTALL_INC_DIR})
set(ORIG_includedir ${includedir})
set(ORIG_CMAKE_INSTALL_FULL_INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(ORIG_CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
set(ORIG_CMAKE_INSTALL_LIBDIR = ${CMAKE_INSTALL_LIBDIR})
set(ORIG_CMAKE_INSTALL_FULL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

set(SKIP_INSTALL_ALL True)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/libs/zlib)
set(INSTALL_INC_DIR ${CMAKE_BINARY_DIR}/libs/zlib)
set(ZLIB_BUILD_EXAMPLES OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/zlib)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/libs/liblzma)
set(CMAKE_INSTALL_FULL_INCLUDEDIR ${CMAKE_SOURCE_DIR}/libs/liblzma/src/liblzma/api/)
set(CMAKE_INSTALL_INCLUDEDIR ../../../libs/liblzma/src/liblzma/api/)
set(CMAKE_INSTALL_LIBDIR .)
set(CMAKE_INSTALL_FULL_LIBDIR ${CMAKE_BINARY_DIR}/libs/liblzma)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/liblzma)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/libs/jansson)
set(includedir ${CMAKE_SOURCE_DIR}/libs/jansson/src)
set(JANSSON_BUILD_SHARED_LIBS OFF)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/jansson)

add_definitions(-DLZMA_API_STATIC)
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/libs/avro/lang/c)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/avro/lang/c)
include_directories("${CMAKE_SOURCE_DIR}/libs/avro/lang/c/src")

if (WIN32)
    set_target_properties(avro-static PROPERTIES COMPILE_FLAGS "/Gd")
endif()

# # Wrap, to simulate target_link_libraries(${PACKAGE_NAME} PRIVATE libname)
# add_library(zlibstatic-wrapper INTERFACE)
# target_link_libraries(zlibstatic-wrapper INTERFACE zlibstatic)
# add_library(jansson-wrapper INTERFACE)
# target_link_libraries(jansson-wrapper INTERFACE jansson)
# add_library(liblzma-wrapper INTERFACE)
# target_link_libraries(liblzma-wrapper INTERFACE liblzma)
# add_library(avro-static-wrapper INTERFACE)
# target_link_libraries(avro-static-wrapper INTERFACE avro-static)
# set_target_properties(avro-static PROPERTIES LINK_INTERFACE_LIBRARIES "")

target_link_libraries(${PACKAGE_NAME} zlibstatic)
target_link_libraries(${PACKAGE_NAME} jansson)
target_link_libraries(${PACKAGE_NAME} liblzma)
target_link_libraries(${PACKAGE_NAME} avro-static)

set_target_properties(avro-static PROPERTIES INTERFACE_LINK_LIBRARIES "zlibstatic;jansson;liblzma")

get_target_property(FINAL_LINK_LIBRARIES ${PACKAGE_NAME} LINK_LIBRARIES)
message(STATUS "XXXXXXXXXXXXX FINAL_LINK_LIBRARIES: ${FINAL_LINK_LIBRARIES}")

set(CMAKE_INSTALL_FULL_LIBDIR ${ORIG_CMAKE_INSTALL_FULL_LIBDIR})
set(CMAKE_INSTALL_LIBDIR = ${ORIG_CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_INCLUDEDIR ${ORIG_CMAKE_INSTALL_INCLUDEDIR})
set(CMAKE_INSTALL_FULL_INCLUDEDIR ${ORIG_CMAKE_INSTALL_FULL_INCLUDEDIR})
set(includedir ${ORIG_includedir})
set(INSTALL_INC_DIR ${ORIG_INSTALL_INC_DIR})
set(SKIP_INSTALL_ALL ${ORIG_SKIP_INSTALL_ALL})
set(CMAKE_INSTALL_PREFIX ${ORIG_CMAKE_INSTALL_PREFIX})


add_dependencies(avro-static zlibstatic jansson liblzma)
add_dependencies(${PACKAGE_NAME} avro-static)


# ----- If using JSON validation in plugin section below is needed

if (APPLE)
  option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
else (APPLE)
  option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
endif (APPLE)

set(PI_JSON_SCHEMA_VALIDATOR ON)

if (PI_JSON_SCHEMA_VALIDATOR)
  message(
    STATUS
      "${CMLOC}Using JSON Schema validation - warning this changes the 'flavour' of the executable and it may not be compatible with OpenCPN"
  )
  add_definitions(-DPI_JSON_SCHEMA_VALIDATOR)
  message(STATUS "${CMLOC}Adding JSON Schema validation")

  add_subdirectory(opencpn-libs/json-schema-validator)
  target_link_libraries(${PACKAGE_NAME} ocpn::json-schema-validator)

else (PI_JSON_SCHEMA_VALIDATOR)
  message(
    STATUS
      "${CMLOC}Not using JSON Schema validation - warning invalid JSON 'may' cause unexpected crashes"
  )
endif (PI_JSON_SCHEMA_VALIDATOR)

# ----- If using JSON validation in plugin section above is needed

if (QT_ANDROID)

endif (QT_ANDROID)

# =============================================================
# Change as required to collect all the set( headers and SRCS This add_library
# can be used in conditionals below.
# =============================================================

set(SRC ${SRCS} ${HDRS})

# =============================================================
# Modify/configure 'Include' Directories below for the Plugin
# =============================================================

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)

# Change target_sources to match your project needs.
# ==============================================================

target_sources(${PACKAGE_NAME} PUBLIC ${SRC})

# ==============================================================

if (NOT OCPN_FLATPAK_CONFIG)
  # Build environment not available when flatpak is being configured so
  # following statements will not work
  message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

  if (WIN32)
    add_subdirectory(opencpn-libs/WindowsHeaders)
    target_link_libraries(${PACKAGE_NAME} windows::headers)
  endif (WIN32)

  add_subdirectory(opencpn-libs/api-18)
  target_link_libraries(${PACKAGE_NAME} ocpn::api)

  add_subdirectory(opencpn-libs/tinyxml)
  target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

  add_subdirectory(opencpn-libs/odapi)
  target_link_libraries(${PACKAGE_NAME} ocpn::odapi)

  add_subdirectory(opencpn-libs/jsonlib)
  target_link_libraries(${PACKAGE_NAME} ocpn::jsonlib)

  add_subdirectory(opencpn-libs/sqlite)
  target_link_libraries(${PACKAGE_NAME} ocpn::sqlite)

  add_subdirectory(opencpn-libs/wxJSON)
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)

  add_subdirectory(opencpn-libs/plugin_dc)
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

endif (NOT OCPN_FLATPAK_CONFIG)

add_definitions(-DTIXML_USE_STL)

#
# ----- If using JSON validation section below is needed
#

if (UNIX
    AND NOT APPLE
    AND NOT QT_ANDRIOD
)
  if (PI_JSON_SCHEMA_VALIDATOR)
    # Change OS/ABI to be UNIX - System V to make it run with OCPN
    set(lib_name lib${PACKAGE_NAME}.so)
    message(STATUS "${CMLOC}lib-name: ${lib_name}")
    find_file(elfedit elfedit)
    if (NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
      message(STATUS "${CMLOC}Will ensure OS/ABI is set to 0 to match OCPN")
      add_custom_command(
        TARGET ${PACKAGE_NAME}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${PACKAGE_NAME}
        COMMENT " Running post build action on ${lib_name}."
        COMMAND
          sh -c
          'elfedit --input-osabi=Linux --output-osabi=none ${lib_name} || (echo 0) '
      )
    else (NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
      message(
        STATUS
          "${CMLOC}Cannot correct OS/ABI to match OCPN if it is generated incorrectly. Package binutils needed"
      )
    endif (NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
  endif (PI_JSON_SCHEMA_VALIDATOR)
endif (
  UNIX
  AND NOT APPLE
  AND NOT QT_ANDRIOD
)

# ------- Change below to match project requirements

# Needed for android builds
if (QT_ANDROID)
  include_directories(BEFORE ${qt_android_include})
endif (QT_ANDROID)

# Needed for all builds Do not change - needed to build app
# ===============================================================

include("PluginInstall")
include("PluginLocalization")
include("PluginPackage")

# ================================================================
# ----- Do not change section above - needed to configure build process

message(STATUS "${CMLOC}include directories: ")
get_property(
  dirs
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY INCLUDE_DIRECTORIES
)
foreach (dir ${dirs})
  message(STATUS "${CMLOC}dir='${dir}'")
endforeach ()

# foreach(lib zlibstatic jansson liblzma avro-static ocpn::json-schema-validator ocpn::api ocpn::tinyxml ocpn::odapi ocpn::jsonlib ocpn::sqlite ocpn::wxjson ocpn::plugin-dc)
#     get_target_property(LINK_LIBS ${lib} INTERFACE_LINK_LIBRARIES)
#     message(STATUS "AAA ${lib} INTERFACE_LINK_LIBRARIES: ${LINK_LIBS}")
#     get_target_property(LINK_LIBS ${lib} LINK_INTERFACE_LIBRARIES)
#     message(STATUS "BBB ${lib} LINK_INTERFACE_LIBRARIES: ${LINK_LIBS}")
#     get_target_property(LINK_LIBS ${lib} LINK_FLAGS)
#     message(STATUS "BBB ${lib} LINK_FLAGS: ${LINK_LIBS}")
# endforeach()

# get_target_property(FINAL_LINK_LIBRARIES ${PACKAGE_NAME} LINK_LIBRARIES)
# message(STATUS "XXXXXXXXXXXXX FINAL_LINK_LIBRARIES: ${FINAL_LINK_LIBRARIES}")
