#pragma once
#include <mysqlx/xdevapi.h>

/// <summary>
/// �÷��̾� Ŭ����, ���ø� �Դϴ�. �����ʿ�
/// </summary>

class Player {
private:
	std::string id;
	std::string name;
	std::string phone;
public:
	Player();
	void set_from_db(const mysqlx::Row& row);
};

