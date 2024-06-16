#pragma once

#include <concepts>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

namespace Config {
    extern nlohmann::json config;

    void setDirectory(std::wstring directory);
    void init(HMODULE handle = nullptr);
    void shutdown();
    void save();

    template <typename T> struct field {
        field() = default;
        field(nlohmann::json::json_pointer k, std::shared_ptr<T> ptr) : k(k), ptr(ptr) {}

        // How to shoot yourself in the foot

        T *operator->() const {
            config[k] = *ptr;
            return ptr.get();
        }

        T operator*() {
            config[k] = *ptr;
            return *ptr;
        }

        T *operator&() const {
            config[k] = *ptr;
            return ptr.get();
        }

        T operator=(const T &val) {
            *ptr = val;
            config[k] = *ptr;

            return val;
        }

        T get() const { return *ptr; }

      private:
        nlohmann::json::json_pointer k;
        std::shared_ptr<T> ptr;
    };

    template <typename T> Config::field<T> get(const std::string key, T def) {
        auto k = nlohmann::json::json_pointer(key);

        if (!config.contains(k)) {
            config[k] = def;
        }

        auto ptr = std::make_shared<T>(config[k].get<T>());

        field<T> ret = {k, ptr};

        return ret;
    }
} // namespace Config
