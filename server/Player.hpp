#pragma once
#include "myStruct.hpp"

/// <summary>
/// �÷��̾� Ŭ����, ���ø� �Դϴ�. �����ʿ�
/// </summary>

class Player {
public:
	std::string nickname;
	Pos position;

	Player();
	Player(std::string _nickname, Pos _pos);

	std::string get_PlayerPos();

};
