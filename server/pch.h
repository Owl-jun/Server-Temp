#pragma once

// 기본 STL
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>


// json
#include <nlohmann/json.hpp>

// 로그
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

// 네트워크
#include <asio.hpp>

// TSL
#include <asio/ssl.hpp>

// 네임스페이스
using asio::ip::tcp;
using json = nlohmann::json;

const int MAX_PACKET_SIZE = 2048;
