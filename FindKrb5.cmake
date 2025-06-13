message(STATUS "Looking for Kerberos libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)
message(STATUS "Kerberos implementation selected: ${KRB_IMPL_UPPER}")

if(KRB_IMPL_UPPER STREQUAL "HEIMDAL")
    find_path(LibKrb5_INCLUDE_DIR
        NAMES krb5.h krb5-types.h
        PATHS
            ${LibKrb5_ROOT_DIR}/include
            /usr/include/heimdal
            /usr/local/include/heimdal
    )
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
    set(LibKrb5_IMPL "Heimdal")
elseif(KRB_IMPL_UPPER STREQUAL "MIT")
    find_path(LibKrb5_INCLUDE_DIR
        NAMES krb5.h
        PATHS
            ${LibKrb5_ROOT_DIR}/include
            /usr/include
            /usr/include/krb5
            /usr/local/include
    )
    find_library(LibKrb5_LIBRARY
        NAMES krb5 libkrb5
        PATHS
            ${LibKrb5_ROOT_DIR}/lib
            /usr/lib
            /usr/lib/x86_64-linux-gnu
            /usr/lib/aarch64-linux-gnu
            /usr/local/lib
    )
    set(LibKrb5_IMPL "MIT")
else()
    message(FATAL_ERROR "Unknown Kerberos implementation: ${KRB_IMPL}")
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
