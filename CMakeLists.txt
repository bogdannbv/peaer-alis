cmake_minimum_required(VERSION 3.26)
project(peaer)
set(${PROJECT_NAME}_MAJOR "0")
set(${PROJECT_NAME}_MINOR "1")
set(${PROJECT_NAME}_PATCH "0")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CPR_VERSION 1.10.4)
SET(ARGPARSE_VERSION v2.9)

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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux|FreeBSD")
    if(NOT LINUX_AUDIO_BACKEND)
        set(LINUX_AUDIO_BACKEND Pulseaudio CACHE STRING "Choose the audio backend, options are: Pulseaudio, Portaudio, Gr-audio" FORCE)
    endif()

    if(${LINUX_AUDIO_BACKEND} MATCHES "Pulseaudio")
        message(STATUS "Pulseaudio backend enabled")
        find_package(PulseAudio REQUIRED)
        # there is a defect in the pulse audio cmake file that does not include this library. So we add it here.
        find_library(PULSE-SIMPLE NAMES pulse-simple REQUIRED)
        add_definitions(-DWITH_PULSEAUDIO)
        unset(PORTAUDIO_INCLUDE_DIRS CACHE)
        unset(PORTAUDIO_LIBRARIES CACHE)
    elseif(${LINUX_AUDIO_BACKEND} MATCHES "Portaudio")
        message(STATUS "Portaudio backend enabled")
        find_package(PORTAUDIO REQUIRED)
        add_definitions(-DWITH_PORTAUDIO)
        unset(PULSEAUDIO_FOUND CACHE)
        unset(PULSEAUDIO_INCLUDE_DIR CACHE)
        unset(PULSEAUDIO_LIBRARY CACHE)
        unset(PulseAudio_DIR CACHE)
        unset(PULSE-SIMPLE CACHE)
        unset(PULSEAUDIO_INCLUDE_DIR CACHE)
        unset(PULSEAUDIO_MAINLOOP_LIBRARY CACHE)
    elseif(${LINUX_AUDIO_BACKEND} MATCHES "Gr-audio")
        message(STATUS "Gr-audio backend enabled")
        unset(PULSEAUDIO_FOUND CACHE)
        unset(PULSEAUDIO_INCLUDE_DIR CACHE)
        unset(PULSEAUDIO_LIBRARY CACHE)
        unset(PulseAudio_DIR CACHE)
        unset(PULSE-SIMPLE CACHE)
        unset(PULSEAUDIO_INCLUDE_DIR CACHE)
        unset(PULSEAUDIO_MAINLOOP_LIBRARY CACHE)
        unset(PORTAUDIO_INCLUDE_DIRS CACHE)
        unset(PORTAUDIO_LIBRARIES CACHE)
    else()
        message(FATAL_ERROR "Invalid audio backend: should be either Pulseaudio, Portaudio or Gr-audio")
    endif()
endif()

# GNURadio and Osmocom
include(FindPkgConfig)
find_package(Gnuradio-osmosdr REQUIRED)

set(GR_REQUIRED_COMPONENTS RUNTIME ANALOG AUDIO BLOCKS DIGITAL FILTER FFT PMT)
find_package(Gnuradio REQUIRED COMPONENTS analog audio blocks digital filter fft network)
if(NOT Gnuradio_FOUND)
    message(FATAL_ERROR "GnuRadio is required to compile ${PROJECT_NAME}")
endif()

# CPR
include(FetchContent)
FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG ${CPR_VERSION}
)
FetchContent_MakeAvailable(cpr)

# argparse
include(FetchContent)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
        GIT_TAG ${ARGPARSE_VERSION}
)
FetchContent_MakeAvailable(argparse)

# spdlog
include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG 1.12.0
)
FetchContent_MakeAvailable(argparse)

# json
include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

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
        ${PULSEAUDIO_LIBRARY}
        ${PULSE-SIMPLE}
        gnuradio::gnuradio-analog
        gnuradio::gnuradio-blocks
        gnuradio::gnuradio-digital
        gnuradio::gnuradio-filter
        gnuradio::gnuradio-network
        gnuradio::gnuradio-audio
        nlohmann_json::nlohmann_json
        cpr::cpr
        argparse
)
