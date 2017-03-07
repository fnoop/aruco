# ----------------------------------------------------------------------------
#   Find Dependencies
# ----------------------------------------------------------------------------

find_package(OpenCV REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )

message(STATUS "OpenCV found, version: ${OpenCV_VERSION} in dir ${OpenCV_INCLUDE_DIRS}")

if(OpenCV_VERSION VERSION_LESS "3.0")#Opencv 2 requires the ad-hoc levmarq method for tracking since solvePnp from intrinsicguess does not work
	if(NOT USE_OWN_EIGEN3)
        message(WARNING "If you do not want to install Eigen you can turn on the option USE_OWN_EIGEN3")
		find_package( Eigen3 REQUIRED )
	else()
		set(EIGEN3_INCLUDE_DIR "3rdparty/eigen3")
	endif()
	include_directories( ${EIGEN3_INCLUDE_DIR} )

	if(USE_DOUBLE_PRECISION_PNP)
		add_definitions(-DDOUBLE_PRECISION_PNP)
	endif()
endif()

# Check if we have OpenCV 3

if(EXISTS ${GLUT_PATH})
	include_directories(${GLUT_PATH}/include)
	set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${GLUT_PATH}/lib)
endif()

##LOOK FOR OPENGL AND GLUT
#FIND OPENGL LIBRARY. In Android, there is no need since it is included
if(NOT ANDROID_CREATION)
	if(BUILD_GLSAMPLES)
		find_package(OpenGL)
		find_package(GLUT)#standard package
		message(STATUS "GLUT_FOUND=${GLUT_FOUND} OPENGL_gl_LIBRARY=${OPENGL_gl_LIBRARY} GLUT_HEADER=${GLUT_HEADER}")
	endif()

	if(NOT GLUT_FOUND) #else, freeglut
		find_library(GLUT_glut_LIBRARY     NAMES freeglut)
		message(STATUS "GLUT_glut_LIBRARY=${GLUT_glut_LIBRARY}")
	endif()

	if ( (NOT GLUT_glut_LIBRARY AND NOT GLUT_FOUND) OR NOT OPENGL_gl_LIBRARY)
		set(GL_FOUND "NO")
	else()
		set(GL_FOUND "YES")
		set (OPENGL_LIBS  ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLUT_glut_LIBRARY})
	endif()
endif()

#Omp
if(WIN32)

else()
	find_package(OpenMP  )
	if (OPENMP_FOUND)
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
		add_definitions(-DUSE_OMP)
	endif()
endif()
