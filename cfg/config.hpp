#pragma once

#include <concepts>
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>

namespace Config {
    extern nlohmann::json config;

    void setDirectory(std::wstring directory);
    void init();
    void shutdown();
    void save();

    template <typename T>
    concept isVectorOrSet =
        std::is_same_v<T, std::vector<typename T::value_type>> || std::is_same_v<T, std::set<typename T::value_type>>;

    template <typename T> struct field {
        field() = default;
        template <isVectorOrSet T> field(nlohmann::json::json_pointer k, T val) : k(k) { ptr = new T(val); }
        field(nlohmann::json::json_pointer k, T *ptr) : k(k), ptr(ptr) {}

        T *operator->() const { return ptr; }

        T &operator*() const {
            config[k] = *ptr;

            return *ptr;
        }

        T *operator&() const { return ptr; }

        std::string operator=(const std::string &val) {
            *ptr = val;
            config[k] = *ptr;

            return val;
        }

      private:
        nlohmann::json::json_pointer k;
        T *ptr;
    };

    template <isVectorOrSet T> Config::field<T> get(const std::string key, T def) {
        auto k = nlohmann::json::json_pointer(key);

        if (!config.contains(k)) {
            config[k] = def;
        }

        auto realPtr = config[k].get<T>();

        field<T> ret = {k, realPtr};

        return ret;
    }

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
