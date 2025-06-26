#pragma once
#include <mysqlx/xdevapi.h>
#include "myStruct.hpp"


class Player {
private:
	int id;
	std::string name;
	Pos pos;
public:
	Player();
	void set_from_db(mysqlx::Row row);
	PlayerInfo get_player_data();
	void set_pos(double x, double y);
};

