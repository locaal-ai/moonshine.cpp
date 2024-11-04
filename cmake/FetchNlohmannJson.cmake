include(FetchContent)

FetchContent_Declare(json
    DOWNLOAD_EXTRACT_TIMESTAMP 1
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)