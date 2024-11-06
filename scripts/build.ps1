param (
    [string]$BuildType = "Release",
    [switch]$BuildExamples = $false
)

# find the directory of the script
$DIR = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ROOT_DIR = Split-Path -Parent $DIR

# Create build directory
New-Item -ItemType Directory -Force -Path "$ROOT_DIR/build" -ErrorAction SilentlyContinue

# Configure with system ONNX Runtime
$cmakeArgs = "-B `"$ROOT_DIR/build`" -S `"$ROOT_DIR`" -DCMAKE_BUILD_TYPE=$BuildType -DCMAKE_INSTALL_PREFIX=`"$ROOT_DIR/dist`""

if ($BuildExamples) {
    $cmakeArgs += " -DBUILD_EXAMPLES=ON"
}

Invoke-Expression "cmake $cmakeArgs"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Config failed."
    exit 1
}

# Build
cmake --build "$ROOT_DIR/build" --config $BuildType
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed."
    exit 1
}

# Install to current directory
cmake --install "$ROOT_DIR/build" --prefix "$ROOT_DIR/dist" --config $BuildType
if ($LASTEXITCODE -ne 0) {
    Write-Host "Install failed."
    exit 1
}

Write-Host "Build and install succeeded."