cmake_minimum_required(VERSION 3.12)
project(n2os_smb_client)

set(CMAKE_CXX_STANDARD 11)


set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Libsmb2 ${LIBSMB2_DIR}")
add_subdirectory(${LIBSMB2_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)

set(LIBJSONC_DIR "${PROJECT_SOURCE_DIR}/deps/json-c")
message(STATUS "Lib JSON-C ${LIBJSONC_DIR}")
#set(BUILD_SHARED_LIBS "OFF")
#set(DISABLE_WERROR true)
#add_subdirectory(${LIBJSONC_DIR} json-c-static)

execute_process(COMMAND cmake "${LIBJSONC_DIR}")
execute_process(COMMAND make -C "${LIBJSONC_DIR}" json-c-static)
include_directories(${LIBJSONC_DIR})

add_executable(n2os_smb_client n2os-smb-client.c)
target_link_libraries(n2os_smb_client ${LIBSMB2_DIR}/lib/libsmb2.a ${LIBJSONC_DIR}/libjson-c.a)
