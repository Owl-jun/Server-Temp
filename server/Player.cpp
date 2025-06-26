#include "pch.h"
#include "Player.hpp"
#include "DBManager.hpp"

Player::Player()
	: id()
	, name("")
	, pos()
{
}

void Player::set_from_db(mysqlx::Row row) 
{
	 id = row[0].get<int>();
	 name = row[1].get<std::string>();
	 double posx = row[7].get<double>();
	 double posy = row[8].get<double>();
	 pos.x = posx;
	 pos.y = posy;
}

PlayerInfo Player::get_player_data()
{
	PlayerInfo pi = { id,name,pos };
	return pi;
}

void Player::set_pos(double x, double y)
{
	pos.x = x;
	pos.y = y;
}
