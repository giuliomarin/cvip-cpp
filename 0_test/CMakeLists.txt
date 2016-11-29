set(project_name 0_test)
project(${project_name})

# Inclusion folders
set(proj_path .)

# Add sources to compile exe
file(GLOB_RECURSE project_src_files ${proj_path}/src/*.cpp)
add_executable(${project_name} ${project_src_files})

# -------------------
# Libraries
# -------------------



# Log message
log_info("Included ${project_name}")
