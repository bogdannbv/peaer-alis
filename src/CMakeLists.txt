add_subdirectory(api)
add_subdirectory(dsp)
add_subdirectory(pulseaudio)
add_subdirectory(receivers)
add_subdirectory(workers)

add_source_files(SRCS_LIST
        main.cpp
        receiver.cpp
        receiver.h
        utils.cpp
        utils.h
        zero_file.cpp
        zero_file.h
)

get_property(${PROJECT_NAME}_SOURCE GLOBAL PROPERTY SRCS_LIST)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCE})
