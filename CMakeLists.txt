cmake_minimum_required(VERSION 3.10)
project(ISPMeetings)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(include handlers/include ${CONAN_INCLUDE_DIRS})
add_subdirectory(handlers)

add_executable(${PROJECT_NAME} main.cpp src/server.cpp src/logger.cpp $<TARGET_OBJECTS:handlers>)
target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

enable_testing()
add_executable(meeting_test	unittest/test.cpp)
target_link_libraries(meeting_test ${CONAN_LIBS})
add_test(NAME meeting_test COMMAND meeting_test)

add_executable(from unittest/from_json.cpp)
target_link_libraries(from ${CONAN_LIBS})
add_test(NAME from COMMAND from)

file(COPY schema.sql DESTINATION etc)