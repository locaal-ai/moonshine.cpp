# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(moonshine-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find ONNX Runtime package
include(cmake/FetchOnnxruntime.cmake)

# Create moonshine library
add_library(moonshine
    src/moonshine.cpp
)

target_include_directories(moonshine
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${ONNXRUNTIME_INCLUDE_DIRS}
)

target_link_libraries(moonshine PRIVATE Ort)

# Create example executable
add_executable(moonshine_example examples/demo.cpp)

target_link_libraries(moonshine_example
    PRIVATE
    moonshine
)

target_include_directories(moonshine_example
    PRIVATE
    ${ONNXRUNTIME_INCLUDE_DIRS}
)

# Installation rules
include(GNUInstallDirs)

install(TARGETS moonshine
    EXPORT moonshine-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES src/moonshine.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/moonshine
)

# Export targets
install(EXPORT moonshine-targets
    FILE moonshine-targets.cmake
    NAMESPACE Moonshine::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonshine
)

# Create and install config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/moonshine-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/moonshine-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonshine
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/moonshine-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/moonshine-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/moonshine-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonshine
)

# Install example executable to bin directory
install(TARGETS moonshine_example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
