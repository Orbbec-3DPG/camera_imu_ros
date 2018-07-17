cmake_minimum_required(VERSION 2.8)

project(orbbec_camera_imu)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  sensor_msgs
  std_msgs
  geometry_msgs
)

catkin_package(
  CATKIN_DEPENDS roscpp sensor_msgs std_msgs geometry_msgs
)

include_directories(
	include
  ${catkin_INCLUDE_DIRS}
)


find_package (Threads)
find_package(OpenCV REQUIRED)
add_executable(orbbec_camera_imu_node src/camera_imu_utils.cpp src/imu_filter.cpp)
add_dependencies( orbbec_camera_imu_node  ${catkin_EXPORTED_TARGETS} )
target_link_libraries(orbbec_camera_imu_node ${OpenCV_LIBS}    ${catkin_LIBRARIES}
pthread)

#add_executable(imu_node src/imu.cpp src/imu_filter.cpp)
#add_dependencies( imu_node  ${catkin_EXPORTED_TARGETS} )
#target_link_libraries(imu_node   ${catkin_LIBRARIES})
