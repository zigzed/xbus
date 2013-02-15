
include_directories(../)

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


set(XBUS_SRC
    ../xbus/xbus.h
    advertise.h advertise.cpp
    discovery.h discovery.cpp
    command.h command.cpp
    monitor.h monitor.cpp
    )

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-fPIC)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

link_directories(../../lib/${CMAKE_BUILD_TYPE})
add_library(xbus SHARED ${XBUS_SRC})
