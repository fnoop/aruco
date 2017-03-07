#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "aruco" for configuration "Release"
set_property(TARGET aruco APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(aruco PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "opencv_imgproc;opencv_calib3d;opencv_features2d"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libaruco.so.2.0.19"
  IMPORTED_SONAME_RELEASE "libaruco.so.2.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS aruco )
list(APPEND _IMPORT_CHECK_FILES_FOR_aruco "${_IMPORT_PREFIX}/lib/libaruco.so.2.0.19" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
