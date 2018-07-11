INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
add_executable(aruco_calibration aruco_calibration.cpp calibrator.h calibrator.cpp )
add_executable(aruco_calibration_fromimages aruco_calibration_fromimages.cpp dirreader.h)

if(CMAKE_COMPILER_IS_GNUCXX OR MINGW OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    target_link_libraries(aruco_calibration             aruco opencv_calib3d pthread)
else()
    target_link_libraries(aruco_calibration             aruco opencv_calib3d)

endif()


target_link_libraries(aruco_calibration_fromimages  aruco opencv_calib3d)

install(TARGETS      aruco_calibration aruco_calibration_fromimages  RUNTIME DESTINATION bin)
