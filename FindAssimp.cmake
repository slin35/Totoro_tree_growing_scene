#
# Find Assimp
#
# Try to find Assimp : Open Asset Import Library.
# This module defines the following variables:
# - ASSIMP_INCLUDE_DIRS
# - ASSIMP_LIBRARIES
# - ASSIMP_FOUND
#
# The following variables can be set as arguments for the module.
# - ASSIMP_ROOT_DIR : Root library directory of Assimp
#

# Additional modules
include(FindPackageHandleStandardArgs)

if (WIN32)
    # Find include files
    message("Windows is not supported yet")
else()
    # Find include files
    find_path(
            ASSIMP_INCLUDE_DIR
            NAMES assimp/scene.h
            PATHS
            ${CMAKE_CURRENT_LIST_DIR}/assimp/include
            DOC "The directory where assimp/scene.h resides")

    # Find library files
    find_library(
            ASSIMP_LIBRARY
            NAMES assimp
            PATHS
            ${CMAKE_CURRENT_LIST_DIR}/assimp/lib
            DOC "The Assimp library")
endif()

# Handle REQUIRD argument, define *_FOUND variable
find_package_handle_standard_args(Assimp DEFAULT_MSG ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)

# Define GLFW_LIBRARIES and GLFW_INCLUDE_DIRS
if (ASSIMP_FOUND)
    set(ASSIMP_LIBRARIES ${ASSIMP_LIBRARY})
    set(ASSIMP_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY)