#pragma once
class Session;

///////////////////////////////////////


struct Task {
	std::shared_ptr<Session> session;
	std::string message;
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