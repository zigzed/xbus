
include_directories(../)
include_directories(${COMMONLIB_INC})

set(XBUS_ROOT ${CMAKE_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${XBUS_ROOT}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${XBUS_ROOT}/bin/${CMAKE_BUILD_TYPE})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PTHREAD pthread)
    set(DL      dl)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DDLL_EXPORTS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    add_definitions(-DDEBUG)
endif(${CMAKE_BUILD_TYPE} MATCHES Debug)


set(XBUS_SRC
    ../xbus/xbus.h
    xbus_msg.h xbus_msg.cpp
    xbus_cfg.h xbus_cfg.cpp
    producer.h producer.cpp
    binlog/binlog.h binlog/binlog.cpp
    binlog/binlogset.cpp
    binlog/ioworker.h binlog/ioworker.cpp
    )

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-fPIC)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

link_directories(../../lib/${CMAKE_BUILD_TYPE})
link_directories(${COMMONLIB_LIB})
add_library(xbus SHARED ${XBUS_SRC})
target_link_libraries(xbus common ${PTHREAD} ${DL})

add_executable(test_ioqueue binlog/test_ioqueue.cpp 
                binlog/binlog.h binlog/ioworker.h binlog/binlog.cpp
                binlog/ioworker.cpp binlog/binlogset.cpp
                xbus_msg.h xbus_msg.cpp
              )
target_link_libraries(test_ioqueue common gtest ${PTHREAD})
add_executable(binlog_iter binlog/binlog_iter.cpp 
		binlog/binlog.h binlog/ioworker.h binlog/binlog.cpp
        binlog/ioworker.cpp binlog/binlogset.cpp
		xbus_msg.h xbus_msg.cpp
		)
target_link_libraries(binlog_iter common ${PTHREAD})
