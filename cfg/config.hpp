#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace Config {
    extern nlohmann::json config;

    void setDirectory(std::wstring directory);
    void init();
    void shutdown();
    void save();

    template <typename T> T *get(const std::string key, T def) {
        auto k = nlohmann::json::json_pointer(key);

        if (!config.contains(k)) {
            config[k] = def;
        }

        return config[k].get_ptr<T *>();
    }
} // namespace Config
