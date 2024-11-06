# FetchSDL.cmake
include(FetchContent)

set(SDL_VERSION "2.30.9")

FetchContent_Declare(
    SDL2
    DOWNLOAD_EXTRACT_TIMESTAMP 1
    URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/SDL2-${SDL_VERSION}.zip
    URL_HASH SHA256=ec855bcd815b4b63d0c958c42c2923311c656227d6e0c1ae1e721406d346444b
)

# Platform specific options
if(WIN32)
    set(SDL_SHARED ON CACHE BOOL "Build SDL2 shared library")
    set(SDL_STATIC OFF CACHE BOOL "Build SDL2 static library")
elseif(APPLE)
    set(SDL_SHARED ON CACHE BOOL "Build SDL2 shared library")
    set(SDL_STATIC OFF CACHE BOOL "Build SDL2 static library")
    set(SDL_FRAMEWORK OFF CACHE BOOL "Build SDL2 framework")
else()
    set(SDL_SHARED ON CACHE BOOL "Build SDL2 shared library")
    set(SDL_STATIC OFF CACHE BOOL "Build SDL2 static library")
endif()

FetchContent_MakeAvailable(SDL2)

# Installation rules
if(WIN32)
    install(
        FILES $<TARGET_FILE:SDL2>
        DESTINATION bin
    )
    install(
        FILES $<TARGET_LINKER_FILE:SDL2>
        DESTINATION lib
    )
else()
    install(
        TARGETS SDL2
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
endif()

# Install headers
install(
    DIRECTORY ${SDL2_SOURCE_DIR}/include/
    DESTINATION include/SDL2
    FILES_MATCHING PATTERN "*.h"
)
