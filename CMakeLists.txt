project(xbus)
cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

set(COMMONLIB_PATH	${PROJECT_SOURCE_DIR}/../common)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    if(CMAKE_CL_64)
	set(ICEE_TAIL "x64")
    else(CMAKE_CL_64)
	set(ICEE_TAIL "x86")
    endif(CMAKE_CL_64)
endif ("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")

add_subdirectory(src)
add_subdirectory(test)

