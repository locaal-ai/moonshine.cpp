# moonshine.cpp

Standalone C++ implementation of [Moonshine ASR](https://github.com/usefulsensors/moonshine) with [ONNXRuntime](https://github.com/microsoft/onnxruntime) and no other dependencies.

## Table of Contents

- [Introduction](#introduction)
- [Build Instructions](#build-instructions)
- [Example Usage](#example-usage)
- [Using as a Library](#using-as-a-library)
- [Credits](#credits)
- [License](#license)

## Introduction

[Moonshine ASR](https://github.com/usefulsensors/moonshine) is an Automatic Speech Recognition (ASR) system implemented in C++ using [ONNX Runtime](https://github.com/microsoft/onnxruntime). This project provides a standalone implementation that can be built and run on various platforms.

## Build Instructions

### Prerequisites

- CMake 3.15 or higher
- A C++17 compatible compiler
- ONNX Runtime for the building OS will be fetched in build time.

### Building on Windows

1. Open a PowerShell terminal.
2. Navigate to the root directory of the project.
3. Run the build script:

    ```ps1
    .\scripts\build.ps1
    ```

### Building on Linux/MacOS

1. Open a terminal.
2. Navigate to the root directory of the project.
3. Run the build script:

    ```sh
    ./scripts/build.sh
    ```

### Manual Build Steps

1. Create a build directory:

    ```sh
    mkdir -p build
    cd build
    ```

2. Configure the project with CMake:

    ```sh
    cmake -DCMAKE_BUILD_TYPE=Release ..
    ```

3. Build the project:

    ```sh
    cmake --build .
    ```

4. Install the project:

    ```sh
    cmake --install . --prefix ../dist
    ```

## Example Usage

After building the project, you can run the example executable to test the ASR system.

```sh
./dist/bin/moonshine_example <models_dir> <wav_file>
```

Replace `<models_dir>` with the directory containing your ONNX models and `<wav_file>` with the path to a WAV file.

### Example

```sh
./dist/bin/moonshine_example models/ example.wav
```

## Using as a Library

To use Moonshine ASR as a library in your C++ project, follow these steps:

1. Build and install the Moonshine ASR library as described in the [Build Instructions](#build-instructions).

2. Link against the installed library in your CMake project:

    ```cmake
    cmake_minimum_required(VERSION 3.15)
    project(my_project)

    # Find the Moonshine package
    find_package(Moonshine REQUIRED)

    add_executable(my_project main.cpp)

    # Link against the Moonshine library
    target_link_libraries(my_project PRIVATE Moonshine::moonshine)
    ```

3. Include the Moonshine header in your source code:

    ```cpp
    #include <moonshine.hpp>

    int main() {
        MoonshineModel model("path/to/models");
        // Use the model...
        return 0;
    }
    ```

## Credits

This project is based on the Moonshine ASR system: https://github.com/usefulsensors/moonshine. Special thanks to the Moonshine team for their contributions.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
