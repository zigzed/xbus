
include_directories(../)
link_directories(${COMMONLIB_LIB})

set(XBUS_ROOT ${CMAKE_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${XBUS_ROOT}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${XBUS_ROOT}/bin/${CMAKE_BUILD_TYPE})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PTHREAD pthread)
    set(DL      dl)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DDEBUG)
endif(${CMAKE_BUILD_TYPE} MATCHES Debug)

add_executable(publisher publisher.cpp)
target_link_libraries(publisher xbus)
add_executable(subscriber subscriber.cpp)
target_link_libraries(subscriber xbus)
