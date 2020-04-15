#pragma once
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "NavigationGrid.h"
#include "NavigationPath.h"

#include "GameWorld.h"
#include "GameObject.h"
#include "GoosePlayer.h"
#include "ObjectNames.h"
#include "Layers.h"

namespace NCL {
	namespace CSC8503 {
		class KeeperAI : public GameObject {
		public:
			KeeperAI(const Vector3 position, const Vector3 roamTo, const std::string& filename, GoosePlayer* goose);
			virtual ~KeeperAI();

			void UpdateGameObject(float dt) override;
			
			NavigationGrid* GetGrid() const { return grid; }
			NavigationPath* GetPath() const { return path; }
			Vector3& GetNavigationPoint() { return navigationPoint; }

			bool GetReverse() const { return reverse; }
			void SetReverse(bool invert) { reverse = invert; }

			Vector3 GetSpawnLocation() const { return spawnLocation; }
			Vector3 GetRoamLocation() const { return roamingLocation; }

			float GetTime() const { return time; }
			void SetTime(float newTime) { time = newTime; }

		protected:
			float meshSize = 4.0f, inverseMass = 0.5f, movementSpeed = 30.0f, time = 0.0f;

			// State Machine
			StateMachine* state;

			// Pathfinding 
			bool reverse = false;
			NavigationGrid* grid;
			NavigationPath* path;
			Vector3 navigationPoint;
			Vector3 spawnLocation;
			Vector3 roamingLocation;

			OGLMesh* keeperMesh = nullptr;
			OGLTexture* keeperTex = nullptr;
			OGLShader* keeperShader = nullptr;					   	

			void InitialiseParkKeeper();
			void MoveParkKeeper();
		};
	}
}