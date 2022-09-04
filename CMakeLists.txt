cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

project(tinylog C CXX)

enable_testing(())

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include/)

find_package(GTest MODULE REQUIRED)

set(LIB_SRC
    # src/LogCategory.cc
    src/LogLevel.cc
    # src/LogMessage.cc
    src/LogName.cc
)

add_library(${PROJECT_NAME} ${LIB_SRC})

find_library(PTHREAD pthread)

set(LIBS
    ${PROJECT_NAME}
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_executable(loglevel_test src/test/LogLevelTest.cc)
target_link_libraries(loglevel_test ${LIBS})
gtest_discover_tests(loglevel_test)

add_executable(logname_test src/test/LogNameTest.cc)
target_link_libraries(logname_test ${LIBS})
gtest_discover_tests(logname_test)