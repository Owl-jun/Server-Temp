#include "pch.h"
#include "Server.hpp"
#include "QueueManager.h"
#include "SessionManager.hpp"

///////////////////////////////
// 스레드 풀 스레드 개수 설정
#define NET_CNT 3
#define WORK_CNT 4
///////////////////////////////

int main() {
	///////////////////////////////
	// 로그 , 터미널 관련 세팅	 
	// utils.hpp
	SetConsoleOutputCP(CP_UTF8); 
	set_debug_log();	
	///////////////////////////////

	// 싱글톤 객체 명시적 생성
	SessionManager& SM = SessionManager::GetInstance();
	QueueManager& QM = QueueManager::GetInstance();
	///////////////////////////////


	asio::io_context io_context;
	// Server.hpp : 클라이언트 연결관리
	Server server(io_context, 9000);	

	std::vector<std::thread> NetThreads;
	for (int i = 0; i < NET_CNT; ++i) {
		NetThreads.emplace_back(std::thread([&io_context] { io_context.run(); }));
	}
	spdlog::info("네트워크 스레드 실행완료");

	std::vector<std::thread> WorkerThreads;
	for (int i = 0; i < WORK_CNT; ++i) {
		WorkerThreads.emplace_back(std::thread([&QM] { QM.run(); }));
	}
	spdlog::info("일꾼 스레드 실행완료");


	// 일하러 나가신 부모님이 와야 잠들 수 있는 main 녀석 ..
	for (auto& NetThread : NetThreads) { NetThread.join(); }
	for (auto& workerthread : WorkerThreads) { workerthread.join(); }

	return 0;
}


