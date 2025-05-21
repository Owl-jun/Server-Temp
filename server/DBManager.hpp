#pragma once

#include "myStruct.hpp"
#include "utils.hpp"
#include <iostream>
#include <mysqlx/xdevapi.h>



struct DBConData {
	DBConData() {
		set_dbInfo(ip, port, userId, pwd);
		std::cout << "[DBManager] IP: " << ip
			<< ", Port: " << port
			<< ", ID: " << userId
			<< ", PWD: " << pwd << std::endl;

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

	mysqlx::Session session;		 // 전용 세션 풀
	std::thread worker;              // 전용 스레드 풀

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
		std::cout << "DB Connection Suc !!" << std::endl;
		worker = std::thread([this]() { run(); });
	}
};



