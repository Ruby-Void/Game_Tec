#pragma once
#include "../CSC8503Common/GameServer.h"
#include "../CSC8503Common/GamePacketReceiver.h"

#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/GoosePlayer.h"
#include "../CSC8503Common/ObsticalPlayer.h"
#include "../CSC8503Common/KeeperAI.h"
#include "../CSC8503Common/AppleObject.h"
#include "../CSC8503Common/ObjectNames.h"
#include "../CSC8503Common/Layers.h"

#include "../../Common/Assets.h"

#include "GameTechRenderer.h"

#include <string>
#include <fstream>

namespace NCL {
	namespace CSC8503 {
		class TutorialGame {
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			// Physics Variables
			bool selectMode = true, useGravity;
			float forceMagnitude;

			// Server
			GamePacketReceiver* serverReceiver;
			GamePacketReceiver* clientReceiver;
			GameServer* server;
			GameClient* client;

			// World Controllers
			GameTechRenderer* renderer = nullptr;
			PhysicsSystem*	  physics  = nullptr;
			GameWorld*		  world	   = nullptr;

			// Players
			GameObject*		controlledPlayer = nullptr;
			GoosePlayer*	goosePlayer		 = nullptr;
			ObsticalPlayer* obsticalPlayer	 = nullptr;

			// World Assets
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader	= nullptr;

			// World Building
			int nodeSize, gridWidth, gridHeight;
			string worldFile;

			void InitialiseAssets();
			void InitialiseNetwork();
			void InitialisePlayers(char controlled);
			void InitialiseMenu();
			void InitialiseWorld(char controlled = ' ');
			void InitialiseCamera();

			void LockedMenuCamera();
			char SelectObject();

			void AddBarriersToWorld();
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, string name, uint8_t layer);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass);

			void GenerateLevelFromFile(const std::string& filename);
		};
	}
}