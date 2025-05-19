#pragma once
#include "myStruct.hpp"

/// <summary>
/// 플레이어 클래스, 템플릿 입니다. 구현필요
/// </summary>

class Player {
public:
	std::string nickname;
	Pos position;

	Player();
	Player(std::string _nickname, Pos _pos);

	std::string get_PlayerPos();

};
