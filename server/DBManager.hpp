#pragma once

#include "utils.hpp"
#include <iostream>
#include <mysqlx/xdevapi.h>

struct DBConData {
	DBConData() {
		set_dbInfo(ip, port, userId, pwd);
	};

	std::string ip;
	int port;
	std::string userId;
	std::string pwd;
};

class DBTask {
public:
	std::function<void(mysqlx::Session&)> func;
};

class DBManager {
	std::thread worker;              // 전용 스레드 (task 실행 loop)
	std::queue<DBTask> queue;        // 외부에서 요청 push
	std::mutex mtx;
	std::condition_variable cv;
	DBConData conData;
	mysqlx::Session session;
	bool running = true;

	void run() {
		while (running) {
			DBTask task;
			{
				std::unique_lock lock(mtx);
				cv.wait(lock, [&] { return !queue.empty(); });
				task = std::move(queue.front());
				queue.pop();
			}
			task.func(session);  
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
		, session(conData.ip, conData.port, conData.userId, conData.pwd)
		
	{
		worker = std::thread([this]() { run(); });
	}
};



