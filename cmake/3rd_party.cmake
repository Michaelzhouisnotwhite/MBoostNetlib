function(fetch_content_check content_name)
    FetchContent_MakeAvailable(${content_name})
    message("---- ${content_name}_SOURCE_DIR is found in ${${content_name}_SOURCE_DIR}")
endfunction()

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 10.0.0
)

FetchContent_Declare(
    Boost
    URL https://github.com/boostorg/boost/releases/download/boost-1.82.0/boost-1.82.0.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_Declare(
    toy
    GIT_REPOSITORY https://github.com/Michaelzhouisnotwhite/My-CXX-Small-Toys
    GIT_TAG main
)

# Also requires one of: libbfd (gnu binutils), libdwarf, libdw (elfutils)
# FetchContent_Declare(backward
#     GIT_REPOSITORY https://github.com/bombela/backward-cpp
#     GIT_TAG master # or a version tag, such as v1.6
#     SYSTEM # optional, the Backward include directory will be treated as system directory
# )
# FetchContent_MakeAvailable(backward)

set(BOOST_INCLUDE_LIBRARIES bind asio url beast)

fetch_content_check(Boost)
fetch_content_check(fmt)
fetch_content_check(toy)