message(STATUS "Looking for Kerberos libraries...")
message(STATUS "KRB_IMPL variable value: ${KRB_IMPL}")

# Get KRB_IMPL from environment or cache
string(TOUPPER "${KRB_IMPL}" KRB_IMPL_UPPER)
message(STATUS "Kerberos implementation selected: ${KRB_IMPL_UPPER}")

if(APPLE)
    # For Apple systems, only HEIMDAL is supported
    if(NOT KRB_IMPL_UPPER STREQUAL "HEIMDAL")
        message(FATAL_ERROR "On Apple systems, only HEIMDAL Kerberos implementation is supported")
    endif()
    
    # For macOS, explicitly prefer Homebrew Heimdal over system Kerberos
    find_path(LibKrb5_INCLUDE_DIR
        NAMES krb5.h krb5-types.h
        PATHS
            /opt/homebrew/opt/heimdal/include
            /usr/local/opt/heimdal/include
        NO_DEFAULT_PATH  # Don't search in system paths
    )
    find_library(LibKrb5_LIBRARY
        NAMES krb5 libkrb5
        PATHS
            /opt/homebrew/opt/heimdal/lib
            /usr/local/opt/heimdal/lib
        NO_DEFAULT_PATH  # Don't search in system paths
    )
    
    if(NOT LibKrb5_INCLUDE_DIR OR NOT LibKrb5_LIBRARY)
        find_path(LibKrb5_INCLUDE_DIR
            NAMES krb5.h
            PATHS
                /System/Library/Frameworks/Kerberos.framework/Headers
        )
        find_library(LibKrb5_LIBRARY
            NAMES krb5 libkrb5
            PATHS
                /usr/lib
        )
    endif()
    
    set(LibKrb5_IMPL "Heimdal")
else()
    # For non-Apple systems, only MIT is supported
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
endif()

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
