cmake_minimum_required(VERSION 3.12)
project(cutter-jsdec-plugin)

set(CUTTER_INSTALL_PLUGDIR "share/rizin/cutter/plugins/native" CACHE STRING "Directory to install Cutter plugin into")
set(JSDEC_BUILD_DIR "../build" CACHE STRING "Directory where to find libjsdec.a")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/Cutter")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(jsdec_cutter MODULE
	JSDecPlugin.h
	JSDecPlugin.cpp
	JSDecDecompiler.h
	JSDecDecompiler.cpp
)

find_library(libjsdec NAMES jsdec PATHS "${JSDEC_BUILD_DIR}")
include_directories("../include")

find_package(Cutter REQUIRED)

target_link_libraries(jsdec_cutter PRIVATE Cutter::Cutter "${libjsdec}")
install(TARGETS jsdec_cutter DESTINATION "${CUTTER_INSTALL_PLUGDIR}")
