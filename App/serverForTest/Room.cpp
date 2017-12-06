#include "Room.h"
#include "h.h"



Room::Room()
{
}


Room::~Room()
{
}

void Room::initialize()
{
	/*for Test*/
	for (int i = 0; i < MAX_PLAYER; ++i)
		m_teamList[i].m_socket = NULL;
}

bool Room::checkMsg(int playerNum)
{
	char msg[MSGSIZE];
	int retval;
	retval = recvn(m_teamList[playerNum].m_socket, msg, MSGSIZE, 0);
	if (retval == SOCKET_ERROR) return false;
	switch (msg[0])
	{
	case msg::STARTPLAY:
		msg[0] = msg::STARTPLAY;
		retval = send(m_teamList[playerNum].m_socket, msg, MSGSIZE, 0);
		if (retval == SOCKET_ERROR) return false;
		return true;

	case msg::TEST:
		msg[0] = msg::OK;
		retval = send(m_teamList[playerNum].m_socket, msg, MSGSIZE, 0);
		if (retval == SOCKET_ERROR) return false;
		return true;

	case msg::LEAVE:
		m_teamList[playerNum].m_socket = NULL;
		return false;

	case msg::OK:
		printf("%d번째 애가 OK래..\n", playerNum);
		return true;

	default:
		printf("이걸 나한테 왜 보내?\n");
		return false;
	}
	return false;
}

void Room::playerLeave()
{

}

int Room::playerArrive(const SOCKET &client_sock)
{
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (m_teamList[i].m_socket == NULL)
		{
			m_teamList[i].m_socket = client_sock;
			return i;
		}
	}
	return -1;
}
