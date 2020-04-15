#pragma once
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"
#include "GameObject.h"
#include "ObjectNames.h"
#include "Layers.h"

#include <string>
#include <vector>

namespace NCL {
		class Camera;
		using Maths::Ray;

	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o);

			void AddAdditionStorage(GameObject* o);
			void AddRemovalStorage(GameObject* o);
			void EraseStorage();

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c);			

			Camera* GetMainCamera() const { return mainCamera; }

			void ShuffleConstraints(bool state) { shuffleConstraints = state; }
			void ShuffleObjects(bool state) { shuffleObjects = state; }

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(GameObjectIterator& first, GameObjectIterator& last) const;

			void GetConstraintIterators(std::vector<Constraint*>::const_iterator& first, std::vector<Constraint*>::const_iterator& last) const;

			Vector3 GetSpawnPoint() const { return spawnPoint; }
			void SetSpawnPoint(Vector3 spawn) { spawnPoint = spawn; }

			Vector3 GetRoamingPoint() const { return roamingPoint; }
			void SetRoamingPoint(Vector3 point) { roamingPoint = point; }

			string OutputLevelToFile();

		protected:
			bool shuffleConstraints, shuffleObjects;
			std::vector<GameObject*> gameObjects;
			std::vector<GameObject*> additionStorage;
			std::vector<GameObject*> removalStorage;
			std::vector<Constraint*> constraints;
			QuadTree<GameObject*>* quadTree;
			Camera* mainCamera;

			Vector3 spawnPoint;	
			Vector3 roamingPoint;

			void UpdateGameObjects(float dt);
			void UpdateTransforms();
			void UpdateObjectList();
			void UpdateQuadTree();			
		};
	}
}