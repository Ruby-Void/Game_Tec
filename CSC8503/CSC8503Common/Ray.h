#pragma once
#include "../../Common/Vector3.h"
#include "../../Common/Plane.h"
#include "Layers.h"

namespace NCL {
	namespace Maths {
		struct RayCollision {
			void*		node;			//Node that was hit
			Vector3		collidedAt;		//WORLD SPACE position of the collision!
			float		rayDistance;

			RayCollision(void*node, Vector3 collidedAt) {
				this->node			= node;
				this->collidedAt	= collidedAt;
				this->rayDistance	= 0.0f;				
			}

			RayCollision() {
				node			= nullptr;
				rayDistance		= FLT_MAX;
			}
		};

		class Ray {
		public:
			Ray(Vector3 position, Vector3 direction) {
				this->position  = position;
				this->direction = direction;
				this->mask = MASK_ACTIVE_LAYERS;
			}
			~Ray(void) {}

			Vector3 GetPosition() const { return position; }
			Vector3 GetDirection() const { return direction; }

			uint8_t GetMask() const { return mask; }
			void SetMask(uint8_t newMask) { mask = newMask; }

		protected:
			Vector3 position;	//World space position
			Vector3 direction;	//Normalised world space direction
			uint8_t mask;
		};
	}
}