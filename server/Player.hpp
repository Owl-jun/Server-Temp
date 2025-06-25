#pragma once
#include <mysqlx/xdevapi.h>
#include "myStruct.hpp"

/// <summary>
/// �÷��̾� Ŭ����, ���ø� �Դϴ�. �����ʿ�
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

