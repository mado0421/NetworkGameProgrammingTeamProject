#include "stdafx.h"
#include "ServerFrameWork.h"

//정의
Room ServerFrameWork::room[MAXROOMCOUNT];

HANDLE ServerFrameWork::hCommunicated[MAXROOMCOUNT][MAX_PLAYER];
HANDLE ServerFrameWork::hSendPacket[MAXROOMCOUNT][MAX_PLAYER];

HANDLE ServerFrameWork::hGameThread[MAXROOMCOUNT];
HANDLE ServerFrameWork::hNextThreadCall;
std::queue<int> ServerFrameWork::m_ThreadQueue;

#define TEST_THREAD_COUNT 1
#define TEST_CNT_COUNT	10000
//int cnt = 0;

ServerFrameWork::ServerFrameWork()
{
	ZeroMemory(room, sizeof(Room)*MAXROOMCOUNT);

	HANDLE hTmp;
	hTmp = CreateThread(NULL, 0, ThreadOrder, 0, 0, NULL);
	CloseHandle(hTmp);

	hNextThreadCall = CreateEvent(NULL, FALSE, FALSE, NULL);
}

ServerFrameWork::~ServerFrameWork()
{
}

void ServerFrameWork::SetSocket(int RoomNumber, int PlayerId,SOCKET socket)
{
	room[RoomNumber].m_teamList[PlayerId].m_socket = socket;
	//	TimeOut 옵션 필요할 것 같음
}

void ServerFrameWork::InitRoom(int roomIndex)
{
	//	Make Event
	for (int i = 0; i < MAX_PLAYER; ++i) {
		hCommunicated[roomIndex][i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hSendPacket[roomIndex][i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	hGameThread[roomIndex] = CreateEvent(NULL, FALSE, FALSE, NULL);

	Room_Player p[MAX_PLAYER];

	HANDLE hTmp;
	for (int i = 0; i < MAX_PLAYER; ++i) {
		p[i].roomNum = roomIndex;
		p[i].playerNum = i;
		hTmp = CreateThread(NULL, 0, CommunicationPlayer, (LPVOID)&p[i], 0, NULL);
		CloseHandle(hTmp);
	}
	hTmp = CreateThread(NULL, 0, GameThread, (LPVOID)roomIndex, 0, NULL);
	CloseHandle(hTmp);
	
	room[roomIndex].m_roomState = Play;
}

void ServerFrameWork::GameStart(int roomIndex)
{
	for (int i = 0; i < MAX_PLAYER; ++i) {
		SetEvent(hSendPacket[roomIndex][i]);
		//NOTUSESLEEP
		//Sleep(1000);
	}
	m_ThreadQueue.push(roomIndex);
	SetEvent(hNextThreadCall);
	room[roomIndex].timeInit();
	printf("room: %d GameStart\n", roomIndex);
}

void ServerFrameWork::GameEnd(int roomIndex)
{
	// Set packet
	SOCKET client_sock[MAX_PLAYER];
	S2CPacket packet;

	::ZeroMemory(&packet, sizeof(S2CPacket));
	packet.SetPacket(roomIndex, room[roomIndex]);

	for (int i = 0; i < MAX_PLAYER; ++i) {
		client_sock[i] = TeamList(roomIndex, i).m_socket;
	}

	for (int i = 0; i < MAX_PLAYER; ++i){
		//packet.Message = end_data;
		send(client_sock[i], (char*)&packet, sizeof(S2CPacket), 0);
	}
	for (int i = 0; i < MAX_PLAYER; ++i)
		SetEvent(hSendPacket[roomIndex][i]);
	room[roomIndex].m_roomState = closing;

	CloseRoom(roomIndex);
	printf("room %d의 게임이 종료 CloseRoom\n", roomIndex);
}

void ServerFrameWork::CloseRoom(int roomIndex)
{
	for (int i = 0; i < MAX_PLAYER; ++i) {
		CloseHandle(hCommunicated[roomIndex][i]);
		CloseHandle(hSendPacket[roomIndex][i]);
		closesocket(TeamList(roomIndex, i).m_socket);
	}
	CloseHandle(hGameThread[roomIndex]);
	room[roomIndex].m_roomState = Lobby;
}

DWORD ServerFrameWork::ThreadOrder(LPVOID arg)
{
	int index;
	DWORD retval;
	while (true) {
		retval = WaitForSingleObject(hNextThreadCall, INFINITE);
		if (retval != WAIT_OBJECT_0)break;

		if (!m_ThreadQueue.empty()) 
		{
			index = m_ThreadQueue.front();
			m_ThreadQueue.pop();

			if (room[index].m_roomState == Play) 
			{
				m_ThreadQueue.push(index);
				SetEvent(hGameThread[index]);
			}
			else 
			{
				SetEvent(hNextThreadCall);
			}
		}
	}
	return 0;
}

DWORD ServerFrameWork::GameThread(LPVOID arg)
{
	int roomIndex = (int)arg;
	DWORD retEvent;
	int retCalc;
	while (true)
	{
		//	wait queue
		WaitForSingleObject(hGameThread[roomIndex], INFINITE);
		
		//	wait Communication
		retEvent = WaitForMultipleObjects(MAX_PLAYER, hCommunicated[roomIndex], TRUE, 100);

		switch (retEvent)
		{
		case WAIT_ABANDONED_0 + 0:
			printf("room: %d의 %d 번째 플레이어는 접속이 끊김\n", roomIndex, 0);
			break;
		case WAIT_ABANDONED_0 + 1:
			printf("room: %d의 %d 번째 플레이어는 접속이 끊김\n", roomIndex, 1);
			break;
		case WAIT_ABANDONED_0 + 2:
			printf("room: %d의 %d 번째 플레이어는 접속이 끊김\n", roomIndex, 2);
			break;
		case WAIT_ABANDONED_0 + 3:
			printf("room: %d의 %d 번째 플레이어는 접속이 끊김\n", roomIndex, 3);
			break;
		default:
			break;
		}
		

#ifdef FIXFREQUENCY
		//	Fix FrameRate with THREADFREQ
		FixFrame(roomIndex);
#endif
		// Calc;
		retCalc = Calculate(roomIndex);
		if (retCalc == end_of_game)
		{
			printf("room:%d retCalc=end_of_game\n",roomIndex);
			break;
		}

		// Set packet
		S2CPacket packet;
		::ZeroMemory(&packet, sizeof(S2CPacket));
		packet.SetPacket(roomIndex, room[roomIndex]);

		//	Send
		SendPacketToClient(&packet, roomIndex);

		//	For CommunicationPlayer Thread, notice ready to Communicate
		for (int i = 0; i < MAX_PLAYER; ++i)
			SetEvent(hSendPacket[roomIndex][i]);

		//	Set Next Thread's
		SetEvent(hNextThreadCall);
	}
	GameEnd(roomIndex);

	return 0;
}

DWORD ServerFrameWork::CommunicationPlayer(LPVOID arg)
{
	Room_Player* index = (Room_Player*)arg;
	int playerID = index->playerNum, roomIndex = index->roomNum;
	int retval;
	DWORD retEvent;
	while (true) 
	{
		retEvent = WaitForSingleObject(hSendPacket[roomIndex][playerID], INFINITE);
		if (retEvent != WAIT_OBJECT_0)
		{
			printf("room:%d , Player:%d hSendPacket의 retEvet가 WAIT_OBJECT가 아님\n");
			break;
		}
		if (room[roomIndex].m_roomState != Play) {
			printf("room:%d 플레이 종료된 CommnunicationPlayer 스레드 종료\n", roomIndex);
			break;
		}

		retval = ReceivePacketFromClient(roomIndex, playerID);

		if (retval == SOCKET_ERROR)
		{
			closesocket(TeamList(roomIndex, playerID).m_socket);
			CloseHandle(hCommunicated[roomIndex][playerID]);
			TeamList(roomIndex, playerID).m_player.m_hp = 0;
			if (room[roomIndex].m_roomState != closing)
				printf("room:%d , Player:%d 가 비정상적인 방법으로 종료됨\n", roomIndex, playerID);
			return 0;
		 }
		SetEvent(hCommunicated[roomIndex][playerID]);
	}
	return 0;
}

int ServerFrameWork::ReceivePacketFromClient(int roomNum, int PlayerID)
{
	C2SPacket packet;
	ZeroMemory(&packet, sizeof(C2SPacket));
	int retval;

	retval = recvn(TeamList(roomNum, PlayerID).m_socket,(char*)&packet, sizeof(C2SPacket), 0);
	if (retval == SOCKET_ERROR)
		return SOCKET_ERROR;

	memcpy(&TeamList(roomNum, PlayerID).m_player, &packet.player, sizeof(InfoPlayer));
	memcpy(&TeamList(roomNum, PlayerID).m_bullets, &packet.Bullets, sizeof(InfoBullet)*MAX_BULLET);

	return retval;
}

void ServerFrameWork::SendPacketToClient(S2CPacket * packet, int roomNum)
{
	SOCKET client_sock[MAX_PLAYER];

	for (int i = 0; i < MAX_PLAYER; ++i){
		client_sock[i] = TeamList(roomNum,i).m_socket;
	}

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		//packet->SendTime = chrono::system_clock::now();
		//packet->Message = data;
		send(client_sock[i], (char*)packet, sizeof(S2CPacket), 0);
	}
}

int ServerFrameWork::Calculate(int roomNum)
{
	InfoPlayer* player;
	InfoBullet* bullet;
	int dead = 0;

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		player = &TeamList(roomNum, i).m_player;

		//	PlayerCheck
		if (IsPlayerDead(player->m_hp))
		{
			//	플레이어가 한명 남은경우 게임을 종료
			if (++dead == MAX_PLAYER - 1)
				return end_of_game;

			// 죽은 플레이어는 계산하지 않고 다음 플레이어를 본다.
			continue;
		}

		//	Bullet Check;
		for (int j = 0; j < MAX_PLAYER; ++j)
		{
			//	동일 플레이어 건너띔
			if (i == j)continue;

			for (int k = 0; k < MAX_BULLET; ++k)
			{
				bullet = &TeamList(roomNum, j).m_bullets[k];
				if (!IsExistBullet(bullet->m_pos.x))continue;

				if (player->m_pos.x - PLAYERSIZE > bullet->m_pos.x + BULLETSIZE)continue;
				if (player->m_pos.x + PLAYERSIZE < bullet->m_pos.x - BULLETSIZE)continue;
				if (player->m_pos.y - PLAYERSIZE > bullet->m_pos.y + BULLETSIZE)continue;
				if (player->m_pos.y + PLAYERSIZE < bullet->m_pos.y - BULLETSIZE)continue;

				//	Collide Test ok
				player->m_hp -= BULLETDAMAGE;
				if (IsPlayerDead(player->m_hp))dead++;
				DestroyBullet(bullet);
			}
		}
	}

	if (dead == MAX_PLAYER - 1)
		return end_of_game;
	return ok;
}

inline void ServerFrameWork::FixFrame(int roomIndex)
{
	while (true) {
		room[roomIndex].Tick();
		if (THREADFREQ > room[roomIndex].m_ElapsedTime)
			Sleep(1);
		else {
			room[roomIndex].m_ElapsedTime = 0;
			break;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////
/// 로비 ///
///////////////////////////////////////////////////////////////////////////////////

bool ServerFrameWork::arrivePlayer(SOCKET& socket, int roomID)
{
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (room[roomID].m_teamList[i].m_socket == NULL)
		{
			room[roomID].m_teamList[i].m_socket = socket;
			return true;
		}
	}
	return false;
}

int ServerFrameWork::findVocantRoom(SOCKET& socket)
{
	for (int i = 0; i < MAXROOMCOUNT; ++i)
	{
		/*room[i]에 모든 플레이어가 있는게 아니면*/
		if (room[i].m_roomState ==Lobby) {
			if (!room[i].checkAllPlayerInRoom())
			{
				/*그 룸에 지금 들어온 애를 넣고*/
				if (!room[i].playerArrive(socket)) return -2;

				/*만약 지금 애 들어가서 그 룸이 4명이 되었으면*/
				if (room[i].checkAllPlayerInRoom())
				{
					/*게임 스타트 시킴*/
					room[i].gameStart();
					room[i].m_roomState = Play;
				}
				return i;
			}
		}
	}
	return -1;
}


