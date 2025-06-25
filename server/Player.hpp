#pragma once
#include <mysqlx/xdevapi.h>
#include "myStruct.hpp"

/// <summary>
/// 플레이어 클래스, 템플릿 입니다. 구현필요
/// </summary>

class Player {
private:
	std::string id;
	std::string name;
	Pos pos;
public:
	Player();
	void set_from_db(const mysqlx::Row& row);
	PlayerInfo get_player_data();
};

