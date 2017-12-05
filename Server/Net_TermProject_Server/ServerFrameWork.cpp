#include "stdafx.h"
#include "ServerFrameWork.h"


//extern int ats;
//extern Room room[MAXROOMCOUNT];

//정의?
Room ServerFrameWork::room[MAXROOMCOUNT];
bool ServerFrameWork::Communicated[MAXROOMCOUNT][MAX_PLAYER];
HANDLE ServerFrameWork::hCommunicated[MAXROOMCOUNT][MAX_PLAYER];
HANDLE ServerFrameWork::hSendPacket[MAXROOMCOUNT][MAX_PLAYER];

HANDLE ServerFrameWork::hroom[2];
ServerFrameWork::ServerFrameWork()
{
	ZeroMemory(room, sizeof(Room)*MAXROOMCOUNT);
	hroom[0] = CreateEvent(NULL, FALSE, TRUE, NULL);
	hroom[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
}

ServerFrameWork::~ServerFrameWork()
{
}



void ServerFrameWork::SetSocket(int RoomNumber, int PlayerId,SOCKET socket)
{
	room[RoomNumber].m_teamList[PlayerId].m_socket = socket;
	//	TimeOut 옵션 필요할 것 같음
}

void ServerFrameWork::InitRoom(int RoomNumber)
{
	//	Make Event
	for (int i = 0; i < MAX_PLAYER; ++i) {
		hCommunicated[RoomNumber][i] = CreateEvent(NULL, FALSE, FALSE, NULL);
		hSendPacket[RoomNumber][i] = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
}

void ServerFrameWork::GameStart(int RoomNumber)
{
	for (int i = 0; i < MAX_PLAYER; ++i) {
		SetEvent(hSendPacket[RoomNumber][i]);
		//NOTUSESLEEP
		Sleep(1000);
	}
	SetEvent(hSendPacket[RoomNumber][PUBLIC_EVENT]);

	room[RoomNumber].timeInit();
}
int cnt = 0;
DWORD ServerFrameWork::GameThread(LPVOID arg)
{
	int RoomNumber = (int)arg;
	DWORD retEvent;
	while (true) {
		WaitForSingleObject(hroom[RoomNumber], INFINITE);
		if (cnt++ >= 1000)
			break;
		::printf("\n cnt = %d\n", cnt);
	
		retEvent = WaitForMultipleObjects(MAX_PLAYER, hCommunicated[RoomNumber], TRUE, INFINITE);
		if (retEvent != WAIT_OBJECT_0)break;

		//	Need Room TimeCheckout
		while (true) {
			room[RoomNumber].Tick();
			if (THREADFREQ > room[RoomNumber].m_ElapsedTime)
				Sleep(1);
			else {
				room[RoomNumber].m_ElapsedTime = 0;
				break;
			}
		}

		//::printf("In GameThread room: %d\n", RoomNumber);
		//	Communication Success?
		//	{
		//
		//	}
		InfoTeam team[MAX_PLAYER];
		/*for (int i = 0; i < MAX_PLAYER; ++i) {
			memcpy(&team[i], &room[RoomNumber].m_teamList[i], sizeof(InfoTeam));
		}*/
		memcpy(&team, &room[RoomNumber].m_teamList, sizeof(InfoTeam)*MAX_PLAYER);

		InfoPlayer iPlayer[MAX_PLAYER];
		InfoBullet iBullet[MAX_PLAYER][MAX_BULLET];
		for (int i = 0; i < MAX_PLAYER; ++i)
		{
			iPlayer[i] = team[i].m_player;
			memcpy(iBullet[i], team[i].m_bullets, sizeof(InfoBullet)*MAX_BULLET);
		}

		// Calc;

		S2CPacket* packet = new S2CPacket;
		::ZeroMemory(packet, sizeof(S2CPacket));

		// Set packet
		packet->SetPacket(RoomNumber, room[RoomNumber]);

		SendPacketToClient(packet, RoomNumber);
		::printf("SendPacketToClient %d \n", RoomNumber);

		delete packet;
		
		for (int i = 0; i < MAX_PLAYER; ++i)
			SetEvent(hSendPacket[RoomNumber][i]);

		SetEvent(hroom[RoomNumber == 1 ? 0 : 1]);
	}
	//	Set Next Thread's Event
	return 0;
}

DWORD ServerFrameWork::CommunicationPlayer(LPVOID arg)
{
	Room_Player* index = (Room_Player*)arg;
	int index_p = index->playerNum, index_r = index->roomNum;
	int retval;
	DWORD retEvent;
	while (true) {
	
		retEvent = WaitForSingleObject(hSendPacket[index_r][index_p], INFINITE);
		if (retEvent != WAIT_OBJECT_0)break;

		retval = ReceivePacketFromClient(index_r, index_p);

		if (retval != SOCKET_ERROR)
			::printf("CommunicationPlayer %d %d retval=%d\n", index_r, index_p, retval);
		else
			::printf("CommunicationPlayer %d %d SOCKET_ERROR\n", index_r, index_p);

		SetEvent(hCommunicated[index_r][index_p]);
		
		if (cnt >= 1000)break;
	}
	return 0;
}

int ServerFrameWork::ReceivePacketFromClient(int roomNum, int PlayerID)
{
	InfoTeam *team = &room[roomNum].m_teamList[PlayerID];
	C2SPacket packet;
	ZeroMemory(&packet, sizeof(C2SPacket));
	int retval;
	retval = recvn(team->m_socket, (char*)&packet, sizeof(C2SPacket), 0);
	if (retval == SOCKET_ERROR)
		return SOCKET_ERROR;
	printf("ReceivePacketFromClient room:%d, PlayerId:%d, retval = %d\n", roomNum, PlayerID, retval);
	memcpy(&team->m_player, &packet.player, sizeof(InfoPlayer));
	memcpy(&team->m_bullets, &packet.Bullets, sizeof(InfoBullet)*MAX_BULLET);

	return retval;
}

void ServerFrameWork::SendPacketToClient(S2CPacket * packet, int roomNum)
{
	InfoTeam teamList[MAX_PLAYER];
	memcpy(&teamList, room[roomNum].m_teamList, sizeof(InfoTeam)*MAX_PLAYER);

	SOCKET client_sock[MAX_PLAYER];
	for (int i = 0; i < MAX_PLAYER; ++i){
		client_sock[i] = teamList[i].m_socket;
	}

	//	ConnectCheck? , Need Thread?
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		packet->SendTime = chrono::system_clock::now();
		send(client_sock[i], (char*)packet, sizeof(S2CPacket), 0);
	}
}


