#pragma once

#include <optional>
#include <string>
#include <vector>

class ICache {
public:
    virtual void Set(const std::string& key, const std::string& value, int ttl = 60) = 0;
    //virtual void Append(const std::string& key, const std::string& value, int ttl = 60) = 0;
    //virtual std::optional<std::vector<std::string>> GetList(const std::string& key) = 0;
    virtual std::optional<std::string> Get(const std::string& key) = 0;
    virtual void Cleanup() = 0;
    virtual ~ICache() {}
};