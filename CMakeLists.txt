cmake_minimum_required(VERSION 3.10)
project(ISPMeetings)

set(CMAKE_CXX_STANDARD 17)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(include domain/include handlers/include dao/include ${CONAN_INCLUDE_DIRS})
add_subdirectory(handlers)
add_subdirectory(domain)
add_subdirectory(dao)

add_executable(${PROJECT_NAME}
		main.cpp
		src/server.cpp
		src/sqlite_session_factory.cpp
		$<TARGET_OBJECTS:domain>
		$<TARGET_OBJECTS:dao>
		$<TARGET_OBJECTS:handlers>)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})