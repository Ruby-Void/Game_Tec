#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "../../Common/Camera.h"
#include <algorithm>

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld()	{
	mainCamera = new Camera();
	quadTree = nullptr;

	shuffleConstraints	= false;
	shuffleObjects		= false;
}

GameWorld::~GameWorld()	{

}

void GameWorld::Clear() {
	gameObjects.clear();
	additionStorage.clear();
	constraints.clear();
}

void GameWorld::ClearAndErase() {
	for (auto& i : gameObjects) { delete i; }
	for (auto& i : constraints) { delete i; }
	Clear();
}

void GameWorld::AddGameObject(GameObject* o) {
	gameObjects.emplace_back(o);
}

void GameWorld::RemoveGameObject(GameObject* o) {
	for (size_t i = 0; i < gameObjects.size(); ++i) {
		if (gameObjects.at(i) == o) {
			gameObjects.erase(gameObjects.begin() + i);			
			break;
		}
	}
	delete o;
}

void GameWorld::AddAdditionStorage(GameObject* o) {
	additionStorage.emplace_back(o);
}

void GameWorld::AddRemovalStorage(GameObject* o) {
	removalStorage.emplace_back(o);
}

void GameWorld::EraseStorage() {
	//for (auto& i : additionStorage) { delete i; }
	//for (auto& i : removalStorage) { delete i; }
	additionStorage.clear();
	removalStorage.clear();
}

void GameWorld::GetObjectIterators(GameObjectIterator& first, GameObjectIterator& last) const {
	first = gameObjects.begin();
	last  = gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f) {
	for (GameObject* g : gameObjects) { f(g); }
}

void GameWorld::UpdateWorld(float dt) {	
	UpdateTransforms();		
	UpdateGameObjects(dt);
	UpdateObjectList();
}

void GameWorld::UpdateGameObjects(float dt) {
	for (auto& i : gameObjects) {
		i->UpdateGameObject(dt); 
	}
}

void GameWorld::UpdateTransforms() {
	for (auto& i : gameObjects) { 
		i->GetTransform().UpdateMatrices(); 
	}
}

void GameWorld::UpdateObjectList() {
	for (auto& i : additionStorage) {
		AddGameObject(i);
	}
	for (auto& i : removalStorage) {
		RemoveGameObject(i);
	}
	EraseStorage();
}

void GameWorld::UpdateQuadTree() {
	delete quadTree;
	/*quadTree = new QuadTree<GameObject*>(Vector2(512, 512), 6);
	for (auto& i : gameObjects) { quadTree->Insert(i); }*/
}

bool GameWorld::Raycast(Ray& r, RayCollision& closestCollision, bool closestObject) const {
	// The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;

	for (auto& i : gameObjects) {
		if (!i->GetBoundingVolume() || !(i->GetLayer() & r.GetMask())) { // Objects might not be collideable
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision)) {
			if (!closestObject) {
				closestCollision = collision;
				closestCollision.node = i;
				return true;
			}
			else {
				if (thisCollision.rayDistance < collision.rayDistance) {
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}			
	}
	if (collision.node) {
		closestCollision = collision;
		closestCollision.node = collision.node;
		return true;
	}
	return false;
}

// Constraint Tutorial Stuff
void GameWorld::AddConstraint(Constraint* c) {
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint* c) {
	std::remove(constraints.begin(), constraints.end(), c); 
}

void GameWorld::GetConstraintIterators(std::vector<Constraint*>::const_iterator& first, std::vector<Constraint*>::const_iterator& last) const {
	first	= constraints.begin(); 
	last	= constraints.end();
}

string GameWorld::OutputLevelToFile() {
	const float positionShift = 90.0f;
	const size_t objectSize = 10;
	const size_t arraySize = 18;
	string outputFile = "";
	string outputLocations[arraySize][arraySize];

	outputFile.append(std::to_string(objectSize) + "\n");
	outputFile.append(std::to_string(arraySize) + "\n");
	outputFile.append(std::to_string(arraySize) + "\n");

	for (size_t z = 0; z < arraySize; ++z) {
		for (size_t x = 0; x < arraySize; ++x) {
			outputLocations[x][z] = 'E';
		}
	}

	int locationX, locationZ;

	for (auto& i : gameObjects) {
		if (i->GetName() != FLOOR_OBJECT && i->GetName() != BARRIER_OBJECT && 
			i->GetName() != GOOSE_PLAYER && i->GetName() != OBSTICAL_PLAYER) {
			locationX = (int)(i->GetTransform().GetWorldPosition().x + positionShift + 5.0f) / 10 - 1;
			locationZ = (int)(i->GetTransform().GetWorldPosition().z + positionShift + 5.0f) / 10 - 1;

			if (i->GetName() == CUBE_OBJECT) {
				outputLocations[locationX][locationZ] = CUBE_SHORT;
			}
			else if (i->GetName() == SPHERE_OBJECT) {
				outputLocations[locationX][locationZ] = SPHERE_SHORT;
			}
			else if (i->GetName() == APPLE_OBJECT) {
				outputLocations[locationX][locationZ] = APPLE_SHORT;
			}
			else if (i->GetName() == KEEPER_AI) {
				outputLocations[locationX][locationZ] = KEEPER_SHORT;
			}
		}
	}

	locationX = (int)(spawnPoint.x + positionShift + 5.0f) / 10 - 1;
	locationZ = (int)(spawnPoint.z + positionShift + 5.0f) / 10 - 1;
	outputLocations[locationX][locationZ] = SPAWN_POINT;

	locationX = (int)(roamingPoint.x + positionShift + 5.0f) / 10 - 1;
	locationZ = (int)(roamingPoint.z + positionShift + 5.0f) / 10 - 1;
	outputLocations[locationX][locationZ] = ROAM_SHORT;

	for (size_t z = 0; z < arraySize; ++z) {
		for (size_t x = 0; x < arraySize; ++x) {
			outputFile.append(outputLocations[x][z]);
		}
		outputFile.append("\n");
	}

	std::cout << outputFile << std::endl;
	return outputFile;
}