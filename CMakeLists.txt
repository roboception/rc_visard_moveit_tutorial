cmake_minimum_required(VERSION 2.8.12)

project(rc_visard_moveit_tutorial)
include(cmake/project_version.cmake)

# - Configuration for code optimization -

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build type: DEBUG or RELEASE" FORCE)
endif ()

include(cmake/optimization_flags.cmake)

# - Standard definitions -

add_definitions(-Wall)
if (CMAKE_MAJOR_VERSION VERSION_LESS "3.1.0")
  add_definitions(-std=c++11)
else ()
  set(CMAKE_CXX_STANDARD 11)
endif ()


find_package(catkin REQUIRED
  COMPONENTS
    interactive_markers
    moveit_core
    moveit_visual_tools
    moveit_ros_planning
    moveit_ros_planning_interface
    moveit_ros_perception
    geometric_shapes
    pcl_ros
    rc_pick_client
)
find_package(Boost REQUIRED system filesystem date_time thread)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
        LIBRARIES
        interactivity_utils
        INCLUDE_DIRS
        CATKIN_DEPENDS
        moveit_core
        moveit_visual_tools
        moveit_ros_planning_interface
        interactive_markers
        rc_pick_client
)

###########
## Build ##
###########

include_directories(
  ${catkin_INCLUDE_DIRS}
)


## Declare a cpp executable
add_executable(rc_visard_moveit_tutorial_node src/grasp_trajectory_planning.cpp)

## Add cmake target dependencies of the executable/library
## as an example, message headers may need to be generated before nodes

## Specify libraries to link a library or executable target against
target_link_libraries(rc_visard_moveit_tutorial_node
   ${catkin_LIBRARIES}
)

#############
## Install ##
#############

# Mark executables and/or libraries for installation
install(TARGETS rc_visard_moveit_tutorial_node
   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
 )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_logging.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)

######################################
## Define information for packaging ##
######################################
# optionally specify dependencies of the debian package here (comma separated!)
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "cvkit (>= 1.7.0), ros-$ENV{ROS_DISTRO}-rc-imagecommon")
include(cmake/package_debian.cmake)
