#include "AppleObject.h"

using namespace NCL::CSC8503;

AppleObject::AppleObject(const Vector3 position) {
	InitialiseApple();
	SphereVolume* volume = new SphereVolume(0.7f);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform().SetWorldScale(Vector3(4.0f, 4.0f, 4.0f));
	GetTransform().SetWorldPosition(position);
	SetRenderObject(new RenderObject(&GetTransform(), appleMesh, nullptr, appleShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));
	GetPhysicsObject()->SetInverseMass(1.0f);
	GetPhysicsObject()->InitSphereInertia();
}

AppleObject::~AppleObject() {
	delete appleMesh;
	delete appleTex;
	delete appleShader;
}

void AppleObject::UpdateGameObject(float dt) {

}

void AppleObject::OnCollisionBegin(GameObject* o) {
	if (o->GetName() == GOOSE_PLAYER) {
		std::cout << "Hit" << std::endl;
	}
}

void AppleObject::InitialiseApple() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("Apple.msh", &appleMesh);

	appleTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	appleShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}
