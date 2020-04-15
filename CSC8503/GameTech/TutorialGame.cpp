#include "TutorialGame.h"

using namespace NCL::CSC8503;

TutorialGame::TutorialGame() : forceMagnitude(10.0f), useGravity(true) {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);
	physics->UseGravity(useGravity);

	InitialiseAssets();
	InitialiseCamera();
	InitialiseNetwork();
	InitialiseMenu();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete basicTex;
	delete basicShader;
	delete physics;
	delete renderer;
	delete world;

	NetworkBase::Destroy();
}

void TutorialGame::UpdateGame(float dt) {
	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);
	renderer->Render();

	if (selectMode) {
		LockedMenuCamera();
		char mode = SelectObject();
		if (mode != ' ') {
			if (mode == 'G') {
				selectMode = false;
				Window::GetWindow()->ShowOSPointer(false);
				Window::GetWindow()->LockMouseToWindow(true);
				InitialiseWorld(mode);
			}
			else if (mode == 'O') {
				selectMode = false;
				Window::GetWindow()->ShowOSPointer(false);
				Window::GetWindow()->LockMouseToWindow(true);
				InitialiseWorld(mode);
			}
		}
	}	
	else {
		server->SendGlobalPacket(StringPacket("Server says hello!")); // Message Server
		client->SendPacket(StringPacket("Client says hello!")); // Message Client
		server->UpdateServer();
		client->UpdateClient();
	}
}

void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

void TutorialGame::InitialiseNetwork() {
	NetworkBase::Initialise();
	int port = NetworkBase::GetDefaultPort();
	serverReceiver = new GamePacketReceiver("Server");
	clientReceiver = new GamePacketReceiver("Client");	
	server = new GameServer(port, 1);
	client = new GameClient();
	server->RegisterPacketHandler(String_Message, serverReceiver);
	client->RegisterPacketHandler(String_Message, clientReceiver);
	client->Connect(127, 0, 0, 1, port);
}

void TutorialGame::InitialisePlayers(char controlled) {
	if (controlled == 'G') {
		goosePlayer = new GoosePlayer(world->GetMainCamera());
		world->AddGameObject(goosePlayer);
	}
	if (controlled == 'O') {
		obsticalPlayer = new ObsticalPlayer(world, physics);
		world->AddGameObject(obsticalPlayer);
	}	
}

void TutorialGame::InitialiseMenu() {
	Vector3 dimensions = Vector3(15.0f, 15.0f, 15.0f);
	AABBVolume* cubeVolume = new AABBVolume(dimensions);
	SphereVolume* sphereVolume = new SphereVolume(5.0f);

	GameObject* gooseMode	 = new GameObject(GOOSE_MODE, LAYER_ONE);	
	gooseMode->SetBoundingVolume((CollisionVolume*)cubeVolume);
	gooseMode->GetTransform().SetWorldPosition(Vector3(25.0f, 5.0f, 5.0f));
	gooseMode->GetTransform().SetWorldScale(dimensions);
	gooseMode->SetRenderObject(new RenderObject(&gooseMode->GetTransform(), cubeMesh, basicTex, basicShader));
	gooseMode->SetPhysicsObject(new PhysicsObject(&gooseMode->GetTransform(), gooseMode->GetBoundingVolume()));
	gooseMode->GetPhysicsObject()->SetInverseMass(0.0f);
	gooseMode->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(gooseMode);

	GameObject* obsticalMode = new GameObject(OBSTICAL_MODE, LAYER_ONE);
	obsticalMode->SetBoundingVolume((CollisionVolume*)sphereVolume);
	obsticalMode->GetTransform().SetWorldPosition(Vector3(-25.0f, 5.0f, 5.0f));
	obsticalMode->GetTransform().SetWorldScale(dimensions);
	obsticalMode->SetRenderObject(new RenderObject(&obsticalMode->GetTransform(), sphereMesh, basicTex, basicShader));
	obsticalMode->SetPhysicsObject(new PhysicsObject(&obsticalMode->GetTransform(), obsticalMode->GetBoundingVolume()));
	obsticalMode->GetPhysicsObject()->SetInverseMass(0.0f);
	obsticalMode->GetPhysicsObject()->InitSphereInertia();
	world->AddGameObject(obsticalMode);
}

void TutorialGame::InitialiseWorld(char controlled) {
	physics->Clear();
	world->ClearAndErase();

	if (controlled != ' ') {
		InitialisePlayers(controlled);

		if (controlled == 'G') {
			controlledPlayer = goosePlayer;
			goosePlayer->SetActive(true);
		}
		else if (controlled == 'O') {
			controlledPlayer = obsticalPlayer;
			obsticalPlayer->SetActive(true);
		}
	}

	world->AddGameObject(AddCubeToWorld(Vector3(0.0f, -2.0f, 0.0f), Vector3(100.0f, 2.0f, 100.0f), 0.0f, FLOOR_OBJECT, LAYER_ONE)); // Floor
	AddBarriersToWorld();

	const string file = "test.txt";
	if (controlledPlayer && controlledPlayer == goosePlayer) {
		GenerateLevelFromFile(file);
		goosePlayer->GetTransform().SetWorldPosition(world->GetSpawnPoint());
	}
}

void TutorialGame::InitialiseCamera() {
	world->GetMainCamera()->SetNearPlane(0.5f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60.0f, 40.0f, 60.0f));
}

void TutorialGame::LockedMenuCamera() {
	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	world->GetMainCamera()->SetPosition(Vector3(0.0f, 240.0f, 0.0f));
	world->GetMainCamera()->SetPitch(-90.0f);
	world->GetMainCamera()->SetYaw(-180.0f);
	world->GetMainCamera()->BuildViewMatrix();
}

char TutorialGame::SelectObject() {
	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			GameObject* selectionObject = (GameObject*)closestCollision.node;
			
			if (selectionObject->GetName() == OBSTICAL_MODE) {
				return 'O';
			}
			else if (selectionObject->GetName() == GOOSE_MODE) {
				return 'G';
			}
		}
	}
	return ' ';
}

void TutorialGame::AddBarriersToWorld() {
	Vector3 horizontalBarrierSize = Vector3(90.0f, 5.0f, 5.0f);
	Vector3 verticalBarrierSize = Vector3(5.0f, 5.0f, 100.0f);
	AABBVolume* horizontalVolume = new AABBVolume(horizontalBarrierSize);
	AABBVolume* verticalVolume = new AABBVolume(verticalBarrierSize);

	// North Barrier
	GameObject* barrierNorth = new GameObject(BARRIER_OBJECT, LAYER_ZERO);
	barrierNorth->SetBoundingVolume((CollisionVolume*)horizontalVolume);
	barrierNorth->GetTransform().SetWorldScale(horizontalBarrierSize);
	barrierNorth->GetTransform().SetWorldPosition(Vector3(0.0f, 5.0f, -95.0f));
	barrierNorth->SetRenderObject(new RenderObject(&barrierNorth->GetTransform(), cubeMesh, basicTex, basicShader));
	barrierNorth->SetPhysicsObject(new PhysicsObject(&barrierNorth->GetTransform(), barrierNorth->GetBoundingVolume()));
	barrierNorth->GetPhysicsObject()->SetInverseMass(0.0f);
	barrierNorth->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(barrierNorth);

	// South Barrier
	GameObject* barrierSouth = new GameObject(BARRIER_OBJECT, LAYER_ZERO);
	barrierSouth->SetBoundingVolume((CollisionVolume*)horizontalVolume);
	barrierSouth->GetTransform().SetWorldScale(horizontalBarrierSize);
	barrierSouth->GetTransform().SetWorldPosition(Vector3(0.0f, 5.0f, 95.0f));
	barrierSouth->SetRenderObject(new RenderObject(&barrierSouth->GetTransform(), cubeMesh, basicTex, basicShader));
	barrierSouth->SetPhysicsObject(new PhysicsObject(&barrierSouth->GetTransform(), barrierSouth->GetBoundingVolume()));
	barrierSouth->GetPhysicsObject()->SetInverseMass(0.0f);
	barrierSouth->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(barrierSouth);

	// East Barrier	   
	GameObject* barrierEast = new GameObject(BARRIER_OBJECT, LAYER_ZERO);
	barrierEast->SetBoundingVolume((CollisionVolume*)verticalVolume);
	barrierEast->GetTransform().SetWorldScale(verticalBarrierSize);
	barrierEast->GetTransform().SetWorldPosition(Vector3(95.0f, 5.0f, 0.0f));
	barrierEast->SetRenderObject(new RenderObject(&barrierEast->GetTransform(), cubeMesh, basicTex, basicShader));
	barrierEast->SetPhysicsObject(new PhysicsObject(&barrierEast->GetTransform(), barrierEast->GetBoundingVolume()));
	barrierEast->GetPhysicsObject()->SetInverseMass(0.0f);
	barrierEast->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(barrierEast);

	// West Barrier
	GameObject* barrierWest = new GameObject(BARRIER_OBJECT, LAYER_ZERO);
	barrierWest->SetBoundingVolume((CollisionVolume*)verticalVolume);
	barrierWest->GetTransform().SetWorldScale(verticalBarrierSize);
	barrierWest->GetTransform().SetWorldPosition(Vector3(-95.0f, 5.0f, 0.0f));
	barrierWest->SetRenderObject(new RenderObject(&barrierWest->GetTransform(), cubeMesh, basicTex, basicShader));
	barrierWest->SetPhysicsObject(new PhysicsObject(&barrierWest->GetTransform(), barrierWest->GetBoundingVolume()));
	barrierWest->GetPhysicsObject()->SetInverseMass(0.0f);
	barrierWest->GetPhysicsObject()->InitCubeInertia();
	world->AddGameObject(barrierWest);
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, string name, uint8_t layer) {
	GameObject* cube = new GameObject(name, layer);
	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(dimensions);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	return cube;
}

GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject("sphere", LAYER_ONE);
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);
	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));
	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();
	return sphere;
}

void TutorialGame::GenerateLevelFromFile(const std::string& filename) {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	// Positioning
	const float offset = 90.0f;
	const float dimension = nodeSize / 2;	

	Vector3 keeperPos;

	for (size_t z = 0; z < gridWidth; ++z) {
		for (size_t x = 0; x < gridHeight; ++x) {
			char type;
			infile >> type;

			float positionX = ((float)x + 1.0f) * nodeSize - (offset + dimension);
			float positionY = 5.0f;
			float positionZ = ((float)z + 1.0f) * nodeSize - (offset + dimension);

			Vector3 position = Vector3(positionX, positionY, positionZ);

			if (type == 'C') {
				world->AddAdditionStorage(AddCubeToWorld(position, Vector3(5.0f, 5.0f, 5.0f), 0.0f, CUBE_OBJECT, LAYER_ONE));
			}
			else if (type == 'S') {
				world->AddAdditionStorage(AddSphereToWorld(position, 5.0f, 10.0f));
			}
			else if (type == 'A') {
				world->AddAdditionStorage(new AppleObject(position));
			}
			else if (type == 'K') {				
				keeperPos = position;
			}
			else if (type == 'R') {
				world->SetRoamingPoint(position);
			}
			else if (type == 'X') {
				world->SetSpawnPoint(position);			
			}
		}
	}

	world->AddAdditionStorage(new KeeperAI(keeperPos, world->GetRoamingPoint(), filename, goosePlayer));
}