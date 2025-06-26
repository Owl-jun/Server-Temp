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
	void set_pos(double x, double y);
	std::string get_name() { return name; }
};

