#pragma once
#include "myStruct.hpp"
struct Pos {
	double x;
	double y;

	Pos()
		: x(0.f)
		, y(0.f)
	{
	}

	Pos(double _x, double _y)
		: x(_x)
		, y(_y)
	{
	}

	std::string GetString()
	{
		std::string str = std::to_string(x) + " " + std::to_string(y);
		return str;
	}
};

class Player {
public:
	std::string nickname;
	Pos position;

	Player()
		: nickname("")
		, position({ 0.f,0.f })
	{
	}
	Player(std::string _nickname, Pos _pos)
		: nickname(_nickname)
		, position(_pos)
	{
	}

	std::string get_PlayerPos()
	{
		std::string msg = position.GetString();
		return msg;
	}
};