cmake_minimum_required(VERSION 3.12)

project(n2os_smb_client)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_compile_options("-O2" "-fPIE" "-fPIC" "-fstack-protector" "-D_FORTIFY_SOURCE=2")
add_link_options("-pie" "-Wl,-z,relro,-z,now")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")

set(LIBJSONC_DIR "${PROJECT_SOURCE_DIR}/deps/json-c")
message(STATUS "Lib json-c: ${LIBJSONC_DIR}")
add_subdirectory(${LIBJSONC_DIR})

set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Lib smb2: ${LIBSMB2_DIR}")

if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    # FreeBSD specific fixes
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_definitions(-include freebsd_fix.h)
    add_compile_options("-Wno-macro-redefined")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/FindKrb5.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindGssApi.cmake)

if(NOT LibKrb5_FOUND)
    message(FATAL_ERROR "Kerberos libraries not found. Please install krb5 development packages.")
endif()

if(NOT GSSAPI_FOUND)
    message(FATAL_ERROR "GSSAPI libraries not found. Please install GSSAPI development packages.")
endif()

message(STATUS "LibKrb5_ROOT_DIR: ${LibKrb5_ROOT_DIR}")
message(STATUS "LibKrb5_LIBRARY: ${LibKrb5_LIBRARY}")
message(STATUS "LibKrb5_INCLUDE_DIR: ${LibKrb5_INCLUDE_DIR}")
message(STATUS "LibKrb5_IMPL: ${LibKrb5_IMPL}")

message(STATUS "GSSAPI_ROOT_DIR: ${GSSAPI_ROOT_DIR}")
message(STATUS "GSSAPI_LIBRARY: ${GSSAPI_LIBRARY}")
message(STATUS "GSSAPI_INCLUDE_DIR: ${GSSAPI_INCLUDE_DIR}")
message(STATUS "KRB_IMPL: ${KRB_IMPL}")

include_directories(BEFORE SYSTEM ${GSSAPI_INCLUDE_DIR} ${LibKrb5_INCLUDE_DIR})

# force compile with krb5, library removes the support if krb5 is not found
add_definitions(-DHAVE_LIBKRB5)
add_subdirectory(${LIBSMB2_DIR})

include_directories(${LIBJSONC_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)

link_libraries(${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 json-c)
add_executable(n2os_smb_client n2os-smb-client.c)
target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 json-c)