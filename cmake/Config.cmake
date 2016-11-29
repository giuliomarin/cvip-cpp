# Additional inclusions
include (cmake/FindArch.cmake)

# ----------------
# Common properties
# ----------------

# set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#-------------------
# Compiler features
#-------------------

if(MSVC)
    # enable multiprocessor build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    if (AVX2)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    endif()
elseif(ANDROID)
    # Android log library
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -llog -fPIC")
    if (${TARGET_ARCH} STREQUAL "x86" OR ${TARGET_ARCH} STREQUAL "x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4 -mfpmath=sse")
    endif()
elseif(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread -fexceptions -frtti -msse4 -mfpmath=sse -fPIC")
    # Disable @rpath in the install name
    set(CMAKE_MACOSX_RPATH 1)
elseif(LINUX)
    # Linux armhf
    if (${TARGET_ARCH} STREQUAL armhf)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread -fexceptions -frtti -mfpu=neon -fPIC")
    # Linux aarch64
    elseif (${TARGET_ARCH} STREQUAL aarch64)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -pthread -fexceptions -frtti -fPIC")
    # Linux x86_64
    elseif(${TARGET_ARCH} STREQUAL x86_64)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -std=c++11 -pthread -fexceptions -frtti -msse4 -mfpmath=sse -fPIC")
    endif()
endif()
