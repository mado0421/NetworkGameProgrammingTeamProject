#pragma once
#include "ObjectManager.h"

#define MAX_KEY 256

class Framework;

class Scene
{
protected:
	Framework *m_pFramework;
public:
	Scene();
	Scene(Framework *pFramework);
	~Scene();
public:
	virtual void initialize() = 0;
	virtual void leave() = 0;

	virtual void update(float elapsedTime) = 0;
	virtual void render() = 0;

	virtual void mouseInput(int button, int state, int x, int y) = 0;
	virtual void keyDown(unsigned char key, int x, int y) = 0;
	virtual void keyUp(unsigned char key, int x, int y) = 0;
	virtual void specialKeyDown(int key, int x, int y) = 0;
	virtual void specialKeyUp(int key, int x, int y) = 0;

	virtual void changeScene(int idx);
};

class TitleScene : public Scene
{
private:
public:
	TitleScene();
	TitleScene(Framework *pFramework);
	~TitleScene();
public:
	virtual void initialize();
	virtual void leave();

	virtual void update(float elapsedTime);
	virtual void render();

	virtual void mouseInput(int button, int state, int x, int y);
	virtual void keyDown(unsigned char key, int x, int y);
	virtual void keyUp(unsigned char key, int x, int y);
	virtual void specialKeyDown(int key, int x, int y);
	virtual void specialKeyUp(int key, int x, int y);
};

class LobbyScene : public Scene
{
private:
public:
	InfoTeam	m_teamInfo[MAX_PLAYER];
	SOCKADDR_IN serveraddr;
	WSADATA		wsa;
	SOCKET		sock;
	int			retval;
	char		ipAddr[BUFSIZE];
	int			m_myTeamNo;

	//--------------------for Test----------------------
	bool		m_lightOn;
	bool		m_connected;
	//--------------------------------------------------
public:
	LobbyScene();
	LobbyScene(Framework *pFramework);
	~LobbyScene();
public:
	virtual void initialize();
	virtual void leave();

	virtual void update(float elapsedTime);
	virtual void render();

	virtual void mouseInput(int button, int state, int x, int y);
	virtual void keyDown(unsigned char key, int x, int y);
	virtual void keyUp(unsigned char key, int x, int y);
	virtual void specialKeyDown(int key, int x, int y);
	virtual void specialKeyUp(int key, int x, int y);

	virtual bool checkMsg();
	/*Room에 몇 번째로 입장했는지 알아야 몇 번 플레이어인지 알 수 있음*/
	virtual bool accessLobby();

	virtual void leaveServer();
};

class PlayScene : public Scene
{
private:
	ObjectManager* m_objMng;
public:
	PlayScene();
	PlayScene(Framework *pFramework);
	~PlayScene();
public:
	virtual void initialize();
	virtual void leave();

	virtual void update(float elapsedTime);
	virtual void render();

	virtual void mouseInput(int button, int state, int x, int y);
	virtual void keyDown(unsigned char key, int x, int y);
	virtual void keyUp(unsigned char key, int x, int y);
	virtual void specialKeyDown(int key, int x, int y);
	virtual void specialKeyUp(int key, int x, int y);
};