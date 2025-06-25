#pragma once

const int MAX_PACKET_SIZE = 2048;

//			FSM			//
//		OPER CODE		//
enum class Opcode
{
	LOGIN = 0x01,
	MOVE = 0x02,
	ATTACK = 0x03
};

//		RECV STATE		//
enum class ReadState
{
	ReadingHeader,
	ReadingBody
};


