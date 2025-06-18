message(STATUS "Looking for GSSAPI libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)
message(STATUS "Kerberos implementation selected: ${KRB_IMPL_UPPER}")

if(APPLE)
    # For Apple systems, only HEIMDAL is supported
    if(NOT KRB_IMPL_UPPER STREQUAL "HEIMDAL")
        message(FATAL_ERROR "On Apple systems, only HEIMDAL Kerberos implementation is supported")
    endif()
    
    # For macOS, explicitly prefer Homebrew Heimdal over system GSSAPI
    find_path(GSSAPI_INCLUDE_DIR
        NAMES gssapi.h gssapi/gssapi.h
        PATHS
            /opt/homebrew/opt/heimdal/include
            /usr/local/opt/heimdal/include
        NO_DEFAULT_PATH  # Don't search in system paths
    )
    find_library(GSSAPI_LIBRARY
        NAMES gssapi libgssapi
        PATHS
            /opt/homebrew/opt/heimdal/lib
            /usr/local/opt/heimdal/lib
        NO_DEFAULT_PATH  # Don't search in system paths
    )
    
    if(NOT GSSAPI_INCLUDE_DIR OR NOT GSSAPI_LIBRARY)
        find_path(GSSAPI_INCLUDE_DIR
            NAMES gssapi.h gssapi/gssapi.h
            PATHS
                /usr/include
        )
        find_library(GSSAPI_LIBRARY
            NAMES gssapi libgssapi
            PATHS
                /usr/lib
        )
    endif()
else()
    # For non-Apple systems, only MIT is supported
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
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GSSAPI DEFAULT_MSG
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)

mark_as_advanced(
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)
