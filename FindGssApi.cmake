message(STATUS "Looking for GSSAPI libraries...")

# Try to find Heimdal GSSAPI first
find_path(GSSAPI_INCLUDE_DIR
    NAMES gssapi.h gssapi/gssapi.h
    PATHS
        ${GSSAPI_ROOT_DIR}/include
        /usr/include/heimdal
        /usr/local/include/heimdal
)

# If Heimdal headers not found, try MIT GSSAPI locations
if(NOT GSSAPI_INCLUDE_DIR OR 
   (NOT EXISTS "${GSSAPI_INCLUDE_DIR}/gssapi.h" AND 
    NOT EXISTS "${GSSAPI_INCLUDE_DIR}/gssapi/gssapi.h"))
    find_path(GSSAPI_INCLUDE_DIR
        NAMES gssapi/gssapi.h
        PATHS
            ${GSSAPI_ROOT_DIR}/include
            /usr/include
            /usr/local/include
    )
endif()

# Check Heimdal lib paths first
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

# If Heimdal lib not found, try MIT GSSAPI locations (gssapi_krb5)
if(NOT GSSAPI_LIBRARY)
    find_library(GSSAPI_LIBRARY
        NAMES gssapi_krb5 libgssapi_krb5
        PATHS
            ${GSSAPI_ROOT_DIR}/lib
            /usr/lib
            /usr/lib/x86_64-linux-gnu
            /usr/lib/aarch64-linux-gnu
            /usr/local/lib
    )
endif()

# Determine which GSSAPI implementation we found
if(GSSAPI_INCLUDE_DIR AND GSSAPI_LIBRARY)
    if(GSSAPI_LIBRARY MATCHES "heimdal" OR EXISTS "${GSSAPI_INCLUDE_DIR}/heimdal")
        set(GSSAPI_IMPL "Heimdal")
    else()
        # Check for MIT mechglue header as another way to identify MIT Kerberos
        if(EXISTS "${GSSAPI_INCLUDE_DIR}/gssapi/mechglue.h")
            set(GSSAPI_IMPL "MIT")
        else()
            # Default to MIT if we can't clearly identify
            set(GSSAPI_IMPL "MIT")
        endif()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GSSAPI DEFAULT_MSG
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
)

if(GSSAPI_FOUND)
    message(STATUS "Found ${GSSAPI_IMPL} GSSAPI implementation")
    message(STATUS "GSSAPI include directory: ${GSSAPI_INCLUDE_DIR}")
    message(STATUS "GSSAPI library: ${GSSAPI_LIBRARY}")
endif()

mark_as_advanced(
    GSSAPI_ROOT_DIR
    GSSAPI_LIBRARY
    GSSAPI_INCLUDE_DIR
    GSSAPI_IMPL
)