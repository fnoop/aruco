INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
link_libraries(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )

add_executable(test_dummy test_dummy.cpp)
target_link_libraries(test_dummy    aruco  opencv_calib3d)
add_executable(create_artag_hrm_dict create_artag_hrm_dict.cpp)
add_executable(create_aruco_dict create_aruco_dict.cpp )
add_executable(mset2dict mset2dict.cpp highlyreliablemarkers.h highlyreliablemarkers.cpp)
add_executable(image2contourfile image2contourfile.cpp)
add_executable(extract_chilitag_dict extract_chilitag_dict.cpp)
add_executable(compute_dist_distance compute_dist_distance.cpp)
#add_executable(speed_test speed_test.cpp)
add_executable(ippe_ratio ippe_ratio.cpp)

target_link_libraries(create_artag_hrm_dict     aruco)
target_link_libraries(create_aruco_dict         aruco)
target_link_libraries(mset2dict                 aruco opencv_imgproc)
target_link_libraries(image2contourfile         aruco opencv_highgui opencv_imgproc)
target_link_libraries(extract_chilitag_dict     aruco opencv_highgui )
target_link_libraries(compute_dist_distance     aruco)
#target_link_libraries(speed_test aruco opencv_calib3d)
target_link_libraries(ippe_ratio    aruco  opencv_calib3d)
add_executable(test_picoflann test_picoflann.cpp)
target_link_libraries(test_picoflann    aruco  opencv_calib3d)

if (OpenCV_VERSION VERSION_GREATER "3.0.0")
    target_link_libraries(image2contourfile     opencv_imgcodecs)
    target_link_libraries(extract_chilitag_dict opencv_imgcodecs)
endif()
add_executable(test_essential test_essential.cpp sgl.h)

target_link_libraries(test_essential    aruco  opencv_calib3d)

#add_executable(aruco_test_calibrationboard aruco_test_calibrationboard.cpp)


