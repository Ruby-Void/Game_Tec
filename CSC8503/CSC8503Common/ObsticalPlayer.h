#pragma once
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "GameWorld.h"
#include "PhysicsSystem.h"
#include "GameObject.h"

#include "ObjectNames.h"
#include "Layers.h"

namespace NCL {
	namespace CSC8503 {
		class ObsticalPlayer : public GameObject {
		public:
			ObsticalPlayer(GameWorld* world, PhysicsSystem* physics);
			virtual ~ObsticalPlayer();

			void UpdateGameObject(float dt) override;
			//void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;

			bool GetActive() const { return active; }
			void SetActive(bool active) { this->active = active; }

			void toggleDeleteMode() { deleteMode = !deleteMode; }

			bool GetColliding() const { return colliding; }
			void SetColliding(bool collide) { colliding = collide; }

			GameObject* GetCollidedObject() const { return collidedObject; }
			void SetCollidedObject(GameObject* object) { collidedObject = object; }

		protected:
			bool active = false;
			GameWorld*		world;
			PhysicsSystem*	physics;
			Camera*			camera;

			bool deleteMode = false;
			
			bool colliding = false;
			GameObject* collidedObject = nullptr;

			// Meshes
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLMesh*	gooseMesh	= nullptr;
			OGLMesh*	keeperMesh	= nullptr;
			OGLMesh*	roamMesh	= nullptr;
			OGLMesh*	appleMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader	= nullptr;

			Vector3 objectPosition = Vector3(85.0f, 5.0f, -85.0f); // X = Horizontal & Z = Vertical. Range 85 to -85 

			void InitialiseAssets();
			void ManipulateObject();

			void AddObjectToWorld();
			GameObject* AddCubeToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float inverseMass = 10.0f);
			GameObject* AddAppleToWorld(const Vector3& position);
			GameObject* AddKeeperToWorld(const Vector3& position);
			GameObject* AddRoamToWorld(const Vector3& position);
			Vector3 AddSpawnToWorld(const Vector3& position);
		};
	}
}