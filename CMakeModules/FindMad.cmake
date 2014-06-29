#-------------------------------------------------------------------------------
#
# Finds libmad include files and libraries. This module sets the following
# variables:
#
# MAD_FOUND - Flag if libmad was found
# MAD_INCLUDE_DIRS - libmad include directory
# MAD_LIBRARIES - libmad library path
#
#-------------------------------------------------------------------------------

include(FindPackageHandleStandardArgs)

find_path(MAD_INCLUDE_DIRS mad.h)
find_library(MAD_LIBRARIES mad)

find_package_handle_standard_args(
    Mad
    REQUIRED_VARS
    MAD_LIBRARIES
    MAD_INCLUDE_DIRS
)

#-------------------------------------------------------------------------------
