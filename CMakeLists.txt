project(cvip-cpp)
cmake_minimum_required(VERSION 2.8.7)

# additional cmake files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include additional scripts
include(cmake/Config.cmake)
include(cmake/DepsUtils.cmake)

# turn on using solution folders
set_property( GLOBAL PROPERTY USE_FOLDERS ON)

# Default install folder
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")

# ----------------
# Apps
# ----------------

set(APPS 0_test 1_opencvgl)

FOREACH(app ${APPS})
    add_subdirectory(${app})
    set_property(TARGET ${app} PROPERTY FOLDER "apps")
ENDFOREACH(app)
