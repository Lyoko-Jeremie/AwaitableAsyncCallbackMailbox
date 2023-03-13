cmake_minimum_required(VERSION 3.25)
set(ProjectMainName AwaitableAsyncCallbackMailbox)
project(${ProjectMainName})

set(CMAKE_CXX_STANDARD 20)

set(DEBUG_BOOST_ASIO OFF CACHE BOOL "DEBUG_BOOST_ASIO")

set(USE_DYNAMIC_BOOST OFF CACHE BOOL "USE_DYNAMIC_BOOST")

set(OpenCV_STATIC ON CACHE BOOL "OpenCV_STATIC")
set(OpenCV_WORLD OFF CACHE BOOL "OpenCV_WORLD")

set(BOOST_ROOT "" CACHE PATH "BOOST_ROOT")

set(OpenCV_DIR "" CACHE PATH "OpenCV_DIR")


if (DEFAULT_CONFIG)
    # ===================== vvvvvv config segment vvvvvv =================================

    #set(OpenCV_DIR d:/CodeLib/opencv-4.5.5-build/)
    set(OpenCV_DIR d:/CodeLib/opencv-4.5.5-build-install/)
    set(OpenCV_STATIC ON)
    set(OpenCV_WORLD ON)

    set(BOOST_ROOT d:/CodeLib/boost_1_81_0_install_x64_v11p3/)
    #set(BOOST_ROOT d:/CodeLib/boost_1_80_0_install_x64_v11p3/)
    #set(Boost_INCLUDE_DIR d:/CodeLib/boost_1_74_0_install_x64_v11p3/include/boost-1_74)
    #set(Boost_LIBRARY_DIR d:/CodeLib/boost_1_74_0_install_x64_v11p3/lib)
    #set(USE_DYNAMIC_BOOST true)
    #set(BOOST_DEBUG ON)

    #add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)

    # config with
    # -DDEBUG_BOOST_ASIO=OFF
    # -DOpenCV_STATIC=OFF
    # -DOpenCV_WORLD=ON
    # -DOpenCV_DIR=h:/CodeLib/opencv-4.5.5-install/
    # -DUSE_DYNAMIC_BOOST=OFF
    # -DBOOST_ROOT=h:/CodeLib/boost_1_81_0_install_x64_v11p3/
    # -DPROTOBUFF_INCLUDE_DIRS=h:/CodeLib/protobuf-cpp-3.21.11/protobuf-3.21.11-install/include/
    # -DPROTOBUFF_LIBS=h:/CodeLib/protobuf-cpp-3.21.11/protobuf-3.21.11-install/lib/libprotobuf.a
    # -DAprilTag_INCLUDE_DIRS=h:/Code/apriltag-build-install/include/apriltag
    # -DAprilTag_LIB=h:/Code/apriltag-build-install/lib/libapriltag.a

    # ===================== ^^^^^^ config segment ^^^^^^ =================================
endif (DEFAULT_CONFIG)

# ======================== boost ==================

if (DEBUG_BOOST_ASIO)
    add_definitions(-DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
endif (DEBUG_BOOST_ASIO)

# https://www.boost.org/doc/libs/1_81_0/libs/assert/doc/html/assert.html
add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)  #

option(USE_DYNAMIC_BOOST "Config USE_DYNAMIC_BOOST")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "enable -O1 when debug to resolve 'string table overflow at offset' 'too big' error on mingw")
    # -O1 to resolve "string table overflow at offset" error on mingw
    add_definitions(-Wall -Wextra -O1 -g)
endif ()

set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#if (NOT_USE_BOOST_THEAD)
#    set(BOOST_THEAD_MODULE "")
#    message(STATUS "NOT_USE_BOOST_THEAD !!!")
#else ()
#    set(BOOST_THEAD_MODULE "thread")
#    add_definitions(-DUSE_BOOST_THEAD)
#endif ()
set(BOOST_THEAD_MODULE "thread")
add_definitions(-DUSE_BOOST_THEAD)


if (USE_DYNAMIC_BOOST)
    message(STATUS "USE DYNAMIC BOOST !!!")
    add_definitions(-DBOOST_ALL_DYN_LINK=1)
else (USE_DYNAMIC_BOOST)
    message(STATUS "USE STATIC BOOST !!!")
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME OFF)
endif (USE_DYNAMIC_BOOST)

set(Boost_USE_MULTITHREADED ON)
#find_package(Boost REQUIRED COMPONENTS  REQUIRED)
find_package(Boost 1.81.0 REQUIRED COMPONENTS system filesystem program_options json log log_setup url ${BOOST_THEAD_MODULE} REQUIRED)

# ======================== boost ==================


## ======================== opencv ==================
#if (OpenCV_WORLD)
#    find_package(OpenCV REQUIRED world)
#else (OpenCV_WORLD)
#    find_package(OpenCV REQUIRED core imgcodecs aruco)
#endif (OpenCV_WORLD)
## ======================== opencv ==================
#
#message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
#message(STATUS "OpenCV_LIBRARY_DIRS: ${OpenCV_LIBRARY_DIRS}")
#message(STATUS "OpenCV_LIBRARIES: ${OpenCV_LIBRARIES}")
#message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}")
#message(STATUS "OpenCV ARCH: ${OpenCV_ARCH}")
#message(STATUS "OpenCV RUNTIME: ${OpenCV_RUNTIME}")
#message(STATUS "OpenCV STATIC: ${OpenCV_STATIC}")

message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
message(STATUS "Boost_VERSION: ${Boost_VERSION}")

add_executable(${ProjectMainName}
        src/main.cpp
        src/MemoryBoost.h
        src/OwlLog/OwlLog.h
        src/OwlLog/OwlLog.cpp
        src/OwlLog/BoostAssertionFailed.cpp
        src/AsyncCallbackMailbox/AsyncCallbackMailbox.h
        )

include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${ProjectMainName} ${Boost_LIBRARIES} ${OpenCV_LIBRARIES})

