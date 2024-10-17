cmake_minimum_required(VERSION 3.26.0)
project(akcore)

if(MSVC)
    SET(CMAKE_GENERATOR_PLATFORM x64)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS OFF)

# Add your source files and headers
set(Headers 
    utils.h
    ark_types.h
    filters/video_filter.h
    filters/video_filter_manager.h
    filters/ai_renderer_video_filter.h
    parameters/parameter.h
    parameters/param_cache.h
    host/video_host_delegate.h
    host/video_host.h
    host/draw_helper.h
    images/ark_image.h
    images/image_buffer.h
    images/image_utils.h
    main_api_connection/main_api_connection.h
    main_api_connection/plugin_json_parser.h
)
set(Sources 
    utils.cpp
    logging/spdlogger.cpp
    logging/logger.cpp
    filters/video_filter_manager.cpp
    filters/ai_renderer_video_filter.cpp
    parameters/parameter.cpp
    parameters/param_cache.cpp
    host/video_host.cpp
    images/image_buffer.cpp
    images/image_utils.cpp
    main_api_connection/main_api_connection.cpp
    main_api_connection/plugin_json_parser.cpp
)

# Create your main library
add_library(${PROJECT_NAME} STATIC 
    ${Sources} 
    ${Headers}
)

# Add the subdirectory containing spdlog
add_subdirectory(../external/spdlog ../external/spdlog/build)

# Include directories for spdlog
include_directories(../external/spdlog/include)

# Add the subdirectory containing libcpr
set(BUILD_SHARED OFF CACHE BOOL "Build shared libraries" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(USE_SYSTEM_CURL ON CACHE BOOL "Use system libcurl" FORCE)
set(CURL_STATICLIB ON CACHE BOOL "Build libcurl as static library" FORCE)
add_subdirectory(../external/cpr ../external/cpr/build)

# Include directories for libcpr
include_directories(../external/cpr/include)

# Link libcpr statically
target_link_libraries(${PROJECT_NAME}
    cpr # Link libcpr
)

# Add the subdirectory containing sentry
add_subdirectory(../external/sentry ${CMAKE_BINARY_DIR}/sentry)

# Include directories for sentry
include_directories(../external/sentry/include)

# Link sentry
target_link_libraries(${PROJECT_NAME}
    sentry
)

include_directories(../external/rapidjson/include)
include_directories(../external/stb)
include_directories(.)
include_directories(logging)
include_directories(filters)
include_directories(host)
include_directories(images)
include_directories(parameters)
include_directories(main_api_connection)

# Create a custom target for spdlog shared library
add_custom_target(SPDLOG_SHARED_LIB)
add_dependencies(SPDLOG_SHARED_LIB spdlog)
