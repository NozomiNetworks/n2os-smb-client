message(STATUS "Looking for Kerberos libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)
message(STATUS "Kerberos implementation selected: ${KRB_IMPL_UPPER}")

# Apple systems are not handled here: libsmb2 configures Kerberos on its
# own against the system GSS.framework (see CMakeLists.txt).
if(NOT KRB_IMPL_UPPER STREQUAL "MIT")
    message(FATAL_ERROR "On non-Apple systems, only MIT Kerberos implementation is supported")
endif()

find_path(LibKrb5_INCLUDE_DIR
    NAMES krb5.h
    PATHS
        /usr/include
        /usr/include/krb5
        /usr/local/include
)

find_library(LibKrb5_LIBRARY
    NAMES krb5 libkrb5
    PATHS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
        /usr/local/lib
)

set(LibKrb5_IMPL "MIT")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibKrb5 DEFAULT_MSG
    LibKrb5_LIBRARY
    LibKrb5_INCLUDE_DIR
)

mark_as_advanced(
    LibKrb5_LIBRARY
    LibKrb5_INCLUDE_DIR
    LibKrb5_IMPL
)
