#pragma once
#include "pch.h"
#include "Session.h"

struct Task {
	std::shared_ptr<Session> session;
	std::string message;
};



