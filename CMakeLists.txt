project(cavan)
cmake_minimum_required(VERSION 3.5)

execute_process(OUTPUT_VARIABLE CAVAN_OS_NAME COMMAND uname -s)
string(STRIP ${CAVAN_OS_NAME} CAVAN_OS_NAME)

execute_process(OUTPUT_VARIABLE CAVAN_PLAT COMMAND gcc -dumpmachine)
string(STRIP ${CAVAN_PLAT} CAVAN_PLAT)

message("CAVAN_OS_NAME = ${CAVAN_OS_NAME}")
message("CAVAN_PLAT = ${CAVAN_PLAT}")

set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/out/host/lib")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/out/host/debug")
set(SSL_CERT_PATH "${PROJECT_SOURCE_DIR}/config/ssl")

add_custom_target(out_lib COMMAND mkdir -pv \"${LIBRARY_OUTPUT_PATH}\")
add_custom_target(out_bin COMMAND mkdir -pv \"${EXECUTABLE_OUTPUT_PATH}\")

include_directories("${PROJECT_INCLUDE_DIR}")
add_definitions(-g -Os -DCAVAN -DCAVAN_OS_NAME=\"${CAVAN_OS_NAME}\" -DCAVAN_PLAT=\"${CAVAN_PLAT}\" -include \"${PROJECT_INCLUDE_DIR}/cavan/config.h\")
add_definitions(-DCONFIG_CAVAN_SSL -DCONFIG_CAVAN_SSL_CERT=\"${SSL_CERT_PATH}/cert.pem\" -DCONFIG_CAVAN_SSL_KEY=\"${SSL_CERT_PATH}/key.pem\")

aux_source_directory("${PROJECT_SOURCE_DIR}/lib" LIB_SOURCES)
aux_source_directory("${PROJECT_SOURCE_DIR}/cpp" LIB_SOURCES)
aux_source_directory("${PROJECT_SOURCE_DIR}/app" APP_SOURCES)
aux_source_directory("${PROJECT_SOURCE_DIR}/test" APP_SOURCES)

add_library(cavan SHARED ${LIB_SOURCES})
add_dependencies(cavan out_lib)
# set_target_properties(cavan PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/host/lib")

add_library(cavan-static STATIC ${LIB_SOURCES})
add_dependencies(cavan-static out_lib)

foreach (src ${APP_SOURCES})
	string(REGEX REPLACE ".*/(.*)\\..+$" "cavan-\\1" app ${src})
	add_executable(${app} ${src})
	target_link_libraries(${app} cavan pthread ssl)
	add_dependencies(${app} out_bin)
	# set_target_properties(${app} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/host/debug")
endforeach()
