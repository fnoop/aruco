set(sources
    ar_omp.cpp
    cameraparameters.cpp
    checkrectcontour.cpp
    cvdrawingutils.cpp
    dictionary.cpp
    ippe.cpp
    marker.cpp
    markerdetector.cpp
    markerlabeler.cpp
    markermap.cpp
    posetracker.cpp
    markerlabelers/dictionary_based.cpp
    #markerlabelers/svmmarkers.cpp
)

set(headers
    ar_omp.h
    cameraparameters.h
    checkrectcontour.h
    cvdrawingutils.h
    dictionary.h
    ippe.h
    marker.h
    markerdetector.h
    markerlabeler.h
    markermap.h
    posetracker.h
    markerlabelers/dictionary_based.h
    #markerlabelers/svmmarkers.h
)

add_library(aruco ${sources} ${headers})

set_target_properties(aruco PROPERTIES          # create *nix style library versions + symbolic links
    DEFINE_SYMBOL ARUCO_DSO_EXPORTS
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_SOVERSION}
    CXX_VISIBILITY_PRESET hidden                # Make all the symbols hidden, so we have to explicitly define an API
    CLEAN_DIRECT_OUTPUT 1                       # allow creating static and shared libs without conflicts
    OUTPUT_NAME "aruco${PROJECT_DLLVERSION}"    # avoid conflicts between library and binary target names
)

include(GenerateExportHeader)
generate_export_header(aruco)

target_link_libraries(aruco PUBLIC opencv_core)
target_link_libraries(aruco PRIVATE opencv_imgproc opencv_calib3d opencv_features2d)

target_include_directories(aruco PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS aruco EXPORT arucoConfig
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    ARCHIVE DESTINATION lib
)

install(FILES ${headers} ${CMAKE_CURRENT_BINARY_DIR}/aruco_export.h
    DESTINATION include/aruco
)

#install(EXPORT arucoConfig DESTINATION "share/aruco/cmake")
