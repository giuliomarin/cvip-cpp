#-----------------------------------------------------------
# Find target architecture and set TARGET_ARCH to:
# - "Win32" or "x86" on Windows
# - "lib" or "lib64" on Linux
# - "armeabi-v7a", "arm64-v8a", "x86" or "x86_64" on Android
#-----------------------------------------------------------

# Additional inclusions
include(cmake/Log.cmake)

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
