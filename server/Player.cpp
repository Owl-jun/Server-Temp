#include "pch.h"
#include "Player.hpp"

Player::Player()
	: nickname("")
, position({ 0.f,0.f })
{
}

Player::Player(std::string _nickname, Pos _pos)
	: nickname(_nickname)
	, position(_pos)
{
}

std::string Player::get_PlayerPos()
{
	std::string msg = position.GetString();
	return msg;
}