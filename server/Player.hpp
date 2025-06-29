#pragma once
#include <mysqlx/xdevapi.h>
#include "myStruct.hpp"


class Player {
private:
	int id;
	std::string name;
	Pos pos;
public:
	bool isSet = false;
	Player();
	void set_from_db(mysqlx::Row row);
	void set_pos(double x, double y);
	void set_name(std::string _name);
	std::string get_name() { return name; }
	std::string get_pos();
};

