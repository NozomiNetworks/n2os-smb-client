cmake_minimum_required(VERSION 3.17)
project(n2os_smb_client)
#include(LibFindMacros)

set(CMAKE_CXX_STANDARD 11)


set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Libsmb2 ${LIBSMB2_DIR}")
add_subdirectory(${LIBSMB2_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)

#message(STATUS "Building libsmb2 in folder ${LIBSMB2_DIR}")
#execute_process(COMMAND cmake -DBUILD_SHARED_LIBS=OFF -DENABLE_EXAMPLES=OFF -S "${LIBSMB2_DIR}" -B "${LIBSMB2_DIR}" )
#execute_process(COMMAND make -C "${LIBSMB2_DIR}")


add_executable(n2os_smb_client n2os-smb-client.c)
target_link_libraries(n2os_smb_client ${LIBSMB2_DIR}/lib/libsmb2.a)
