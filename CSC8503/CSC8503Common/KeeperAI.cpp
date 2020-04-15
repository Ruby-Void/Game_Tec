#include "KeeperAI.h"

using namespace NCL::CSC8503;

KeeperAI::KeeperAI(const Vector3 position, const Vector3 roamTo, const std::string& filename, GoosePlayer* goose) : GameObject(KEEPER_AI, LAYER_FOUR) {
	InitialiseParkKeeper();
	spawnLocation = position;
	roamingLocation = roamTo;

	grid = new NavigationGrid(filename);
	path = new NavigationPath();
	state = new StateMachine();

	// Keeper Patrols Level
	StateFunc PatrolFunc = [](void* keeper, void* goose) {
		KeeperAI* keeperData = (KeeperAI*)keeper;
		GoosePlayer* gooseData = (GoosePlayer*)goose;

		NavigationGrid* grid = keeperData->GetGrid();
		NavigationPath* path = keeperData->GetPath();

		if (path->IsEmpty()) {
			bool found = false;
			while (!found) {
				Vector3 keeperPos = keeperData->GetTransform().GetWorldPosition();
				keeperPos.y = 0.0f;
				if (keeperData->GetReverse()) {
					found = grid->FindPath(keeperPos, keeperData->GetSpawnLocation(), *path);
				}
				else {
					found = grid->FindPath(keeperPos, keeperData->GetRoamLocation(), *path);
				}
			}
			path->PopWaypoint(keeperData->GetNavigationPoint());
		}
	};

	// Keeper Hunts Goose
	StateFunc HuntFunc = [](void* keeper, void* goose) {
		KeeperAI* keeperData = (KeeperAI*)keeper;
		GoosePlayer* gooseData = (GoosePlayer*)goose;

		NavigationGrid* grid = keeperData->GetGrid();
		NavigationPath* path = keeperData->GetPath();

		bool pathFound = false;

		if (keeperData->GetTime() > 2.0f) {
			Vector3 keeperPos = keeperData->GetTransform().GetWorldPosition();
			keeperPos.y = 0.0f;
			Vector3 goosePos = gooseData->GetTransform().GetWorldPosition();
			goosePos.y = 0.0f;

			pathFound = grid->FindPath(keeperPos, goosePos, *path);
			if (keeperData->GetTime() != 0.0f) {
				keeperData->SetTime(0.0f);
			}
		}
		if (pathFound) {
			path->PopWaypoint(keeperData->GetNavigationPoint());
		}
	};

	GenericState* patrol	= new GenericState(PatrolFunc, this, goose);
	GenericState* hunt		= new GenericState(HuntFunc, this, goose);

	state->AddState(patrol);
	state->AddState(hunt);

	typedef bool(*transitionfunc)(void*, void*);
	transitionfunc patrolToHuntFunc = [](void* keeper, void* goose) {
		KeeperAI* keeperData = (KeeperAI*)keeper;
		GoosePlayer* gooseData = (GoosePlayer*)goose;
		if ((keeperData->GetTransform().GetWorldPosition() - gooseData->GetTransform().GetWorldPosition()).Length() < 30.0f) {
			keeperData->GetPath()->Clear();
			return true;
		}
		else {
			return false;
		}
	};

	typedef bool(*transitionfunc)(void*, void*);
	transitionfunc huntToPatrolFunc = [](void* keeper, void* goose) {
		KeeperAI* keeperData = (KeeperAI*)keeper;
		GoosePlayer* gooseData = (GoosePlayer*)goose;
		if ((keeperData->GetTransform().GetWorldPosition() - gooseData->GetTransform().GetWorldPosition()).Length() > 60.0f) {
			keeperData->GetPath()->Clear();
			return true;
		}
		else {
			return false;
		}
	};

	GenericTransition<void*, void*>* huntToPatrolTran = new GenericTransition<void*, void*>(huntToPatrolFunc, this, goose, hunt, patrol);
	GenericTransition<void*, void*>* patrolToHuntTran = new GenericTransition<void*, void*>(patrolToHuntFunc, this, goose, patrol, hunt);

	state->AddTransition(huntToPatrolTran);
	state->AddTransition(patrolToHuntTran);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	SetBoundingVolume((CollisionVolume*)volume);

	GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	GetTransform().SetWorldPosition(position);

	SetRenderObject(new RenderObject(&GetTransform(), keeperMesh, nullptr, keeperShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(inverseMass);
	GetPhysicsObject()->InitCubeInertia();
}

KeeperAI::~KeeperAI() {
	delete keeperMesh;
	delete keeperTex;
	delete keeperShader;
}

void KeeperAI::UpdateGameObject(float dt) {	
	time += dt;
	state->Update();
	MoveParkKeeper();
}

void KeeperAI::InitialiseParkKeeper() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("CharacterA.msh", &keeperMesh);

	keeperTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	keeperShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

void KeeperAI::MoveParkKeeper() {
	Vector3 currentPos = GetTransform().GetWorldPosition();
	currentPos.y = 0.0f;

	Vector3 relativePos = navigationPoint - currentPos;

	if (relativePos.Length() < 2.5f) {
		path->PopWaypoint(navigationPoint);
	}

	Vector3 direction = relativePos.Normalised();

	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), RadiansToDegrees(atan2(direction.x, direction.z))));
	GetPhysicsObject()->AddForce(direction * movementSpeed);

	if ((GetTransform().GetWorldPosition() - GetRoamLocation()).Length() < 5.0f) {
		reverse = true;
	}

	if ((GetTransform().GetWorldPosition() - GetSpawnLocation()).Length() < 5.0f) {
		reverse = false;
	}
}