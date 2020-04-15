#pragma once
#include "../CSC8503Common/GameClient.h"
#include <iostream>

class GamePacketReceiver : public PacketReceiver {
public:
	GamePacketReceiver(std::string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) { // Source -1 Server & source 0 client
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			std::string msg = realPacket->GetStringFromData();
			std::cout << name << " received message : " << msg << std::endl;
		}
	}
protected:
	std::string name;
};