message(STATUS "Looking for GSSAPI libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)

if(KRB_IMPL_UPPER STREQUAL "HEIMDAL")
    find_path(GSSAPI_INCLUDE_DIR
        NAMES gssapi.h gssapi/gssapi.h
        PATHS
            ${GSSAPI_ROOT_DIR}/include
            /usr/include/heimdal
            /usr/local/include/heimdal
    )
    find_library(GSSAPI_LIBRARY
        NAMES gssapi libgssapi
        PATHS
            ${GSSAPI_ROOT_DIR}/lib
            ${GSSAPI_ROOT_DIR}/lib/heimdal
            /usr/lib/heimdal
            /usr/lib/x86_64-linux-gnu/heimdal
            /usr/lib/aarch64-linux-gnu/heimdal
            /usr/local/lib/heimdal
    )
elseif(KRB_IMPL_UPPER STREQUAL "MIT")
    find_path(GSSAPI_INCLUDE_DIR
        NAMES gssapi/gssapi.h
        PATHS
            ${GSSAPI_ROOT_DIR}/include
            /usr/include
            /usr/local/include
    )
    find_library(GSSAPI_LIBRARY
        NAMES gssapi_krb5 libgssapi_krb5
        PATHS
            ${GSSAPI_ROOT_DIR}/lib
            /usr/lib
            /usr/lib/x86_64-linux-gnu
            /usr/lib/aarch64-linux-gnu
            /usr/local/lib
    )
else()
    message(FATAL_ERROR "Unknown KRB_IMPL: ${KRB_IMPL}. Use 'MIT' or 'Heimdal'.")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GSSAPI DEFAULT_MSG
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)

mark_as_advanced(
    GSSAPI_ROOT_DIR
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
    KRB_IMPL
)
