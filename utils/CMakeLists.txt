INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
add_executable(aruco_print_dictionary aruco_print_dictionary.cpp)
add_executable(aruco_print_marker aruco_print_marker.cpp)
add_executable(aruco_simple aruco_simple.cpp)
add_executable(aruco_test aruco_test.cpp)
add_executable(aruco_tracker aruco_tracker.cpp)
add_executable(aruco_batch_processing aruco_batch_processing.cpp)

target_link_libraries(aruco_print_dictionary    aruco opencv_calib3d)
target_link_libraries(aruco_print_marker        aruco opencv_calib3d)
target_link_libraries(aruco_simple              aruco opencv_calib3d)
target_link_libraries(aruco_test                aruco opencv_calib3d)
target_link_libraries(aruco_tracker             aruco opencv_calib3d)
target_link_libraries(aruco_batch_processing    aruco opencv_calib3d)

#aruco_create_board
INSTALL(TARGETS aruco_batch_processing aruco_tracker aruco_simple aruco_test aruco_print_marker aruco_print_dictionary       RUNTIME DESTINATION bin)
