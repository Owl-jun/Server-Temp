#pragma once

// �⺻ STL
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

// �α�
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

// ��Ʈ��ũ
#include <asio.hpp>

// TSL
#include <asio/ssl.hpp>

// ���ӽ����̽�
using asio::ip::tcp;
using json = nlohmann::json;

const int MAX_PACKET_SIZE = 2048;
