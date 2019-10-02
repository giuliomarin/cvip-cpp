project(cvip-cpp)
cmake_minimum_required(VERSION 3.1)

# Include additional scripts
set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cpp-lib" CACHE PATH "Path to the deps folder" FORCE)
include(${DEPS_DIR}/libraries.cmake)
include(Config.cmake)

# turn on using solution folders
set_property( GLOBAL PROPERTY USE_FOLDERS ON)

# ----------------
# Apps
# ----------------

set(APPS 0_test 1_opencvgl)
FOREACH(app ${APPS})
    add_subdirectory(${app})
    set_property(TARGET ${app} PROPERTY FOLDER "apps")
ENDFOREACH(app)
