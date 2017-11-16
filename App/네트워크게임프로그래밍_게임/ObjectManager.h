#pragma once
#include "Object.h"

class ObjectManager
{
private:
	std::vector<Player> m_playerList;
	std::vector<Bullet> m_bulletList;
	std::vector<Item>	m_itemList;

	std::vector<Tile>	m_tileList;
public:
	ObjectManager();
	~ObjectManager();

	void initialize();

	void addBullet(float x, float y, int team);
	void reloadAmmo(int team);
	void addItem(float x, float y);

	void changePlayerDirection(unsigned char key, int state, int team);
	int getPlayerDirection(unsigned char key, int team);

	void update(float elapsedTime);
	void render();

};
