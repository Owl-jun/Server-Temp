#include "pch.h"
#include "Player.hpp"
#include "DBManager.hpp"

Player::Player()
	: id()
	, name("")
	, pos()
{
}

void Player::set_from_db(const mysqlx::Row& row) 
{
	 id = row[0].get<std::string>();
	 name = row[1].get<std::string>();
	 double posx = row[2].get<double>();
	 double posy = row[3].get<double>();
	 pos.x = posx;
	 pos.y = posy;
}

PlayerInfo Player::get_player_data()
{
	PlayerInfo pi = { id,name,pos };
	return pi;
}
