#include "pch.h"
#include "LoginCache.hpp"

LoginCache& LoginCache::GetInstance()
{
	static LoginCache instance;
	return instance;
}

void LoginCache::Set(const std::string& key, const std::string& value, int ttl)
{
	std::cout << "LoginCache -> Set" << std::endl;
	ValueEntry val; 
	{
		std::lock_guard<std::mutex> lock(cacheMutex);
		val.value = value;
		val.expire = std::chrono::steady_clock::now() + std::chrono::seconds(ttl);
		valueCache[key] = val;
	}
}

std::optional<std::string> LoginCache::Get(const std::string& key)
{
	std::lock_guard<std::mutex> lock(cacheMutex);
	std::cout << "LoginCache -> Get" << std::endl;
	auto it = valueCache.find(key);
	if (it == valueCache.end())
		return std::nullopt;
	auto now = std::chrono::steady_clock::now();
	if (it->second.expire <= now) {
		// soft TTL 
		auto value = it->second.value;

		// sliding TTL
		it->second.expire = now + std::chrono::seconds(DEFAULT_TTL);
		return value;
	}

	return it->second.value;
}

void LoginCache::StartCleanupThread()
{
	std::cout << "LoginCache -> StartCleanupThread" << std::endl;

	if (cleanupThread.joinable()) return;
	{
		std::lock_guard<std::mutex> lock(cacheMutex);
		running = true;
	}
	cleanupThread = std::thread([this]() {
		std::unique_lock<std::mutex> lock(cvMutex);
		while (running)
		{
			Cleanup();
			cv.wait_for(lock, std::chrono::seconds(10), [&] { return !running; });
		}
	});
}

void LoginCache::StopCleanupThread()
{
	{
		std::lock_guard<std::mutex> lock(cacheMutex);
		running = false;
	}
	cv.notify_all();
	cleanupThread.join();
}

void LoginCache::Cleanup()
{

	std::cout << "LoginCache -> Cleanup" << std::endl;

	std::lock_guard<std::mutex> lock(cacheMutex);
	for (auto it = valueCache.begin(); it != valueCache.end(); ) 
	{
		if (it->second.expire <= std::chrono::steady_clock::now())
			it = valueCache.erase(it);  // erase는 다음 유효 iter 리턴
		else
			++it;
	}
}





