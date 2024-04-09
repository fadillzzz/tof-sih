#pragma once

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
        field(nlohmann::json::json_pointer k, T *ptr) : k(k), ptr(ptr) {}

        T *operator->() const { return ptr; }

        T &operator*() const {
            // Wildest hack of 2024
            // This lets us save the file whenever the pointer is being dereferenced.
            // Ideally this only executes when there's an assignment, but I could
            // not figure out how to do that.
            config[k] = *ptr;
            save();

            return *ptr;
        }

        T *operator&() const { return ptr; }

        template <typename U> U *operator=(const U &val) {
            *ptr = val;

            save();

            return (U *)ptr;
        }

      private:
        nlohmann::json::json_pointer k;
        T *ptr;
    };

    template <typename T> Config::field<T> get(const std::string key, T def) {
        auto k = nlohmann::json::json_pointer(key);

        if (!config.contains(k)) {
            config[k] = def;
        }

        auto realPtr = config[k].get_ptr<T *>();

        field<T> ret = {k, realPtr};

        return ret;
    }
} // namespace Config
