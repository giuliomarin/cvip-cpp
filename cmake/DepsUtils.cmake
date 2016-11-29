#---------------
# Configuration
#---------------

set(deps_path ${CMAKE_CURRENT_SOURCE_DIR}/../Nitrogen/deps)

#--------
# OpenCV
#--------

set(OPENCV_VERSION 300)

# Include OpenCV headers
function(aq_include_opencv)
    if (WIN32)
        include_directories("${deps_path}/windows/opencv/include")
    elseif(ANDROID)
        include_directories("${deps_path}/android/opencv/${AQ_TARGET_ARCH}/jni/include")
    elseif(APPLE)
        include_directories("${deps_path}/macos/opencv/include")
    elseif(LINUX)
        include_directories("${deps_path}/linux/${AQ_TARGET_ARCH}/opencv/include")
    endif()
endfunction(aq_include_opencv)

# Link OpenCV (i.e. opencv_world)
function(aq_link_opencv project_name)
    # OpenCV world
    if (WIN32)
        _aq_link_opencv(${project_name} calib3d core features2d flann highgui imgcodecs imgproc ml objdetect photo shape stitching superres video videoio videostab)
    elseif(ANDROID)
        _aq_link_opencv(${project_name} calib3d features2d flann highgui imgcodecs video objdetect imgproc core ml photo shape stitching superres videoio videostab)
    elseif(APPLE)
        _aq_link_opencv(${project_name} core flann imgproc ml photo video imgcodecs shape videoio highgui objdetect superres features2d calib3d stitching videostab)
    elseif(LINUX)
        _aq_link_opencv(${project_name} calib3d features2d flann highgui imgcodecs video objdetect imgproc core ml photo shape stitching superres videoio videostab)
    endif()

    # OpenCV contrib modules
    _aq_link_opencv(${project_name} bgsegm)

    # 3rd-party dependencies
    if (WIN32)
        _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf zlib)
    elseif(ANDROID)
        _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf zlib)
        if (${AQ_TARGET_ARCH} STREQUAL "x86")
             _aq_link_opencv_3rdparty(${project_name} ippicv)
        endif()
    elseif(APPLE)
        _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf libjpeg zlib ippicv)
    elseif(LINUX)
        if(${AQ_TARGET_ARCH} STREQUAL armhf)
            _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf zlib)
        elseif(${AQ_TARGET_ARCH} STREQUAL aarch64)
            _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf zlib)
        elseif(${AQ_TARGET_ARCH} STREQUAL x86_64)
            _aq_link_opencv_3rdparty(${project_name} libjasper libpng libtiff libwebp IlmImf zlib ippicv)
        endif()
    endif()

    # Other platform-specific dependencies
    if (WIN32)
        target_link_libraries(${project_name}
            Comctl32.lib
            ${deps_path}/windows/opencv/lib/${AQ_TARGET_ARCH}/ippicvmt.lib
                debug ${deps_path}/windows/tbb/lib/${AQ_TARGET_ARCH}/tbb_debug.lib
                optimized ${deps_path}/windows/tbb/lib/${AQ_TARGET_ARCH}/tbb.lib
                debug ${deps_path}/windows/libjpeg/lib/${AQ_TARGET_ARCH}/libjpegd.lib
                optimized ${deps_path}/windows/libjpeg/lib/${AQ_TARGET_ARCH}/libjpeg.lib)
    elseif(ANDROID)
        target_link_libraries(${project_name}
            ${deps_path}/android/tbb/lib/${AQ_TARGET_ARCH}/libtbb.a
            ${deps_path}/android/libjpeg/lib/${AQ_TARGET_ARCH}/liblibjpeg.a)
    elseif(APPLE)
        target_link_libraries(${project_name} ${deps_path}/macos/tbb/${AQ_TARGET_ARCH}/libtbb.a)
        # link Cocoa AppKit QuartzCore QTKit and OpenCL frameworks
        FIND_LIBRARY(FRAMEWORK_COCOA Cocoa)
        FIND_LIBRARY(FRAMEWORK_APPKIT AppKit )
        FIND_LIBRARY(FRAMEWORK_QUARTZCORE QuartzCore)
        FIND_LIBRARY(FRAMEWORK_QTKIT QTKit )
        FIND_LIBRARY(FRAMEWORK_OPENCL OpenCL )
        MARK_AS_ADVANCED (FRAMEWORK_COCOA FRAMEWORK_APPKIT FRAMEWORK_QUARTZCORE FRAMEWORK_QTKIT FRAMEWORK_OPENCL)
        SET(EXTRA_LIBS ${FRAMEWORK_COCOA} ${FRAMEWORK_APPKIT} ${FRAMEWORK_QUARTZCORE} ${FRAMEWORK_QTKIT} ${FRAMEWORK_OPENCL})
        target_link_libraries(${project_name} ${EXTRA_LIBS})
    elseif(LINUX)
        # For aarch64 the libraries are expected to be provided as part of the toolchain sysroot
        if (${AQ_TARGET_ARCH} STREQUAL aarch64)
            target_link_libraries(${project_name} dl z)
            target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtCore.so)
            target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtGui.so)
            target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtTest.so)
        else()
            target_link_libraries(${project_name} dl QtCore QtGui QtTest)
        endif()
        target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/tbb/lib/libtbb.a
            ${deps_path}/linux/${AQ_TARGET_ARCH}/libjpeg/lib/liblibjpeg.a)
    endif()
endfunction(aq_link_opencv)

# Link OpenCV modules (e.g. core highgui etc.)
function(_aq_link_opencv project_name)
    if (WIN32)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                optimized ${deps_path}/windows/opencv/lib/${AQ_TARGET_ARCH}/opencv_${module_name}${OPENCV_VERSION}.lib
                debug ${deps_path}/windows/opencv/lib/${AQ_TARGET_ARCH}/opencv_${module_name}${OPENCV_VERSION}d.lib)
        endforeach()
    elseif(ANDROID)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/android/opencv/${AQ_TARGET_ARCH}/libs/${AQ_TARGET_ARCH}/libopencv_${module_name}.a)
        endforeach()
    elseif(APPLE)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/macos/opencv/${AQ_TARGET_ARCH}/libopencv_${module_name}.a)
        endforeach()
    elseif(LINUX)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/opencv/lib/libopencv_${module_name}.a)
        endforeach()
    endif()
endfunction(_aq_link_opencv)

# Link OpenCV 3rd party modules (e.g. libjpeg, libippicv)
function(_aq_link_opencv_3rdparty project_name)
    if (WIN32)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                debug ${deps_path}/windows/opencv/lib/${AQ_TARGET_ARCH}/${module_name}d.lib
        optimized ${deps_path}/windows/opencv/lib/${AQ_TARGET_ARCH}/${module_name}.lib)
        endforeach()
    elseif(ANDROID)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/android/opencv/${AQ_TARGET_ARCH}/3rdparty/libs/${AQ_TARGET_ARCH}/lib${module_name}.a)
        endforeach()
    elseif(APPLE)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/macos/opencv/share/OpenCV/3rdparty/${AQ_TARGET_ARCH}/lib${module_name}.a)
        endforeach()
    elseif(LINUX)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/opencv/share/OpenCV/3rdparty/lib/lib${module_name}.a)
        endforeach()
    endif()
endfunction(_aq_link_opencv_3rdparty)

#--------
# Boost
#--------

set(BOOST_VERSION 1_55)

# Include boost headers
function(aq_include_boost)
  if(APPLE)
    file(GLOB Boost_libfile "${deps_path}/boost/macos/${AQ_TARGET_ARCH}/*.a")
    set(Boost_LIBRARIES ${Boost_libfile} CACHE INTERNAL "Boost libraries")
  endif()
  set(Boost_INCLUDE_DIR "${deps_path}/boost" CACHE INTERNAL "Boost include")
  set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR} CACHE INTERNAL "Boost include")
  set(Boost_FOUND TRUE CACHE INTERNAL "Boost found")
  include_directories(${Boost_INCLUDE_DIRS})
endfunction(aq_include_boost)

# Link Boost
function(aq_link_boost project_name)
    _aq_link_boost(${project_name} thread system date_time chrono iostreams filesystem)
endfunction(aq_link_boost)

# Link Boost modules (e.g. thread etc.)
function(_aq_link_boost project_name)
    if (WIN32)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                optimized ${deps_path}/boost/windows/lib/${AQ_TARGET_ARCH}/libboost_${module_name}-vc120-mt-${BOOST_VERSION}.lib
                debug ${deps_path}/boost/windows/lib/${AQ_TARGET_ARCH}/libboost_${module_name}-vc120-mt-gd-${BOOST_VERSION}.lib)
        endforeach()
    elseif (ANDROID)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                ${deps_path}/boost/android/lib/libboost_${module_name}-gcc49-mt-1_55.a)
        endforeach()
    elseif (LINUX)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                ${deps_path}/boost/linux/${AQ_TARGET_ARCH}/libboost_${module_name}.a)
        endforeach()
    elseif (APPLE)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
            ${deps_path}/boost/macos/${AQ_TARGET_ARCH}/libboost_${module_name}.a)
        endforeach()
    else()
        aq_log_error("Linking Boost not supported for this platform.")
    endif()
endfunction(_aq_link_boost)

#--------
# Eigen
#--------

# Include Eigen headers
function(aq_include_eigen)
  set(EIGEN_INCLUDE_DIRS "${deps_path}/eigen/include/eigen3" CACHE INTERNAL "Eigen include")
  set(EIGEN_INCLUDE_DIR ${EIGEN_INCLUDE_DIRS} CACHE INTERNAL "Eigen include")
  set(EIGEN_FOUND TRUE CACHE INTERNAL "Eigen found")
  include_directories(${EIGEN_INCLUDE_DIRS})
endfunction(aq_include_eigen)

#--------
# ANN
#--------

# Include ANN headers
function(aq_include_ann)
	include_directories("${deps_path}/ann/include")
endfunction(aq_include_ann)

#-----
# TBB
#-----

# Include TBB headers and lib folder
function(aq_include_tbb)
    if (WIN32)
    elseif(ANDROID)
        include_directories("${deps_path}/android/tbb/include")
        link_directories("${deps_path}/android/tbb/${AQ_TARGET_ARCH}")
    elseif(APPLE)
        include_directories("${deps_path}/macos/tbb/include")
    elseif(LINUX)
        include_directories("${deps_path}/linux/${AQ_TARGET_ARCH}/tbb/include")
        link_directories("${deps_path}/linux/${AQ_TARGET_ARCH}/tbb/lib")
    endif()
endfunction(aq_include_tbb)

# Link TBB
function(aq_link_tbb project_name)
    if (WIN32)
    elseif(ANDROID)
        target_link_libraries(${project_name} ${deps_path}/android/tbb/lib/${AQ_TARGET_ARCH}/libtbb.a)
    elseif(APPLE)
        target_link_libraries(${project_name} ${deps_path}/macos/tbb/${AQ_TARGET_ARCH}/libtbb.a)
    elseif(LINUX)
        target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/tbb/lib/libtbb.a)
    endif()
endfunction(aq_link_tbb)

#--------
# PCL
#--------

# Include PCL headers
function(aq_include_pcl)
    if (WIN32)
        include_directories(${deps_path}/windows/pcl/include/pcl-1.7)
    elseif(ANDROID)
        include_directories(${deps_path}/android/pcl/include/)
    elseif(APPLE)
        include_directories(${deps_path}/macos/pcl/include/pcl-1.7)
    else()
        include_directories(${deps_path}/linux/${AQ_TARGET_ARCH}/pcl/include/pcl-1.7)
    endif()
endfunction(aq_include_pcl)

# Link PCL
function(aq_link_pcl project_name)
    _aq_link_pcl(${project_name} filters io io_ply octree sample_consensus search kdtree surface common)
endfunction(aq_link_pcl)

# Link PCL modules
function(_aq_link_pcl project_name)
    if (WIN32)
        set(pcl_libs_path ${deps_path}/windows/pcl/lib/${AQ_TARGET_ARCH})
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name}
                optimized ${pcl_libs_path}/pcl_${module_name}_release.lib
                debug ${pcl_libs_path}/pcl_${module_name}_debug.lib)
            list(APPEND pcl_debug_dlls ${pcl_libs_path}/pcl_${module_name}_debug.dll)
            list(APPEND pcl_release_dlls ${pcl_libs_path}/pcl_${module_name}_release.dll)
        endforeach()
        file(COPY ${pcl_release_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
        file(COPY ${pcl_release_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo)
        file(COPY ${pcl_debug_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
        # copy vtk libs also
        file(GLOB_RECURSE vtk_dlls ${deps_path}/windows/vtk/lib/x64/vtk*.dll)
        file(COPY ${vtk_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
        file(COPY ${vtk_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo)
        file(COPY ${vtk_dlls} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
    elseif(ANDROID)
        aq_log_warning("Linking Android ARM PCL for now.")
        set(pcl_libs_path ${deps_path}/android/pcl/lib)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${pcl_libs_path}/libpcl_${module_name}.a)
        endforeach()

    # link openmp
        target_link_libraries(${project_name} gomp)
    elseif(LINUX)
        set(pcl_libs_path ${deps_path}/linux/${AQ_TARGET_ARCH}/pcl/lib)
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${pcl_libs_path}/libpcl_${module_name}.a)
        endforeach()
        # link openmp
        target_link_libraries(${project_name} gomp)
  elseif(APPLE)
        set(pcl_libs_path ${deps_path}/macos/pcl/${AQ_TARGET_ARCH})
        foreach(module_name IN LISTS ARGN)
            target_link_libraries(${project_name} ${pcl_libs_path}/libpcl_${module_name}.a)
        endforeach()
    endif()
endfunction(_aq_link_pcl)

#--------
# FLANN
#--------

# Include FLANN headers
function(aq_include_flann)
    if (WIN32)
      set(FLANN_INCLUDE_directories "${deps_path}/windows/flann/include")
    elseif(ANDROID)
      set(FLANN_INCLUDE_directories "${deps_path}/android/flann/include")
    elseif(APPLE)
      set(FLANN_INCLUDE_directories "${deps_path}/macos/flann/include")
    else()
      set(FLANN_INCLUDE_directories "${deps_path}/linux/${AQ_TARGET_ARCH}/flann/include")
    endif()
    set(FLANN_INCLUDE_DIRS ${FLANN_INCLUDE_directories} CACHE INTERNAL "Flann include")
    set(FLANN_INCLUDE_DIR ${FLANN_INCLUDE_DIRS} CACHE INTERNAL "Flann include")
    if(APPLE)
      set(FLANN_LIBRARIES "${deps_path}/macos/flann/lib64/libflann_s.a" CACHE INTERNAL "Flann libraries")
      set(FLANN_LIBRARY ${FLANN_LIBRARIES} CACHE INTERNAL "Flann libraries")
    endif()
    set(FLANN_FOUND TRUE CACHE INTERNAL "Flann found")
    include_directories(${FLANN_INCLUDE_DIRS})
endfunction(aq_include_flann)

#------------
# GoogleTest
#------------

# Include Google Test headers
function(aq_include_googletest)
    if (WIN32)
        include_directories("${deps_path}/windows/googletest/include")
    elseif(ANDROID)
        include_directories("${deps_path}/android/googletest/include")
    elseif(APPLE)
        include_directories("${deps_path}/macos/googletest/include")
    elseif(LINUX)
        include_directories("${deps_path}/linux/${AQ_TARGET_ARCH}/googletest/include")
    endif()
endfunction(aq_include_googletest)

# Link Google Test
function(aq_link_googletest project_name)
    if (WIN32)
    target_link_libraries(${project_name}
      optimized ${deps_path}/windows/googletest/lib/${AQ_TARGET_ARCH}/Release/gtest.lib
      debug ${deps_path}/windows/googletest/lib/${AQ_TARGET_ARCH}/Debug/gtest.lib)
    elseif(ANDROID)
        target_link_libraries(${project_name} ${deps_path}/android/googletest/libs/${AQ_TARGET_ARCH}/libgtest.a)
    elseif(APPLE)
        target_link_libraries(${project_name} ${deps_path}/macos/googletest/lib64/libgtest.a)
    elseif(LINUX)
        target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/googletest/lib/libgtest.a)
    endif()
endfunction(aq_link_googletest)


#------------
# GL Libs
#------------

# Include GL libs
function(aq_include_gllibs)
  include_directories("${deps_path}/glm")
  if (WIN32)
    include_directories("${deps_path}/windows/glew/include")
    set(gl_folder_glew ${deps_path}/windows/glew/bin/Release/${AQ_TARGET_ARCH})
    set(gl_files  ${gl_folder_glew}/glew32.dll )
    include_directories("${deps_path}/windows/freeglut/include")
    set(gl_folder_glut ${deps_path}/windows/freeglut/bin/${AQ_TARGET_ARCH} )
    set(gl_files  ${gl_files} ${gl_folder_glut}/freeglut.dll )
    file(COPY ${gl_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    file(COPY ${gl_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo)
    file(COPY ${gl_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  elseif(APPLE)
    include_directories("${deps_path}/macos/glew/include")
    include_directories("${deps_path}/macos/freeglut/include")
  elseif(ANDROID)
# no deps required
  elseif(LINUX)
   # For Linux aarch64 (e.g. S900) the inclusions are expected to be part of sysroot
   if (NOT ${AQ_TARGET_ARCH} STREQUAL aarch64)
    # use default dir for glew from (apt-get install libglew-dev)
    include_directories("/usr/include/")
   endif()
# no deps required
    endif()
endfunction(aq_include_gllibs)

# Link GL libs
function(aq_link_gllibs project_name)
  if (WIN32)
    target_link_libraries(${project_name} ${deps_path}/windows/glew/lib/Release/${AQ_TARGET_ARCH}/glew32.lib)
    target_link_libraries(${project_name} ${deps_path}/windows/glew/lib/Release/${AQ_TARGET_ARCH}/glew32s.lib)
    target_link_libraries(${project_name} ${deps_path}/windows/freeglut/lib/${AQ_TARGET_ARCH}/freeglut.lib)
  elseif(APPLE)
    target_link_libraries(${project_name} ${deps_path}/macos/glew/lib/libGLEW.a)
    if (AQ_VIZ3D)
      target_link_libraries(${project_name} ${deps_path}/macos/freeglut/${AQ_TARGET_ARCH}/libfreeglut.a
        /opt/X11/lib/libGL.dylib
        /opt/X11/lib/libXrandr.dylib
        /opt/X11/lib/libXxf86vm.dylib
        /opt/X11/lib/libglut.dylib
        /opt/X11/lib/libX11.dylib)
    else()
      FIND_LIBRARY(FRAMEWORK_GLUT GLUT)
      MARK_AS_ADVANCED (FRAMEWORK_GLUT)
      FIND_LIBRARY(FRAMEWORK_OPENGL OPENGL)
      MARK_AS_ADVANCED (FRAMEWORK_OPENGL)
      target_link_libraries(${project_name} ${FRAMEWORK_GLUT} ${FRAMEWORK_OPENGL})
  endif()
  elseif(ANDROID)
    #Android doesn't need this library
  elseif(LINUX)
    # For Linux aarch64 (e.g. S900) the inclusions are expected to be part of sysroot
    if (${AQ_TARGET_ARCH} STREQUAL aarch64)
        target_link_libraries(${project_name} GLEW GL GLU glut)
    else()
        #Link Linux GL
        target_link_libraries(${project_name} /usr/lib/${AQ_TARGET_ARCH}-linux-gnu/libGLEW.so)
        target_link_libraries(${project_name} GL GLU glut)
    endif()
  else()
    #Application based on this library are not supported for other platforms
    aq_log_error("GL Libs not supported in this platform")
  endif()
endfunction(aq_link_gllibs)

#--------
# Qt
#--------
set(deps_path_qt ${NITROGEN_ROOT}/deps)
# Include Qt headers
function(aq_include_qt)
  if (WIN32)
    include_directories("${deps_path_qt}/windows/qt/include")
    include_directories("${deps_path_qt}/windows/qt/include/QtNetwork")
    include_directories("${deps_path_qt}/windows/qt/include/QtOpenGL")
    include_directories("${deps_path_qt}/windows/qt/include/QtTest")
    include_directories("${deps_path_qt}/windows/qt/include/QtGui")
    include_directories("${deps_path_qt}/windows/qt/include/QtCore")

    set(qt_folder ${deps_path_qt}/windows/qt/bin)
    set(qt_files  ${qt_folder}/QtNetwork4.dll ${qt_folder}/QtOpenGL4.dll ${qt_folder}/QtTest4.dll ${qt_folder}/QtGui4.dll ${qt_folder}/QtCore4.dll)
    file(COPY ${qt_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release)
    file(COPY ${qt_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo)
    file(COPY ${qt_files} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
  elseif(APPLE)
    include_directories("${deps_path_qt}/macos/qt/include")
    include_directories("${deps_path_qt}/macos/qt/src")
    include_directories("${deps_path_qt}/macos/qt/include/QtNetwork")
    include_directories("${deps_path_qt}/macos/qt/include/QtOpenGL")
    include_directories("${deps_path_qt}/macos/qt/include/QtTest")
    include_directories("${deps_path_qt}/macos/qt/include/QtGui")
    include_directories("${deps_path_qt}/macos/qt/include/QtCore")
    include_directories("${deps_path_qt}/macos/qt/include/QtWidgets")
   elseif(LINUX)
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/src")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include/QtNetwork")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include/QtOpenGL")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include/QtTest")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include/QtGui")
    include_directories("${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/include/QtCore")
  else()
    aq_log_error("QT not supported for this platform")
  endif()
endfunction(aq_include_qt)


function(aq_link_qt)
  if (WIN32)
    target_link_libraries(${project_name} ${deps_path_qt}/windows/qt/lib/QtGui4.lib)
    target_link_libraries(${project_name} ${deps_path_qt}/windows/qt/lib/qtmain.lib)
    target_link_libraries(${project_name} ${deps_path_qt}/windows/qt/lib/QtCore4.lib)
    target_link_libraries(${project_name} ${deps_path_qt}/windows/qt/lib/QtTest4.lib)
    target_link_libraries(${project_name} ${deps_path_qt}/windows/qt/lib/QtOpenGL4.lib)
    target_link_libraries(${project_name} OpenGL32.Lib glu32.lib)

  elseif(APPLE)
    # link QtCore QtGui QtOpenGL QtTest QtWidgets
    set(qt_folder ${deps_path_qt}/macos/qt/lib/)
    set(qt_files  ${qt_folder}/QtCore.framework ${qt_folder}/QtGui.framework ${qt_folder}/QtOpenGL.framework ${qt_folder}/QtTest.framework ${qt_folder}/QtWidgets.framework)
    target_link_libraries(${project_name} ${qt_files})

    # link OpenGL framework
    FIND_LIBRARY(FRAMEWORK_OPENGL OpenGL)
    MARK_AS_ADVANCED (FRAMEWORK_OPENGL)
    target_link_libraries(${project_name} ${FRAMEWORK_OPENGL})
  elseif(LINUX)
    target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtGui.so)
    target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtCore.so)
    target_link_libraries(${project_name} ${deps_path_qt}/linux/${AQ_TARGET_ARCH}/qt/lib/libQtOpenGL.so)
    target_link_libraries(${project_name} GL GLU glut)
  else()
    aq_log_error("QT not supported for this platform")
  endif()
endfunction(aq_link_qt)

#--------
# Qt
# 1. find path to QT and cache it
#--------
function(aq_find_qt)
    if (WIN32)
        set(QTDIR "${deps_path}/windows/qt" CACHE PATH "QT Location")
    elseif(UNIX)
        set(QTDIR "${deps_path}/linux/qt" CACHE PATH "QT Location")
    else()
      aq_log_error("QT not supported for this platform")
    endif()
endfunction(aq_find_qt)

#------------
# OpenSSL
#------------

# Include OpenSSL headers
function(aq_include_openssl)
    if (WIN32)
    elseif(ANDROID)
        include_directories("${deps_path}/android/openssl/${AQ_TARGET_ARCH}/include")
    elseif(APPLE)
        include_directories("${deps_path}/linux/x86_64/openssl/include")
    elseif(LINUX)
        include_directories("${deps_path}/linux/${AQ_TARGET_ARCH}/openssl/include")
    endif()
endfunction(aq_include_openssl)

# Link OpenSSL
function(aq_link_openssl project_name)
    if (WIN32)
    elseif(ANDROID)
        target_link_libraries(${project_name} ${deps_path}/android/openssl/${AQ_TARGET_ARCH}/lib/libcrypto.a dl)
    elseif(APPLE)
        FIND_LIBRARY(APPLE_LIBCRYPTO libcrypto.dylib)
        target_link_libraries(${project_name} ${APPLE_LIBCRYPTO})
    elseif(LINUX)
        if (${AQ_TARGET_ARCH} STREQUAL x86_64)
            target_link_libraries(${project_name} ${deps_path}/linux/${AQ_TARGET_ARCH}/openssl/lib/libcrypto.a dl)
        endif()
    endif()
endfunction(aq_link_openssl)

#--------
# OpenCL
#--------

# include opencl
function(aq_include_opencl)
    if (ANDROID)
        include_directories("${deps_path}/opencl/include/${AQ_OPENCL_VERSION}")
    elseif(LINUX AND ${AQ_TARGET_ARCH} STREQUAL aarch64)
        include_directories("${deps_path}/opencl/include/${AQ_OPENCL_VERSION}")
    else()
        if (OpenCL_FOUND)
          include_directories(${OpenCL_INCLUDE_DIR})
        endif()
  endif()
endfunction(aq_include_opencl)

# link opencl
function(aq_link_opencl project_name)
    if (ANDROID)
        target_link_libraries(${project_name} ${deps_path}/opencl/lib/${AQ_TARGET_ARCH}/libOpenCL.a)
    elseif(LINUX AND "${AQ_TARGET_ARCH}" STREQUAL "aarch64")
        target_link_libraries(${project_name} libPVROCL.so)
    else()
        if (OpenCL_FOUND)
            target_link_libraries(${project_name} ${OpenCL_LIBRARY})
        endif()
    endif()
endfunction(aq_link_opencl)

#--------
# zippp
#--------

# include zipios
function(aq_include_zippp)
    if (ANDROID)
        include_directories("${deps_path}/android/libzippp/jni")
        include_directories("${deps_path}/android/libzippp/jni/zippp")
    endif()
endfunction(aq_include_zippp)

function(aq_link_zippp project_name)
    if (ANDROID)
        # Add zlib
        target_link_libraries(${project_name} z)
        target_link_libraries(${project_name} debug ${deps_path}/android/libzippp/libs/${AQ_TARGET_ARCH}/libzipppd.a
            optimized ${deps_path}/android/libzippp/libs/${AQ_TARGET_ARCH}/libzippp.a)
    endif()
endfunction(aq_link_zippp)
