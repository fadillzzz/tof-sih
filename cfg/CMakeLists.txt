include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

add_library(Config STATIC
    config.cpp
)
target_precompile_headers(Config PRIVATE pch.hpp)
target_link_libraries(Config nlohmann_json::nlohmann_json)