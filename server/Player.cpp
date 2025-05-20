#include "pch.h"
#include "Player.hpp"
#include "DBManager.hpp"

Player::Player()
	: id()
	, name("")
	, phone("")
{
}

void Player::set_from_db(const mysqlx::Row& row) 
{
	 //id = row[0].get<std::string>();
	 //name = row[1].get<std::string>();
	 //phone = row[2].get<std::string>();
}