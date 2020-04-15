#include "ObsticalPlayer.h"

using namespace NCL::CSC8503;

ObsticalPlayer::ObsticalPlayer(GameWorld* world, PhysicsSystem* physics) : GameObject("Obstical", LAYER_SEVEN) {
	this->world = world;	
	this->physics = physics;
	this->camera = world->GetMainCamera();
	InitialiseAssets();

	Vector3 cubeDims = Vector3(5.0f, 5.0f, 5.0f);
	Vector3 volumeDims = Vector3(4.0f, 4.0f, 4.0f);
	AABBVolume* volume = new AABBVolume(volumeDims);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform().SetWorldPosition(objectPosition);
	GetTransform().SetWorldScale(cubeDims);

	SetRenderObject(new RenderObject(&GetTransform(), cubeMesh, basicTex, basicShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume(), true));
}

ObsticalPlayer::~ObsticalPlayer() {
	delete cubeMesh;
	delete sphereMesh;
	delete gooseMesh;
	delete keeperMesh;
	delete appleMesh;

	delete basicTex;
	delete basicShader;

	world = nullptr;
	physics = nullptr;
	camera = nullptr;
}

void ObsticalPlayer::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		  , &cubeMesh);
	loadFunc("sphere.msh"	  , &sphereMesh);
	loadFunc("goose.msh"	  , &gooseMesh);
	loadFunc("CharacterA.msh" , &keeperMesh);
	loadFunc("CharacterM.msh" , &roamMesh);
	loadFunc("Apple.msh"	  , &appleMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}

void ObsticalPlayer::UpdateGameObject(float dt) {
	if (active) {
		if (!GetRenderObject()) {
			SetRenderObject(new RenderObject(&GetTransform(), cubeMesh, basicTex, basicShader));
		}

		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::T)) {
			toggleDeleteMode();
		}

		camera->SetPosition(Vector3(0.0f, 240.0f, 0.0f));
		camera->SetPitch(-90.0f);
		camera->SetYaw(-180.0f);
		camera->BuildViewMatrix();

		ManipulateObject();
		AddObjectToWorld();

		if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::RETURN)) {
			world->OutputLevelToFile();
		}
	}	
	else {
		SetRenderObject(nullptr);
	}
}

void ObsticalPlayer::OnCollisionEnd(GameObject* otherObject) {
	SetCollidedObject(otherObject);
}

void ObsticalPlayer::ManipulateObject() {
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::W)) {		
		if (!(objectPosition.z >= 85.0f)) {
			objectPosition += Vector3(0.0f, 0.0f, 10.0f);
		}
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::S)) {
		if (!(objectPosition.z <= -85.0f)) {
			objectPosition -= Vector3(0.0f, 0.0f, 10.0f);
		}
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::A)) {
		if (!(objectPosition.x >= 85.0f)) {
			objectPosition += Vector3(10.0f, 0.0f, 0.0f);
		}
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::D)) {
		if (!(objectPosition.x <= -85.0f)) {
			objectPosition -= Vector3(10.0f, 0.0f, 0.0f);			
		}
	}		
	GetTransform().SetWorldPosition(objectPosition);
}

void ObsticalPlayer::AddObjectToWorld() {
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM1)) {
		SetRenderObject(new RenderObject(&GetTransform(), cubeMesh, basicTex, basicShader));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM2)) {
		SetRenderObject(new RenderObject(&GetTransform(), sphereMesh, basicTex, basicShader));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM3)) {
		SetRenderObject(new RenderObject(&GetTransform(), appleMesh, basicTex, basicShader));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM4)) {
		SetRenderObject(new RenderObject(&GetTransform(), keeperMesh, basicTex, basicShader));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM5)) {
		SetRenderObject(new RenderObject(&GetTransform(), roamMesh, basicTex, basicShader));
	}
	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::NUM6)) {
		SetRenderObject(new RenderObject(&GetTransform(), gooseMesh, basicTex, basicShader));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::E)) {
		if (!deleteMode) {
			if (!GetCollidedObject() || GetCollidedObject()->GetTransform().GetWorldPosition() != objectPosition) {
				if (GetRenderObject()->GetMesh() == cubeMesh) {
					world->AddAdditionStorage(AddCubeToWorld(objectPosition));
				}
				else if (GetRenderObject()->GetMesh() == sphereMesh) {
					world->AddAdditionStorage(AddSphereToWorld(objectPosition));
				}
				else if (GetRenderObject()->GetMesh() == appleMesh) {
					world->AddAdditionStorage(AddAppleToWorld(objectPosition));
				}
				else if (GetRenderObject()->GetMesh() == keeperMesh) {
					world->AddAdditionStorage(AddKeeperToWorld(objectPosition));
				}
				else if (GetRenderObject()->GetMesh() == roamMesh) {
					world->AddAdditionStorage(AddRoamToWorld(objectPosition));
					world->SetRoamingPoint(objectPosition);
				}
				else if (GetRenderObject()->GetMesh() == gooseMesh) {
					world->SetSpawnPoint(objectPosition);
				}
			}			
		}
		else {
			if (GetCollidedObject()) {
				if (GetCollidedObject()->GetName() != FLOOR_OBJECT && GetCollidedObject()->GetName() != BARRIER_OBJECT &&
					GetCollidedObject()->GetName() != GOOSE_PLAYER && GetCollidedObject()->GetName() != OBSTICAL_PLAYER) {
					world->AddRemovalStorage(GetCollidedObject());
					SetCollidedObject(nullptr);
					physics->Clear();
				}
			}
		}
	}
}

GameObject* ObsticalPlayer::AddCubeToWorld(const Vector3& position) {
	Vector3 cubeDims = Vector3(5.0f, 5.0f, 5.0f);
	GameObject* cube = new GameObject(CUBE_OBJECT, LAYER_ONE);
	AABBVolume* volume = new AABBVolume(cubeDims);
	cube->SetBoundingVolume((CollisionVolume*)volume);
	cube->GetTransform().SetWorldPosition(position);
	cube->GetTransform().SetWorldScale(cubeDims);
	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));
	cube->GetPhysicsObject()->SetInverseMass(0.0f);
	return cube;
}

GameObject* ObsticalPlayer::AddSphereToWorld(const Vector3& position, float inverseMass) {
	float sphereRadius = 5.0f;
	GameObject* sphere = new GameObject(SPHERE_OBJECT, LAYER_ONE);

	Vector3 sphereSize = Vector3(sphereRadius, sphereRadius, sphereRadius);
	SphereVolume* volume = new SphereVolume(sphereRadius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);
	sphere->GetTransform().SetWorldScale(sphereSize);
	sphere->GetTransform().SetWorldPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	return sphere;
}

GameObject* ObsticalPlayer::AddAppleToWorld(const Vector3& position) {
	GameObject* apple = new GameObject(APPLE_OBJECT, LAYER_THREE);

	SphereVolume* volume = new SphereVolume(0.7f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform().SetWorldScale(Vector3(4.0f, 4.0f, 4.0f));
	apple->GetTransform().SetWorldPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), appleMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	return apple;
}

GameObject* ObsticalPlayer::AddKeeperToWorld(const Vector3& position) {
	GameObject* keeper = new GameObject(KEEPER_AI, LAYER_FOUR);
	float meshSize = 4.0f, inverseMass = 0.5f, movementSpeed = 20.0f;
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	keeper->SetBoundingVolume((CollisionVolume*)volume);

	keeper->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	keeper->GetTransform().SetWorldPosition(position);

	keeper->SetRenderObject(new RenderObject(&keeper->GetTransform(), keeperMesh, nullptr, basicShader));
	keeper->SetPhysicsObject(new PhysicsObject(&keeper->GetTransform(), keeper->GetBoundingVolume()));

	keeper->GetPhysicsObject()->SetInverseMass(inverseMass);
	keeper->GetPhysicsObject()->InitCubeInertia();
	return keeper;
}

GameObject* ObsticalPlayer::AddRoamToWorld(const Vector3& position) {
	GameObject* roam = new GameObject(ROAM_LOCATION, LAYER_FOUR);
	float meshSize = 4.0f, inverseMass = 0.5f, movementSpeed = 20.0f;
	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	roam->SetBoundingVolume((CollisionVolume*)volume);

	roam->GetTransform().SetWorldScale(Vector3(meshSize, meshSize, meshSize));
	roam->GetTransform().SetWorldPosition(position);

	roam->SetRenderObject(new RenderObject(&roam->GetTransform(), roamMesh, nullptr, basicShader));
	roam->SetPhysicsObject(new PhysicsObject(&roam->GetTransform(), roam->GetBoundingVolume()));

	roam->GetPhysicsObject()->SetInverseMass(inverseMass);
	roam->GetPhysicsObject()->InitCubeInertia();
	return roam;
}

Vector3 ObsticalPlayer::AddSpawnToWorld(const Vector3& position) {
	return position;
}
