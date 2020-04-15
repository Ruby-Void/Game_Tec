#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GameClient.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

void TestStateMachine() {
	
}

// create new class
class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) { // Source -1 Server & source 0 client
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;
			string msg = realPacket->GetStringFromData();
			std::cout << name << " received message : " << msg << std::endl;
		}
	}
protected:
	string name;
};
void TestNetworking() {
	NetworkBase::Initialise();

	//Tuturial
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);
	// to here

	// Call in update.
	for (int i = 0; i < 100; ++i) {
		server->SendGlobalPacket(StringPacket("Server says hello!" + std::to_string(i))); // Message Server
		client->SendPacket(StringPacket("Client says hello!" + std::to_string(i))); // Message Client
		server->UpdateServer();
		client->UpdateClient();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// Destructor
	NetworkBase::Destroy();
}

vector<Vector3> testNodes;

void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");
	NavigationPath outPath;

	Vector3 startPos(80.0f, 0.0f, 10.0f), endPos(80.0f, 0.0f, 80.0f);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1], b = testNodes[i];
		Debug::DrawLine(a, b, Vector4(0.0f, 1.0f, 0.0f, 1.0f));
	}
}



/* 
The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that, instead. 
This time, we've added some extra functionality to the window class - we can hide or show the 
*/
int main() {
	Window* w = Window::CreateGameWindow("CSC8503 Game Technology!", 1720, 1000);

	if (!w->HasInitialised()) { return -1; }	

	//TestStateMachine();
	//TestNetworking();
	//TestPathfinding();
	
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();	

	w->GetTimer()->GetTimeDeltaSeconds();
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; // Must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		//DisplayPathfinding();

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}