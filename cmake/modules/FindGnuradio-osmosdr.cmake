if(NOT PKG_CONFIG_FOUND)
    include(FindPkgConfig)
endif()
pkg_check_modules(PC_GNURADIO_OSMOSDR gnuradio-osmosdr)

find_path(
    GNURADIO_OSMOSDR_INCLUDE_DIRS
    NAMES osmosdr/source.h
    HINTS $ENV{GNURADIO_OSMOSDR_DIR}/include
        ${PC_GNURADIO_OSMOSDR_INCLUDEDIR}
        ${CMAKE_INSTALL_PREFIX}/include/osmosdr
    PATHS /usr/local/include/osmosdr
          /usr/include/osmosdr
)

find_library(
    GNURADIO_OSMOSDR_LIBRARIES
    NAMES gnuradio-osmosdr
    HINTS $ENV{GNURADIO_OSMOSDR_DIR}/lib
        ${PC_GNURADIO_OSMOSDR_LIBDIR}
        ${CMAKE_INSTALL_PREFIX}/lib
        ${CMAKE_INSTALL_PREFIX}/lib64
    PATHS /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Gnuradio-osmosdr DEFAULT_MSG GNURADIO_OSMOSDR_LIBRARIES GNURADIO_OSMOSDR_INCLUDE_DIRS)
mark_as_advanced(GNURADIO_OSMOSDR_LIBRARIES GNURADIO_OSMOSDR_INCLUDE_DIRS)
