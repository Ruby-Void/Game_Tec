#include "GoosePlayer.h"

using namespace NCL::CSC8503;

GoosePlayer::GoosePlayer(Camera* camera, const Vector3 position) : GameObject("Goose", LAYER_TWO) {
	InitialiseGoose();

	SphereVolume* volume = new SphereVolume(size);
	SetBoundingVolume((CollisionVolume*)volume);
	GetTransform().SetLocalPosition(position);

	GetTransform().SetWorldScale(Vector3(size, size, size));

	SetRenderObject(new RenderObject(&GetTransform(), gooseMesh, nullptr, gooseShader));
	SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

	GetPhysicsObject()->SetInverseMass(inverseMass);
	GetPhysicsObject()->InitSphereInertia();

	mainCamera = camera;
	cameraPos = GetTransform().GetWorldPosition() - lockedOffset;
}

GoosePlayer::~GoosePlayer() {
	delete gooseMesh;
	delete gooseTex;
	delete gooseShader;

	mainCamera = nullptr;
}

void GoosePlayer::UpdateGameObject(float dt) {
	if (active) {
		MoveCamera();
		MoveGoose();		
	}
}

void GoosePlayer::SetCameraPostion(const Vector3 position) {
	mainCamera->SetPosition(position);
}

void GoosePlayer::MoveGoose() {	
	Matrix4 view = mainCamera->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();
	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); // View is inverse of model!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0.0f, movementSpeed, 0.0f), rightAxis);
	Vector3 relativePos = GetTransform().GetWorldPosition() - cameraPos;
	GetTransform().SetLocalOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), RadiansToDegrees(atan2(relativePos.x, relativePos.z))));

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		GetPhysicsObject()->AddForce(-rightAxis * movementSpeed);
		//selectionObject->GetPhysicsObject()->AddForce((Window::GetKeyboard()->KeyDown(KeyboardKeys::W) || Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) ? -rightAxis * 5.0f : -rightAxis * 10.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		GetPhysicsObject()->AddForce(rightAxis * movementSpeed);
		//selectionObject->GetPhysicsObject()->AddForce((Window::GetKeyboard()->KeyDown(KeyboardKeys::W) || Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) ? rightAxis * 5.0f : rightAxis * 10.0f);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		GetPhysicsObject()->AddForce(fwdAxis);
		//selectionObject->GetPhysicsObject()->AddForce((Window::GetKeyboard()->KeyDown(KeyboardKeys::A) || Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) ? fwdAxis / 2 : fwdAxis);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		GetPhysicsObject()->AddForce(-fwdAxis);
		//selectionObject->GetPhysicsObject()->AddForce((Window::GetKeyboard()->KeyDown(KeyboardKeys::A) || Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) ? -fwdAxis / 2 : -fwdAxis);
	}		
}

void GoosePlayer::MoveCamera() {
	Vector3 goosePrePos = goosePos;
	goosePos = GetTransform().GetWorldPosition();
	Vector2 mousePos = Window::GetMouse()->GetRelativePosition();
	cameraPos = (Matrix4::Rotation(-mousePos.x * 2.5f, Vector3(0, 1, 0)) * (cameraPos - goosePos) + goosePos);
	cameraPos += goosePos - goosePrePos;

	Matrix4 temp = Matrix4::BuildViewMatrix(cameraPos, goosePos, Vector3(0.0f, 1.0f, 0.0f));

	Matrix4 modelMat = temp.Inverse();

	Quaternion q(modelMat);
	Vector3 angles = q.ToEuler(); // Nearly there now!

	mainCamera->SetPosition(cameraPos);
	mainCamera->SetPitch(angles.x);
	mainCamera->SetYaw(angles.y);
}

void GoosePlayer::InitialiseGoose() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("goose.msh", &gooseMesh);

	gooseTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	gooseShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
}
