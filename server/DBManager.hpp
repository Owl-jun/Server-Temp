#pragma once

#include "myStruct.hpp"
#include "utils.hpp"
#include <iostream>
#include <mysqlx/xdevapi.h>
#include <chrono>

#define DBWORKER_CNT 2

struct DBConData {
	DBConData() {
		set_dbInfo(ip, port, userId, pwd);
	};
	
	std::string ip;
	int port;
	std::string userId;
	std::string pwd;
};

class DBManager {
	
	std::mutex mtx;
	std::condition_variable cv;
	DBConData conData;

	std::queue<DBTask> queue;        // Task Queue
	// mysqlx::Session session;
	// std::thread worker;

	std::vector<mysqlx::Session> sessions;		 // 전용 세션 풀
	std::vector<std::thread> workers;              // 전용 스레드 풀

	bool running = true;

	void run(int idx) {
		while (running) {
			auto& session = sessions[idx];
			DBTask task;
			{
				std::unique_lock lock(mtx);
				cv.wait(lock, [&] { return !queue.empty(); });

				if (!running && queue.empty())  
					return;

				task = std::move(queue.front());
				queue.pop();
			}
			// 응답 시간 TEST ms
			auto start = std::chrono::high_resolution_clock::now();

			// 세션 풀 중 유휴 세션을 찾는 로직 구현해야함
			task.func(session);


			// 응답 시간 TEST ms
			auto end = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			std::cout << "Query Response Time : " << elapsed << std::endl;
			spdlog::info("[DBThread {}] query response time: {}us", idx, elapsed);
		}
	}

	void stop() {
		{
			std::lock_guard<std::mutex> lock(mtx);
			running = false;
		}

		cv.notify_all();  

		for (auto& worker : workers) {
			if (worker.joinable())
				worker.join();  
		}
	}

public:
	static DBManager& GetInstance() {
		static DBManager instance;
		return instance;
	}

	void PushTask(DBTask task) {
		{
			std::lock_guard lock(mtx);
			queue.push(std::move(task));
		}
		cv.notify_one();  
	}

private:
	DBManager() 
		: conData()
	{
		for (int i = 0; i < DBWORKER_CNT; ++i)
		{
			sessions.emplace_back(mysqlx::Session(conData.ip, conData.port, conData.userId, conData.pwd));
		}
		for (int j = 0; j < DBWORKER_CNT; ++j)
		{
			workers.emplace_back(std::thread([this,j]() { run(j); }));
		}
		
		std::cout << "DB Connected !" << std::endl;
	}
};



