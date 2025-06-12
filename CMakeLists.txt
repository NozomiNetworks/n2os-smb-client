cmake_minimum_required(VERSION 3.12)

project(n2os_smb_client)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_compile_options("-O2" "-fPIE" "-fPIC" "-fstack-protector" "-D_FORTIFY_SOURCE=2")
add_link_options("-pie" "-Wl,-z,relro,-z,now")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs")

# Add option for static build
option(BUILD_STATIC "Build statically linked binary" OFF)
if(BUILD_STATIC)
    message(STATUS "Building statically linked binary")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

set(LIBJSONC_DIR "${PROJECT_SOURCE_DIR}/deps/json-c")
message(STATUS "Lib json-c: ${LIBJSONC_DIR}")
add_subdirectory(${LIBJSONC_DIR})


set(LIBSMB2_DIR "${PROJECT_SOURCE_DIR}/deps/libsmb2")
message(STATUS "Lib smb2: ${LIBSMB2_DIR}")

# Check if ENV_ARM is set to 1
if(DEFINED ENV{ENV_ARM} AND "$ENV{ENV_ARM}" STREQUAL "1")
    set(ARCH_LIB_DIR "aarch64-linux-gnu")
    message(STATUS "Building for ARM64 architecture")
else()
    set(ARCH_LIB_DIR "x86_64-linux-gnu")
    message(STATUS "Building for x86_64 architecture")
endif()

if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    # FreeBSD specific fixes
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_definitions(-include freebsd_fix.h)
    add_compile_options("-Wno-macro-redefined")

    # FreeBSD Heimdal Kerberos paths (instead of MIT)
    message(STATUS "Using Heimdal Kerberos on FreeBSD")
    set(GSSAPI_INCLUDE_DIR "/usr/include")
    set(GSSAPI_LIBRARIES "/usr/lib")
    set(GSSAPI_LIBRARY "/usr/lib/libgssapi.so")
    set(KRB_LIBRARY "/usr/lib/libkrb5.so")
    
    # Add Heimdal-specific define
    add_definitions(-DUSE_HEIMDAL_KERBEROS)
else()
    # Linux Kerberos paths (MIT)
    message(STATUS "Using MIT Kerberos on Linux")
    set(GSSAPI_INCLUDE_DIR "/usr/include")
    set(GSSAPI_LIBRARIES "/usr/lib/${ARCH_LIB_DIR}/")
    set(GSSAPI_LIBRARY "/usr/lib/${ARCH_LIB_DIR}/libgssapi_krb5.so.2")
    set(KRB_LIBRARY "/usr/lib/${ARCH_LIB_DIR}/libkrb5.so.3")
endif()

message(STATUS "Kerberos support is ENABLED")
message(STATUS "Using libraries from: ${GSSAPI_LIBRARIES}")
include_directories(BEFORE SYSTEM ${GSSAPI_INCLUDE_DIR})

# Set additional compiler flags for libsmb2 to support Heimdal
if (NOT "${CMAKE_SYSTEM}" MATCHES "Linux")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_HEIMDAL")
endif()

add_subdirectory(${LIBSMB2_DIR})

include_directories(${LIBJSONC_DIR})
include_directories(${LIBSMB2_DIR}/include/smb2)
include_directories(${LIBSMB2_DIR}/include)

# Different libraries for Linux and FreeBSD
if ("${CMAKE_SYSTEM}" MATCHES "Linux")
    link_libraries(${GSSAPI_LIBRARY} ${KRB_LIBRARY} smb2 json-c)
    add_executable(n2os_smb_client n2os-smb-client.c)
    target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${KRB_LIBRARY} smb2 json-c)
else()
    # For FreeBSD with Heimdal
    link_libraries(${GSSAPI_LIBRARY} ${KRB_LIBRARY} smb2 json-c)
    add_executable(n2os_smb_client n2os-smb-client.c)
    target_link_libraries(n2os_smb_client ${GSSAPI_LIBRARY} ${KRB_LIBRARY} smb2 json-c)
endif()