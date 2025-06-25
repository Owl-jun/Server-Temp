#include "pch.h"
#include "RedisManager.hpp"

struct RedisReplyDeleter {
    void operator()(redisReply* reply) const {
        if (reply) freeReplyObject(reply);
    }
};

RedisManager& RedisManager::GetInstance() {
    static RedisManager instance;
    return instance;
}

bool RedisManager::Connect(const std::string& host, int port) {
    redisContext* raw = redisConnect(host.c_str(), port);
    if (!raw || raw->err)
    {
        std::cout << "REDIS Connect FAILED !" << std::endl;
        return false;
    }
    std::cout << "REDIS Connected !" << std::endl;

    commandContext = std::shared_ptr<redisContext>(raw, [](redisContext* ctx) {
        if (ctx) redisFree(ctx);
        });
    return true;
}

bool RedisManager::Set(const std::string& key, const std::string& value, int ttl) {
    if (!commandContext) return false;

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        ttl > 0
        ? (redisReply*)redisCommand(commandContext.get(), "SETEX %s %d %s", key.c_str(), ttl, value.c_str())
        : (redisReply*)redisCommand(commandContext.get(), "SET %s %s", key.c_str(), value.c_str()));

    return reply && reply->type == REDIS_REPLY_STATUS && std::string(reply->str) == "OK";
}

std::optional<std::string> RedisManager::Get(const std::string& key) {
    if (!commandContext) return std::nullopt;

    std::unique_ptr<redisReply, RedisReplyDeleter> reply(
        (redisReply*)redisCommand(commandContext.get(), "GET %s", key.c_str()));

    if (!reply || reply->type != REDIS_REPLY_STRING)
        return std::nullopt;

    return std::string(reply->str);
}

