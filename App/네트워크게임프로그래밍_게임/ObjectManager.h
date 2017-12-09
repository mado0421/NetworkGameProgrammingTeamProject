#pragma once
#include "Object.h"
extern HANDLE hCommunicateEvent;
extern HANDLE hUpdateEvent;
extern C2SPacket c2spacket;
extern S2CPacket s2cpacket;
class ObjectManager
{
private:
	std::vector<Player> m_playerList;
	//std::vector<Bullet> m_bulletList;
	Bullet m_myBulletList[MAX_BULLET];
	Bullet m_OtherBulletList[MAX_BULLET * 3];
	std::vector<Item>	m_itemList;
	std::vector<Tile>	m_tileList;
	int m_myTeamNo;
public:
	ObjectManager();
	~ObjectManager();

	void initialize(int team);

	void addBullet(float x, float y, int team);
	void reloadAmmo(int team);
	void addItem(float x, float y);

	void changePlayerDirection(unsigned char key, int state, int team);
	int getPlayerDirection(unsigned char key, int team);

	void update(float elapsedTime);
	void render();

	void updatePlayerInfo();
	void updatePlayerInfoFirst();
	void setPlayerNum(int number) { m_myTeamNo = number; }
	Bullet* getBulletList() { return m_myBulletList; }
};