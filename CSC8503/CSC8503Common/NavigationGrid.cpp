#include "../../Common/Assets.h"

#include "NavigationGrid.h"

#include <fstream>

using namespace NCL;
using namespace CSC8503;

const int LEFT_NODE		= 0;
const int RIGHT_NODE	= 1;
const int TOP_NODE		= 2;
const int BOTTOM_NODE	= 3;

const char WALL_NODE	= 'C';
const char FLOOR_NODE	= 'E';

NavigationGrid::NavigationGrid()	{
	nodeSize	= 0;
	gridWidth	= 0;
	gridHeight	= 0;
	allNodes	= nullptr;
}

NavigationGrid::NavigationGrid(const std::string&filename) : NavigationGrid() {
	std::ifstream infile(Assets::DATADIR + filename);

	infile >> nodeSize;
	infile >> gridWidth;
	infile >> gridHeight;

	allNodes = new GridNode[gridWidth * gridHeight];
	float dimension = nodeSize / 2;
	float offset = 90.0f;

	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];

			// Type assigned
			char type = 0;			
			infile >> type;
			n.type = type;
			
			// Node Positioning
			float positionX = ((float)x + 1.0f) * nodeSize - (offset + dimension);
			float positionY = 0.0f;
			float positionZ = ((float)y + 1.0f) * nodeSize - (offset + dimension);

			n.position = Vector3(positionX, positionY, positionZ);
		}
	}
	
	// Now to build the connectivity between the nodes
	for (int y = 0; y < gridHeight; ++y) {
		for (int x = 0; x < gridWidth; ++x) {
			GridNode& n = allNodes[(gridWidth * y) + x];

			if (y > 0) { //get the above node
				n.connected[0] = &allNodes[(gridWidth * (y - 1)) + x];
			}
			if (y < gridHeight - 1) { //get the below node
				n.connected[1] = &allNodes[(gridWidth * (y + 1)) + x];
			}
			if (x > 0) { //get left node
				n.connected[2] = &allNodes[(gridWidth * (y)) + (x - 1)];
			}
			if (x < gridWidth - 1) { //get right node
				n.connected[3] = &allNodes[(gridWidth * (y)) + (x + 1)];
			}

			for (uint8_t i = 0; i < 4; ++i) {
				if (n.connected[i]) {
					if (n.connected[i]->type == FLOOR_NODE || n.connected[i]->type == 'P' || n.connected[i]->type == 'X') {
						n.costs[i] = 1;
					}
					if (n.connected[i]->type == WALL_NODE) {
						n.connected[i] = nullptr; // Actually a wall, disconnect!
					}
				}
			}
		}	
	}
}

NavigationGrid::~NavigationGrid()	{
	delete[] allNodes;
}

bool NavigationGrid::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	// Positioning
	const float offset = 90.0f;
	const float dimension = (float)nodeSize / 2.0f;

	// need to work out which node 'from ' sits in , and 'to ' sits in
	int fromX = (int)(from.x + (offset + dimension) + 0.5f) / nodeSize - 1, toX = (int)(to.x + (offset + dimension) + 0.5f) / nodeSize - 1;
	int fromZ = (int)(from.z + (offset + dimension) + 0.5f) / nodeSize - 1, toZ = (int)(to.z + (offset + dimension) + 0.5f) / nodeSize - 1;

	if (fromX < 0 || fromX > gridWidth - 1 || fromZ < 0 || fromZ > gridHeight - 1) {
		return false; // Outside of map region!
	}
	if (toX < 0 || toX > gridWidth - 1 || toZ < 0 || toZ > gridHeight - 1) {
		return false; // Outside of map region!
	}

	GridNode* startNode = &allNodes[(fromZ * gridWidth) + fromX];
	GridNode* endNode = &allNodes[(toZ * gridWidth) + toX];

	std::vector<GridNode*> openList, closedList;

	openList.emplace_back(startNode);

	startNode->f = 0;
	startNode->g = 0;
	startNode->parent = nullptr;

	GridNode* currentBestNode = nullptr;
	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {// We've found the path!
			GridNode* node = endNode;
			outPath.PushWaypoint(to);
			while (node != nullptr) {
				outPath.PushWaypoint(node->position);
				node = node->parent; // Build up the waypoints
			}
			return true;
		}
		else {
			for (int i = 0; i < 4; ++i) {
				GridNode* neighbour = currentBestNode->connected[i];
				if (!neighbour) { // Might not be connected...
					continue;
				}

				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; // Already discarded this neighbour...
				}

				float h = Heuristic(neighbour, endNode);
				float g = currentBestNode->g + currentBestNode->costs[i];
				float f = h + g;

				bool inOpen = NodeInList(neighbour, openList);

				if (!inOpen) { // First time we've seen this neighbour
					openList.emplace_back(neighbour);
				}

				// Might be a better route to this node !
				if (!inOpen || f < neighbour->f) {
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}
	return false; // Open list emptied out with no path !
}

bool NavigationGrid::NodeInList(GridNode* n, std::vector<GridNode*>& list) const {
	std::vector <GridNode*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

GridNode* NavigationGrid::RemoveBestNode(std::vector<GridNode*>& list) const {
	std::vector <GridNode*>::iterator bestI = list.begin();
	GridNode* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode = (*i);
			bestI = i;
		}
	}

	list.erase(bestI);
	return bestNode;
}

float NavigationGrid::Heuristic(GridNode* hNode, GridNode* endNode) const {
	return (hNode->position - endNode->position).Length();
}