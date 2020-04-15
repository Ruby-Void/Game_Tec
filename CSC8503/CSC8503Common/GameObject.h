#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"

#include <vector>

#include "Layers.h"

using std::vector;

namespace NCL {
	namespace CSC8503 {
		class NetworkObject;

		class GameObject {
		public:
			GameObject(string name = "", uint8_t layerNumber = LAYER_ZERO);
			~GameObject();

			virtual void UpdateGameObject(float dt);

			void SetBoundingVolume(CollisionVolume* vol) { boundingVolume = vol; }

			const CollisionVolume* GetBoundingVolume() const { return boundingVolume; }

			bool IsActive() const { return isActive; }

			const Transform& GetConstTransform() const { return transform; }

			Transform& GetTransform() { return transform; }

			RenderObject* GetRenderObject() const { return renderObject; }

			PhysicsObject* GetPhysicsObject() const { return physicsObject; }

			NetworkObject* GetNetworkObject() const { return networkObject; }

			void SetRenderObject(RenderObject* newObject) { renderObject = newObject; }

			void SetPhysicsObject(PhysicsObject* newObject) { physicsObject = newObject; }

			uint8_t GetLayer() const { return layer; }

			const string& GetName() const { return name; }

			virtual void OnCollisionBegin(GameObject* otherObject) {}

			virtual void OnCollisionEnd(GameObject* otherObject) {}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			NetworkObject*		networkObject;

			bool isActive;
			uint8_t layer;
			string name;

			Vector3 broadphaseAABB;
		};
	}
}

