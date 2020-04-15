#pragma once
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Maths.h"

#include "GameWorld.h"
#include "GameObject.h"
#include "SphereVolume.h"
#include "Layers.h"

namespace NCL {
	namespace CSC8503 {
		class GoosePlayer : public GameObject {
		public:
			GoosePlayer(Camera* camera, const Vector3 position = Vector3(-85.0f, 7.5f, 85.0f));
			virtual ~GoosePlayer();

			void UpdateGameObject(float dt) override;
			
			void SetCameraPostion(const Vector3 position); 

			bool GetActive() const { return active; }
			void SetActive(bool active) { this->active = active; }

		protected:
			OGLMesh*	gooseMesh	= nullptr;
			OGLTexture* gooseTex	= nullptr;
			OGLShader*	gooseShader	= nullptr;
			Camera*		mainCamera	= nullptr;

			float size = 1.0f, inverseMass = 1.0f, movementSpeed = 20.0f;
			bool active = false;

			Vector3 goosePos;
			Vector3 cameraPos;
			Vector3 lockedOffset = Vector3(0.0f, -4.0f, -11.5f);

			void InitialiseGoose();			
			void MoveGoose();
			void MoveCamera();
		};
	}
}