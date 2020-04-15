#pragma once
#include "NavigationMap.h"
#include <string>
namespace NCL {
	namespace CSC8503 {
		struct GridNode {
			GridNode* parent;
			GridNode* connected[4];
			int costs[4];
			Vector3 position;
			float f, g;
			int type;

			GridNode() : f(0.0f), g(0.0f), type(0), parent(nullptr) {
				for (uint8_t i = 0; i < 4; ++i) {
					connected[i] = nullptr;
					costs[i] = 0;
				}
			}
			~GridNode() {}
		};

		class NavigationGrid : public NavigationMap	{
		public:
			NavigationGrid();
			NavigationGrid(const std::string&filename);
			~NavigationGrid();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
				
		protected:
			bool		NodeInList(GridNode* n, std::vector<GridNode*>& list) const;
			GridNode*	RemoveBestNode(std::vector<GridNode*>& list) const;
			float		Heuristic(GridNode* hNode, GridNode* endNode) const;

			int nodeSize, gridWidth, gridHeight;
			GridNode* allNodes;

		};
	}
}

