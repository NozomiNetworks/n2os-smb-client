cmake_minimum_required(VERSION 3.12)

project(n2os_smb_client)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_compile_options("-O2" "-fPIE" "-fPIC" "-fstack-protector" "-D_FORTIFY_SOURCE=2")

if(NOT APPLE)
    add_link_options("-pie" "-Wl,-z,relro,-z,now")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")

## LIB SMB2
if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux" AND NOT APPLE)
    # FreeBSD specific fixes
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_definitions(-include freebsd_fix.h)
    add_compile_options("-Wno-macro-redefined")
endif()

set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Lib smb2: ${LIBSMB2_DIR}")

include(${CMAKE_CURRENT_LIST_DIR}/FindKrb5.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindGssApi.cmake)

if(NOT LibKrb5_FOUND)
    message(FATAL_ERROR "Kerberos libraries not found. Please install krb5 development packages.")
endif()

if(NOT GSSAPI_FOUND)
    message(FATAL_ERROR "GSSAPI libraries not found. Please install GSSAPI development packages.")
endif()

message(STATUS "LibKrb5_LIBRARY: ${LibKrb5_LIBRARY}")
message(STATUS "LibKrb5_INCLUDE_DIR: ${LibKrb5_INCLUDE_DIR}")
message(STATUS "LibKrb5_IMPL: ${LibKrb5_IMPL}")

message(STATUS "GSSAPI_LIBRARY: ${GSSAPI_LIBRARY}")
message(STATUS "GSSAPI_INCLUDE_DIR: ${GSSAPI_INCLUDE_DIR}")
message(STATUS "KRB_IMPL: ${KRB_IMPL}")

include_directories(BEFORE SYSTEM ${GSSAPI_INCLUDE_DIR} ${LibKrb5_INCLUDE_DIR})

# force compile with krb5, library removes the support if krb5 is not found
add_definitions(-DHAVE_LIBKRB5)

add_subdirectory(${LIBSMB2_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)


## LIB JSON-C
if(APPLE)
    # Use Homebrew's json-c on macOS
    execute_process(COMMAND brew --prefix json-c
        OUTPUT_VARIABLE BREW_JSONC_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(JSONC_INCLUDE_DIR "${BREW_JSONC_PREFIX}/include")
    set(JSONC_LIBRARY "${BREW_JSONC_PREFIX}/lib/libjson-c.dylib")
    message(STATUS "Using Homebrew json-c at: ${BREW_JSONC_PREFIX}")
    
    include_directories(${JSONC_INCLUDE_DIR})
    link_directories(${BREW_JSONC_PREFIX}/lib)
else()
    set(LIBJSONC_DIR "${PROJECT_SOURCE_DIR}/deps/json-c")
    message(STATUS "Lib json-c: ${LIBJSONC_DIR}")
    add_subdirectory(${LIBJSONC_DIR})
    include_directories(${LIBJSONC_DIR})
endif()

# Finalize
if(APPLE)
    link_libraries(${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 ${JSONC_LIBRARY})
else()
    link_libraries(${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 json-c)
endif()

add_executable(n2os_smb_client n2os-smb-client.c)

if(APPLE)
    target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 ${JSONC_LIBRARY})
else()
    target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${LibKrb5_LIBRARY} smb2 json-c)
endif()