configure_file(cmake/arucoConfig.cmake.in arucoConfig.cmake IMMEDIATE @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/arucoConfig.cmake" DESTINATION share/aruco/cmake)
install(FILES "${PROJECT_SOURCE_DIR}/cmake/arucoConfig-release.cmake" DESTINATION share/aruco/cmake)
install(FILES "${PROJECT_SOURCE_DIR}/cmake/arucoConfig-debug.cmake" DESTINATION share/aruco/cmake)

# pkg-config
configure_file(${PROJECT_SOURCE_DIR}/cmake/aruco.pc.in aruco.pc @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/aruco-uninstalled.pc.in aruco-uninstalled.pc @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/aruco-uninstalled.pc" "${PROJECT_BINARY_DIR}/aruco.pc" DESTINATION lib/pkgconfig)

install(FILES "${PROJECT_SOURCE_DIR}/utils_calibration/aruco_calibration_board_a4.pdf"   "${PROJECT_SOURCE_DIR}/utils_calibration/aruco_calibration_board_a4.yml" DESTINATION ${PROJECT_BINARY_DIR}/utils_calibration)
