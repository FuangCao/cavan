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
aux_source_directory(app APP_SOURCES)
aux_source_directory(test APP_SOURCES)

add_library(cavan SHARED ${LIB_SOURCES})
# set_target_properties(cavan PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/host/lib")

add_library(cavan-static STATIC ${LIB_SOURCES})

foreach (src ${APP_SOURCES})
	string(REGEX REPLACE ".*/(.*)\\..+$" "cavan-\\1" app ${src})
	add_executable(${app} ${src})
	target_link_libraries(${app} cavan pthread)
	# set_target_properties(${app} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/host/debug")
endforeach()

set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/out/host/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/out/host/debug")
