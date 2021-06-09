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

if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
set(GSSAPI_LIBRARY "/usr/local/lib/libgssapi_krb5.so")
set(KRB_LIBRARY "/usr/local/lib/libkrb5.so")
endif()

if ("${CMAKE_SYSTEM}" MATCHES "Linux")
set(GSSAPI_LIBRARY "/usr/lib/x86_64-linux-gnu/libgssapi_krb5.so.2")
set(KRB_LIBRARY "/usr/lib/x86_64-linux-gnu/libkrb5.so.3")
endif()

target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${KRB_LIBRARY} smb2 json-c)