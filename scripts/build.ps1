param (
    [string]$BuildType = "Release"
)

# find the directory of the script
$DIR = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ROOT_DIR = Split-Path -Parent $DIR

# Create build directory
New-Item -ItemType Directory -Force -Path "$ROOT_DIR/build"

# Configure with system ONNX Runtime
cmake -B "$ROOT_DIR/build" -S "$ROOT_DIR" -DCMAKE_BUILD_TYPE=$BuildType
if (!$LASTEXITCODE -eq 0) {
    Write-Host "Config failed."
    exit 1
}

# Build
cmake --build "$ROOT_DIR/build" --config $BuildType
if ($LASTEXITCODE -eq 0) {
    Write-Host "Build succeeded."
} else {
    Write-Host "Build failed."
    exit 1
}

# Install to current directory
cmake --install "$ROOT_DIR/build" --prefix "$ROOT_DIR/dist"
