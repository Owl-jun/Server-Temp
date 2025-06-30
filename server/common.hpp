#pragma once

const int MAX_PACKET_SIZE = 2048;

//			FSM			//

//		OPER CODE		//
enum class Opcode
{
	LOGIN = 0x01,
	MOVE = 0x02,
	ATTACK = 0x03,
	LOGOUT = 0x04,
	PING = 0x09
};

//		RECV STATE		//
enum class ReadState
{
	ReadingHeader,
	ReadingBody
};

//		LOGIN STATE		//
enum class LoginState
{
	LOGIN = 0x00,
	LOGOUT = 0x01
};

