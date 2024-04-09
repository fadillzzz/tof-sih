#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace Config {
    extern nlohmann::json config;

    void setDirectory(std::wstring directory);
    void init();
    void shutdown();
    void save();

    template <typename T> struct field {
        field(T *ptr) : ptr(ptr) {}

        T *operator->() const { return ptr; }

        template <typename U> U *operator=(const U &val) {
            *ptr = val;

            save();

            return (U *)ptr;
        }

      private:
        T *ptr;
    };

    template <typename T> Config::field<T> get(const std::string key, T def) {
        auto k = nlohmann::json::json_pointer(key);

        if (!config.contains(k)) {
            config[k] = def;
        }

        auto realPtr = config[k].get_ptr<T *>();

        field<T> ret = {realPtr};

        return ret;
    }
} // namespace Config
