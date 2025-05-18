#include "pch.h"
#include "Server.hpp"
#include "QueueManager.h"
#include "SessionManager.hpp"
int main() {
	SetConsoleOutputCP(CP_UTF8);
	set_debug_log();

	// 네트워크 관련 비동기 작업 등록
	asio::io_context io_context;
	Server server(io_context, 9000);
	
	// 로직 처리 관련 매니저
	QueueManager& QM = QueueManager::GetInstance();
	SessionManager& SM = SessionManager::GetInstance();

	// 네트워크 통신 담당 쓰레드 1~2개
	std::vector<std::thread> NetThreads;
	for (int i = 0; i < 2; ++i) {
		NetThreads.emplace_back(std::thread([&io_context] { io_context.run(); }));
	}
	spdlog::info("네트워크 스레드 실행완료");

	// 로직 처리 담당 쓰레드 4개
	std::vector<std::thread> WorkerThreads;
	for (int i = 0; i < 4; ++i) {
		WorkerThreads.emplace_back(std::thread([&QM] { QM.run(); }));
	}
	spdlog::info("로직 스레드 실행완료");
	


	// 일하러 나가신 부모님이 와야 잠들 수 있는 main 쓰레드녀석 ..
	for (auto& NetThread : NetThreads) {
		NetThread.join();
	}
	std::cout << "[main] : 네트워크 스레드 퇴근함" << std::endl;

	for (auto& workerthread : WorkerThreads) {
		workerthread.join();
		std::cout << "[main] : 워커 스레드 퇴근함" << std::endl;
	}
	std::cout << "[main] : 모든 스레드 퇴근함" << std::endl;

	return 0;
}


