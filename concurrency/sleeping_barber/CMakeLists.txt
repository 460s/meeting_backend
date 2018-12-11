cmake_minimum_required(VERSION 3.10)
project(sleeping_barber)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g")
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	
endif()

add_executable(${PROJECT_NAME} 
	sleeping_barber.cpp
	barber.cpp
	customer.cpp
	waiting_room.cpp
	)
