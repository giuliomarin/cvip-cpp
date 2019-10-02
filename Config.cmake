# Define colors
if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColorReset  "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

#-----------------------
# Log message functions
#-----------------------

# Print message with no color
function(log text)
  message(STATUS ${text})
endfunction(log text)

# Print debug message
function(log_info text)
  message(STATUS "${Green}${text}${ColorReset}")
endfunction(log_info)

# Print warning message
function(log_warning text)
  message(WARNING "${Yellow}${text}${ColorReset}")
endfunction(log_warning)

# Print error message
function(log_error text)
  message(FATAL_ERROR "${BoldRed}${text}${ColorReset}")
endfunction(log_error)

# ----------------
# Get configuration
# ----------------

# Get configuration
if(WIN32)
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(TARGET_ARCH x64)
  else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(TARGET_ARCH Win32)
  endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
elseif(ANDROID)
  if("${ANDROID_ABI}" STREQUAL "armeabi-v7a")
    set(TARGET_ARCH armeabi-v7a)
  elseif("${ANDROID_ABI}" STREQUAL "armeabi-v7a with NEON")
    set(TARGET_ARCH armeabi-v7a)
  elseif("${ANDROID_ABI}" STREQUAL "armeabi-v7a with VFPV3")
    set(TARGET_ARCH armeabi-v7a)
  elseif("${ANDROID_ABI}" STREQUAL "arm64-v8a")
    set(TARGET_ARCH arm64-v8a)
  elseif("${ANDROID_ABI}" STREQUAL "x86")
    set(TARGET_ARCH x86)
  elseif("${ANDROID_ABI}" STREQUAL "x86_64")
    set(TARGET_ARCH x86_64)
  else()
    log_error("Target architecture not supported: ${TARGET_ARCH}")
  endif()
elseif(APPLE)
  set(TARGET_ARCH lib64)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(LINUX 1)
  # ARM v7 (
  if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7l")
    set(TARGET_ARCH armhf)
    # ARM aarch64
  elseif ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64")
    set(TARGET_ARCH aarch64)
    # Intel  x86_64
  elseif("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(TARGET_ARCH x86_64)
    # Intel x86
  elseif("${CMAKE_SIZEOF_VOID_P}" NOT EQUAL "8")
    set(TARGET_ARCH x86)
  endif()
endif()

log_info("Target is ${TARGET_ARCH}")

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -llog -fPIC")
  if (${TARGET_ARCH} STREQUAL "x86" OR ${TARGET_ARCH} STREQUAL "x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4 -mfpmath=sse")
  endif()
elseif(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pthread -fexceptions -frtti -msse4 -mfpmath=sse -fPIC")
  # Disable @rpath in the install name
  set(CMAKE_MACOSX_RPATH 1)
elseif(LINUX)
  # Linux armhf
  if (${TARGET_ARCH} STREQUAL armhf)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pthread -fexceptions -frtti -mfpu=neon -fPIC")
    # Linux aarch64
  elseif (${TARGET_ARCH} STREQUAL aarch64)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pthread -fexceptions -frtti -fPIC")
    # Linux x86_64
  elseif(${TARGET_ARCH} STREQUAL x86_64)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -std=c++17 -pthread -fexceptions -frtti -msse4 -mfpmath=sse -fPIC")
  endif()
endif()
