#pragma once
#include "Scene.h"

enum SceneType
{
	Title=0,
	Lobby,
	Play
};


GLvoid initialize(GLvoid);
GLvoid run(GLvoid);

GLvoid renderScene(GLvoid);
GLvoid reshape(int w, int h);
GLvoid idle(GLvoid);
GLvoid mouseInput(int button, int state, int x, int y);
GLvoid keyDown(unsigned char key, int x, int y);
GLvoid keyUp(unsigned char key, int x, int y);
GLvoid specialKeyDown(int key, int x, int y);
GLvoid specialKeyUp(int key, int x, int y);

class Framework
{
private:
	Scene*	m_scenes[3];

	DWORD	prevTime;
	DWORD	curTime;
	Scene*	m_currentScene=nullptr;
public:
	Framework();
	~Framework();

public:
	GLvoid initialize(int argc, char **argv);
	GLvoid run(GLvoid);

	GLvoid renderScene(GLvoid);
	GLvoid reshape(int w, int h);
	GLvoid idle(GLvoid);
	GLvoid mouseInput(int button, int state, int x, int y);
	GLvoid keyDown(unsigned char key, int x, int y);
	GLvoid keyUp(unsigned char key, int x, int y);
	GLvoid specialKeyDown(int key, int x, int y);
	GLvoid specialKeyUp(int key, int x, int y);

	bool changeScene(int idx);
	void updatePlayerInfo(InfoPlayer* p)
	{
		if(m_currentScene!=nullptr)
			((PlayScene*)m_currentScene)->updatePlayerInfo(p);
	}
};

