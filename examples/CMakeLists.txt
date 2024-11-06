# examples/CMakeLists.txt

include(../cmake/FetchSDL.cmake)

# Create example executables
add_executable(moonshine_example demo.cpp)
add_executable(moonshine_live live.cpp)

target_link_libraries(moonshine_example
    PRIVATE
        moonshine
)

target_link_libraries(moonshine_live
    PRIVATE
        moonshine
        SDL2::SDL2
)

target_include_directories(moonshine_example
    PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIRS}
)

target_include_directories(moonshine_live
    PRIVATE
        ${ONNXRUNTIME_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
)

# Install example executable to bin directory
install(TARGETS moonshine_example moonshine_live
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
