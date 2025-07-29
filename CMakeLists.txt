cmake_minimum_required(VERSION 3.16)
project(HttpApi VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
if(MSVC)
    add_compile_options(/W4)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Create the main library
add_library(httpapi STATIC
    src/http_server.cpp
    src/router.cpp
    src/middleware.cpp
    src/request.cpp
    src/response.cpp
    src/json_handler.cpp
    src/static_files.cpp
    src/utils.cpp
)

# Link Windows libraries
target_link_libraries(httpapi
    ws2_32
    iphlpapi
)

# Set include directories for the library
target_include_directories(httpapi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Create the main executable
add_executable(httpapi_app examples/main.cpp)
target_link_libraries(httpapi_app httpapi) 