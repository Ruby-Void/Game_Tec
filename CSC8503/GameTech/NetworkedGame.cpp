#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#define COLLISION_MSG 30

using namespace NCL::CSC8503;

NetworkedGame::NetworkedGame() {

}

NetworkedGame::~NetworkedGame() {
	
}

void NetworkedGame::StartAsServer() {

}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) {

}

void NetworkedGame::UpdateGame(float dt) {

}

void NetworkedGame::SpawnPlayer() {

}

void NetworkedGame::StartLevel() {

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {

}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {

}

void NetworkedGame::UpdateAsServer(float dt) {

}

void NetworkedGame::UpdateAsClient(float dt) {
	//ClientPacket newPacket;

	//if (Window::GetKeyboard()->KeyPressed()) {
	//	// fire button pressed !
	//	newPacket.buttonstates[0] = 1;
	//	newPacket.lastID = 0; // You ’ll need to work this out somehow ...
	//}
	//thisClient->SendPacket(newPacket);
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector < GameObject* >::const_iterator first;
	std::vector < GameObject* >::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		int playerState = 0; // You ’ll need to do this bit !
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket); // change ...
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {

}