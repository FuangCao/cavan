project(cavan)
cmake_minimum_required(VERSION 3.5)

execute_process(OUTPUT_VARIABLE CAVAN_OS_NAME COMMAND uname -s)
string(STRIP ${CAVAN_OS_NAME} CAVAN_OS_NAME)

execute_process(OUTPUT_VARIABLE CAVAN_PLAT COMMAND gcc -dumpmachine)
string(STRIP ${CAVAN_PLAT} CAVAN_PLAT)

message("CAVAN_OS_NAME = ${CAVAN_OS_NAME}")
message("CAVAN_PLAT = ${CAVAN_PLAT}")

include_directories(include)
add_definitions(-g -Os -DCAVAN -DCAVAN_OS_NAME="${CAVAN_OS_NAME}" -DCAVAN_PLAT="${CAVAN_PLAT}")

aux_source_directory(lib LIB_SOURCES)
aux_source_directory(cpp LIB_SOURCES)

add_library(cavan SHARED ${LIB_SOURCES})
add_library(cavan-static STATIC ${LIB_SOURCES})
