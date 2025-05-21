#pragma once
#include <mysqlx/xdevapi.h>
class Session;

///////////////////////////////////////


class Task {
public:
	std::shared_ptr<Session> session;
	std::string message;
};

/// <summary>
/// 
/// </summary>
class DBTask {
public:
	//int sessionIndex;
	std::function<void(mysqlx::Session&)> func;
};

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