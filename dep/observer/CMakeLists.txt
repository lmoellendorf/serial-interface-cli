cmake_minimum_required (VERSION 2.8)
project (observer)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -fmessage-length=0")

set (OBSERVER_SOURCES
    ${PROJECT_SOURCE_DIR}/sf_subject.cpp
    ${PROJECT_SOURCE_DIR}/sf_event.cpp
    )

add_library(observer ${OBSERVER_SOURCES})

# Needed for add_subdirectory() in parent projects
target_include_directories(observer PUBLIC ${PROJECT_SOURCE_DIR})

install (TARGETS observer DESTINATION lib)
install (FILES ${PROJECT_SOURCE_DIR}/sf_observer.h
    ${PROJECT_SOURCE_DIR}/sf_subject.h
    DESTINATION include)
