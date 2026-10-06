message(STATUS "Looking for GSSAPI libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)
message(STATUS "Kerberos implementation selected: ${KRB_IMPL_UPPER}")

# Apple systems are not handled here: libsmb2 configures Kerberos on its
# own against the system GSS.framework (see CMakeLists.txt).
if(NOT KRB_IMPL_UPPER STREQUAL "MIT")
    message(FATAL_ERROR "On non-Apple systems, only MIT Kerberos implementation is supported")
endif()

find_path(GSSAPI_INCLUDE_DIR
    NAMES gssapi/gssapi.h
    PATHS
        /usr/include
        /usr/local/include
)

find_library(GSSAPI_LIBRARY
    NAMES gssapi_krb5 libgssapi_krb5
    PATHS
        /usr/lib
        /usr/local/lib
        /usr/lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GSSAPI DEFAULT_MSG
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)

mark_as_advanced(
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)
