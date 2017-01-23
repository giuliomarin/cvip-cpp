set(project_name 1_opencvgl)
project(${project_name})

# Inclusion folders
set(proj_path .)

# Add sources to compile exe
file(GLOB_RECURSE project_src_files ${proj_path}/src/*.cpp)
add_executable(${project_name} ${project_src_files})

# -------------------
# Libraries
# -------------------

link_opencv(${project_name})
link_glfw(${project_name})

# Log message
log_info("Included ${project_name}")
