cmake_minimum_required(VERSION 3.26)
project(alis)
set(${PROJECT_NAME}_MAJOR "0")
set(${PROJECT_NAME}_MINOR "1")
set(${PROJECT_NAME}_PATCH "0")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CPR_TAG 1.10.4)
set(JSON_TAG v3.11.2)
set(ARGPARSE_TAG v2.9)
set(CHANNEL_TAG v0.8.0)
set(COMMAND_TAG 9a9edca13f15182b0c405902747f86c4a19638ec)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    add_definitions(-Wall)
    add_definitions(-Wextra)
    add_definitions(-Wdouble-promotion)
    add_definitions(-Wno-unused-parameter)
    add_definitions(-Wsign-compare)
endif()

# Functions
function(add_source_files list)
    get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
    if(NOT is_defined)
        define_property(GLOBAL PROPERTY ${list}
                BRIEF_DOCS "List of source files"
                FULL_DOCS "List of source files to be compiled in one library")
    endif()
    # make absolute paths
    set(SRCS)
    foreach(s IN LISTS ARGN)
        if(NOT IS_ABSOLUTE "${s}")
            get_filename_component(s "${s}" ABSOLUTE)
        endif()
        list(APPEND SRCS "${s}")
    endforeach()
    # append to global list
    set_property(GLOBAL APPEND PROPERTY ${list} "${SRCS}")
endfunction(add_source_files)

# GNURadio and Osmocom
include(FindPkgConfig)
find_package(Gnuradio-osmosdr REQUIRED)

set(GR_REQUIRED_COMPONENTS RUNTIME ANALOG BLOCKS DIGITAL FILTER FFT)
find_package(Gnuradio REQUIRED COMPONENTS analog blocks digital filter fft)
if(NOT Gnuradio_FOUND)
    message(FATAL_ERROR "GnuRadio is required to compile ${PROJECT_NAME}")
endif()

# CPR
include(FetchContent)
FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG ${CPR_TAG}
)
FetchContent_MakeAvailable(cpr)

# argparse
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG ${ARGPARSE_TAG}
)
FetchContent_MakeAvailable(argparse)

# json
include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG ${JSON_TAG}
)
FetchContent_MakeAvailable(json)

# channel
include(FetchContent)
FetchContent_Declare(
        channel
        GIT_REPOSITORY https://github.com/andreiavrammsd/cpp-channel.git
        GIT_TAG ${CHANNEL_TAG}
)
FetchContent_Populate(channel)
include_directories(${channel_SOURCE_DIR}/include)

# command
include(FetchContent)
FetchContent_Declare(
        command
        GIT_REPOSITORY https://github.com/RaymiiOrg/cpp-command-output.git
        GIT_TAG ${COMMAND_TAG}
)
FetchContent_Populate(command)
include_directories(${command_SOURCE_DIR})

# Pass the GNU Radio version as 0xMMNNPP BCD.
math(EXPR GNURADIO_BCD_VERSION
        "(${Gnuradio_VERSION_MAJOR} / 10) << 20 |
     (${Gnuradio_VERSION_MAJOR} % 10) << 16 |
     (${Gnuradio_VERSION_MINOR} / 10) << 12 |
     (${Gnuradio_VERSION_MINOR} % 10) <<  8 |
     (${Gnuradio_VERSION_PATCH} / 10) <<  4 |
     (${Gnuradio_VERSION_PATCH} % 10) <<  0
    "
)

add_definitions(-DGNURADIO_VERSION=${GNURADIO_BCD_VERSION})

# print
message(STATUS "Gnuradio version: ${channel_SOURCE_DIR}")

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${GNURADIO_OSMOSDR_INCLUDE_DIRS}
)

link_directories(
        ${GNURADIO_RUNTIME_LIBRARY_DIRS}
)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${GNURADIO_OSMOSDR_LIBRARIES}
        gnuradio::gnuradio-analog
        gnuradio::gnuradio-blocks
        gnuradio::gnuradio-digital
        gnuradio::gnuradio-filter
        nlohmann_json::nlohmann_json
        cpr::cpr
        argparse
)
