add_library(Config STATIC
    config.cpp
)
target_link_libraries(Config nlohmann_json::nlohmann_json)