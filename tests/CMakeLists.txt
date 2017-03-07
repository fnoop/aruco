link_libraries(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )

add_executable(create_artag_hrm_dict create_artag_hrm_dict.cpp)
add_executable(create_aruco_dict create_aruco_dict.cpp )
add_executable(mset2dict mset2dict.cpp highlyreliablemarkers.h highlyreliablemarkers.cpp)
add_executable(image2contourfile image2contourfile.cpp)
add_executable(extract_chilitag_dict extract_chilitag_dict.cpp)
add_executable(compute_dist_distance compute_dist_distance.cpp)

target_link_libraries(create_artag_hrm_dict     aruco)
target_link_libraries(create_aruco_dict         aruco)
target_link_libraries(mset2dict                 aruco opencv_imgproc)
target_link_libraries(image2contourfile         aruco opencv_highgui opencv_imgproc)
target_link_libraries(extract_chilitag_dict     aruco opencv_highgui )
target_link_libraries(compute_dist_distance     aruco)

if (OpenCV_VERSION VERSION_GREATER "3.0.0")
    target_link_libraries(image2contourfile     opencv_imgcodecs)
    target_link_libraries(extract_chilitag_dict opencv_imgcodecs)
endif()

#add_executable(aruco_test_calibrationboard aruco_test_calibrationboard.cpp)


