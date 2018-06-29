#pragma once

#include <string>

using std::string;

class Player
{
public:
	Player();
	~Player();
	int id;
	string name;
	int posX;
	int posY;
	int faction;
	bool isInvisible;
};

