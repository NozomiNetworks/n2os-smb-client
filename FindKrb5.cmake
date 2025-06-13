message(STATUS "Looking for Kerberos libraries...")

find_path(LibKrb5_INCLUDE_DIR
    NAMES krb5.h krb5-types.h
    PATHS
        ${LibKrb5_ROOT_DIR}/include
        /usr/include/heimdal
        /usr/local/include/heimdal
)

# If Heimdal headers not found, try MIT Kerberos locations
if(NOT LibKrb5_INCLUDE_DIR)
    find_path(LibKrb5_INCLUDE_DIR
        NAMES krb5.h
        PATHS
            ${LibKrb5_ROOT_DIR}/include
            /usr/include
            /usr/include/krb5
            /usr/local/include
    )
endif()

# Check Heimdal lib paths first
find_library(LibKrb5_LIBRARY
    NAMES krb5 libkrb5
    PATHS
        ${LibKrb5_ROOT_DIR}/lib
        ${LibKrb5_ROOT_DIR}/lib/heimdal
        /usr/lib/heimdal
        /usr/lib/x86_64-linux-gnu/heimdal
        /usr/lib/aarch64-linux-gnu/heimdal
        /usr/local/lib/heimdal
)

# If Heimdal lib not found, try MIT Kerberos locations
if(NOT LibKrb5_LIBRARY)
    find_library(LibKrb5_LIBRARY
        NAMES krb5 libkrb5
        PATHS
            ${LibKrb5_ROOT_DIR}/lib
            /usr/lib
            /usr/lib/x86_64-linux-gnu
            /usr/lib/aarch64-linux-gnu
            /usr/local/lib
    )
endif()

# Determine which Kerberos implementation we found
if(LibKrb5_INCLUDE_DIR AND LibKrb5_LIBRARY)
    if(EXISTS "${LibKrb5_INCLUDE_DIR}/krb5-types.h")
        set(LibKrb5_IMPL "Heimdal")
    else()
        set(LibKrb5_IMPL "MIT")
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibKrb5 DEFAULT_MSG
    LibKrb5_LIBRARY
    LibKrb5_INCLUDE_DIR
)

if(LibKrb5_FOUND)
    message(STATUS "Found ${LibKrb5_IMPL} Kerberos implementation")
endif()

mark_as_advanced(
    LibKrb5_ROOT_DIR
    LibKrb5_LIBRARY
    LibKrb5_INCLUDE_DIR
    LibKrb5_IMPL
)