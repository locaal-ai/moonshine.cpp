include(FetchContent)

set(CUSTOM_ONNXRUNTIME_URL
    ""
    CACHE STRING "URL of a downloaded ONNX Runtime tarball")

set(CUSTOM_ONNXRUNTIME_HASH
    ""
    CACHE STRING "Hash of a downloaded ONNX Runtime tarball")

set(Onnxruntime_VERSION "1.19.2")

if(CUSTOM_ONNXRUNTIME_URL STREQUAL "")
  set(USE_PREDEFINED_ONNXRUNTIME ON)
else()
  if(CUSTOM_ONNXRUNTIME_HASH STREQUAL "")
    message(FATAL_ERROR "Both of CUSTOM_ONNXRUNTIME_URL and CUSTOM_ONNXRUNTIME_HASH must be present!")
  else()
    set(USE_PREDEFINED_ONNXRUNTIME OFF)
  endif()
endif()

if(USE_PREDEFINED_ONNXRUNTIME)
  set(Onnxruntime_BASEURL "https://github.com/microsoft/onnxruntime/releases/download/v${Onnxruntime_VERSION}")

  if(APPLE)
    set(Onnxruntime_URL "${Onnxruntime_BASEURL}/onnxruntime-osx-universal2-${Onnxruntime_VERSION}.tgz")
    set(Onnxruntime_HASH SHA256=b0289ddbc32f76e5d385abc7b74cc7c2c51cdf2285b7d118bf9d71206e5aee3a)
  elseif(MSVC)
    set(Onnxruntime_URL "${Onnxruntime_BASEURL}/onnxruntime-win-x64-${Onnxruntime_VERSION}.zip")
    set(OOnnxruntime_HASH SHA256=dc4f841e511977c0a4f02e5066c3d9a58427644010ab4f89b918614a1cd4c2b0)
  else()
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
      set(Onnxruntime_URL "${Onnxruntime_BASEURL}/onnxruntime-linux-aarch64-${Onnxruntime_VERSION}.tgz")
      set(Onnxruntime_HASH SHA256=dc4f841e511977c0a4f02e5066c3d9a58427644010ab4f89b918614a1cd4c2b0)
    else()
      set(Onnxruntime_URL "${Onnxruntime_BASEURL}/onnxruntime-linux-x64-gpu-${Onnxruntime_VERSION}.tgz")
      set(Onnxruntime_HASH SHA256=4d1c10f0b410b67261302c6e18bb1b05ba924ca9081e3a26959e0d12ab69f534)
    endif()
  endif()
else()
  set(Onnxruntime_URL "${CUSTOM_ONNXRUNTIME_URL}")
  set(Onnxruntime_HASH "${CUSTOM_ONNXRUNTIME_HASH}")
endif()

FetchContent_Declare(
  onnxruntime
  DOWNLOAD_EXTRACT_TIMESTAMP 1
  URL ${Onnxruntime_URL}
  URL_HASH ${Onnxruntime_HASH})
FetchContent_MakeAvailable(onnxruntime)

add_library(Ort INTERFACE)
set(ONNXRUNTIME_INCLUDE_DIRS "${onnxruntime_SOURCE_DIR}/include")

if(APPLE)
  set(Onnxruntime_LIB "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.${Onnxruntime_VERSION}.dylib")

  target_link_libraries(Ort PRIVATE "${Onnxruntime_LIB}")
  target_include_directories(Ort PRIVATE "${onnxruntime_SOURCE_DIR}/include")
  target_sources(Ort PRIVATE "${Onnxruntime_LIB}")
  set_property(SOURCE "${Onnxruntime_LIB}" PROPERTY MACOSX_PACKAGE_LOCATION Frameworks)
  source_group("Frameworks" FILES "${Onnxruntime_LIB}")
  add_custom_command(
    TARGET Ort
    POST_BUILD
    COMMAND
      ${CMAKE_INSTALL_NAME_TOOL} -change "@rpath/libonnxruntime.${Onnxruntime_VERSION}.dylib"
      "@loader_path/../Frameworks/libonnxruntime.${Onnxruntime_VERSION}.dylib" $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
elseif(MSVC)
  set(Onnxruntime_LIB_NAMES onnxruntime;onnxruntime_providers_shared)
  foreach(lib_name IN LISTS Onnxruntime_LIB_NAMES)
    add_library(Ort::${lib_name} SHARED IMPORTED)
    set_target_properties(Ort::${lib_name} PROPERTIES IMPORTED_IMPLIB ${onnxruntime_SOURCE_DIR}/lib/${lib_name}.lib)
    set_target_properties(Ort::${lib_name} PROPERTIES IMPORTED_LOCATION ${onnxruntime_SOURCE_DIR}/lib/${lib_name}.dll)
    set_target_properties(Ort::${lib_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${onnxruntime_SOURCE_DIR}/include)
    target_link_libraries(Ort INTERFACE Ort::${lib_name})
    install(FILES ${onnxruntime_SOURCE_DIR}/lib/${lib_name}.dll DESTINATION "${CMAKE_INSTALL_LIBDIR}/")
  endforeach()
else()
  set(Onnxruntime_LINK_LIBS "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so.${Onnxruntime_VERSION}")
  set(Onnxruntime_ADDITIONAL_LIBS
  "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime_providers_shared.so"
  "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so" "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so.1")
  
  target_link_libraries(Ort INTERFACE "${Onnxruntime_LINK_LIBS}")

  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(Onnxruntime_INSTALL_LIBS ${Onnxruntime_LINK_LIBS} ${Onnxruntime_ADDITIONAL_LIBS})
  else()
    set(Onnxruntime_INSTALL_LIBS ${Onnxruntime_LINK_LIBS} ${Onnxruntime_ADDITIONAL_LIBS})
  endif()
  install(FILES ${Onnxruntime_INSTALL_LIBS} DESTINATION "${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}")
  set_target_properties(Ort PROPERTIES INSTALL_RPATH "$ORIGIN/${CMAKE_PROJECT_NAME}")
endif()
