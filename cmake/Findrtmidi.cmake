# First try to use a cmake build if there is one
find_package(rtmidi QUIET CONFIG)

if(${rtmidi_FOUND})

    # The rtmidi find_package config wants us to #include <rtmidi/RtMidi.h>
    # which is different from how documentation and pkg-config does it.
    get_target_property(RTMIDI_INCLUDE_DIR RtMidi::rtmidi INTERFACE_INCLUDE_DIRECTORIES)
    string(APPEND RTMIDI_INCLUDE_DIR "/rtmidi")

    set_target_properties(RtMidi::rtmidi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RTMIDI_INCLUDE_DIR}"
    )
    get_target_property(RTMIDI_LIBRARY RtMidi::rtmidi LOCATION)
 
    message(STATUS "Using cmake-enabled rtmidi:
        Include: ${RTMIDI_INCLUDE_DIR}
        Library: ${RTMIDI_LIBRARY}"
    )

else()

    # TODO: attempt to pkg-config?
    find_path(RTMIDI_INCLUDE_DIR RtMidi.h PATH_SUFFIXES rtmidi)

    find_library(RTMIDI_LIBRARY NAMES rtmidi)

    add_library(RtMidi::rtmidi INTERFACE IMPORTED)
    set_target_properties(RtMidi::rtmidi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RTMIDI_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${RTMIDI_LIBRARY}"
    )

    message(STATUS "Using rtmidi:
    Include: ${RTMIDI_INCLUDE_DIR}
    Library: ${RTMIDI_LIBRARY}"
    )

endif()

if(rtmidi_FIND_REQUIRED)
     if(NOT (RTMIDI_INCLUDE_DIR AND RTMIDI_LIBRARY))
         message(FATAL_ERROR "Failed to locate rtmidi. Either:\n"
             "1. Build and install rtmidi using cmake, then set CMAKE_PREFIX_PATH so that it contains the install directory\n"
             "2. If you have a generic rtmidi installation, set RTMIDI_INCLUDE_DIR and RTMIDI_LIBRARY manually\n")
     endif()
 endif()
