cmake_minimum_required(VERSION 3.1.0)

project(orbbec_camera_imu)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

INCLUDE_DIRECTORIES(include src)

set(ORBBEC_HPP
    include/h/advanced_mode_command.h
    include/h/context.h
    include/h/device.h
    include/h/frame.h
    include/h/internal.h
    include/h/option
    include/h/pipeline.h
    include/h/processing.h
    include/h/record_playback.h
    include/h/sensor.h
    include/h/types.h

    include/hpp/context.hpp
    include/hpp/frame.hpp
    include/hpp/types.hpp

    include/orbbec.h
    include/orbbec.hpp)
#include_directories(include)
add_library(orbbec-lib
            SHARED
            ${ORBBEC_HPP})


