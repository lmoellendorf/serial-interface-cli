cmake_minimum_required (VERSION 2.6)
project (serial-interface-gunit)

set (COMMON_COMPILER_FLAGS "")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILER_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x ${COMMON_COMPILER_FLAGS}")

#if (WIN32)
#    MESSAGE(STATUS "On Windows")
#    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
#endif()

# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}/src_under_test/")
include_directories("${PROJECT_SOURCE_DIR}/mocks/")
include_directories("${PROJECT_SOURCE_DIR}/tests/")
include_directories("${PROJECT_SOURCE_DIR}/gmock/include/")
include_directories("${PROJECT_SOURCE_DIR}/gmock/gtest/include/")

add_subdirectory(${PROJECT_SOURCE_DIR}/gmock ${PROJECT_BINARY_DIR}/gmock)

add_definitions (-D_cplusplus -DGTEST_HAS_PTHREAD)

set (SOURCES
    ${PROJECT_SOURCE_DIR}/tests/test
    ${PROJECT_SOURCE_DIR}/tests/test_sample
    ${PROJECT_SOURCE_DIR}/tests/printHex
    ${PROJECT_SOURCE_DIR}/src_under_test/foobar
    ${PROJECT_SOURCE_DIR}/mocks/mocked_foobar_extern
    )


add_executable(stackforce-serial-interface-gunit ${SOURCES})

# This together with ${CMAKE_THREAD_LIBS_INIT} is a portable way to link pthread
find_package ( Threads REQUIRED )
target_link_libraries (stackforce-serial-interface-gunit gtest gmock
    ${CMAKE_THREAD_LIBS_INIT})

