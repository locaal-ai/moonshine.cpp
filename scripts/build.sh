# Set default build type to Release
BUILD_TYPE=${1:-Release}

# find the directory of the script
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ROOT_DIR="$(dirname "$DIR")"

# Create build directory
mkdir -p $ROOT_DIR/build

# Configure with system ONNX Runtime
cmake -B $ROOT_DIR/build -S $ROOT_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
cmake --build $ROOT_DIR/build --config $BUILD_TYPE

# Install to current directory
cmake --install $ROOT_DIR/build --prefix $ROOT_DIR/dist --config $BUILD_TYPE
