# pkg-config
configure_file(${PROJECT_SOURCE_DIR}/cmake/aruco.pc.in aruco.pc @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/cmake/aruco-uninstalled.pc.in aruco-uninstalled.pc @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/aruco-uninstalled.pc" "${PROJECT_BINARY_DIR}/aruco.pc" DESTINATION lib/pkgconfig)

CONFIGURE_FILE("${PROJECT_SOURCE_DIR}/cmake/config.cmake.in" "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" DESTINATION share/${PROJECT_NAME} )
if(WIN32)
    SET(PROJECT_DLLVERSION "${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}")
ENDIF()
install(FILES "${PROJECT_SOURCE_DIR}/utils_calibration/aruco_calibration_grid_board_a4.pdf"   "${PROJECT_SOURCE_DIR}/utils_calibration/aruco_calibration_grid_board_a4.yml" DESTINATION ${PROJECT_BINARY_DIR}/utils_calibration)
install(FILES "${PROJECT_SOURCE_DIR}/utils/myown.dict"   DESTINATION ${PROJECT_BINARY_DIR}/utils)
