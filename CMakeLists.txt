cmake_minimum_required(VERSION 3.12)
project(n2os_smb_client)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_compile_options("-O2" "-fPIE" "-fPIC" "-fstack-protector" "-D_FORTIFY_SOURCE=2")
add_link_options("-pie" "-Wl,-z,relro,-z,now")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")
set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Lib smb2: ${LIBSMB2_DIR}")
add_subdirectory(${LIBSMB2_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)

set(LIBJSONC_DIR "${PROJECT_SOURCE_DIR}/deps/json-c")
message(STATUS "Lib json-c: ${LIBJSONC_DIR}")
add_subdirectory(${LIBJSONC_DIR})

include_directories(${LIBJSONC_DIR})

add_executable(n2os_smb_client n2os-smb-client.c)
target_link_libraries(n2os_smb_client smb2 json-c)