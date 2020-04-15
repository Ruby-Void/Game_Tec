#pragma once
#include "BoundingVolume.h"

namespace NCL {
	class BoundingSphere : BoundingVolume {
	public:
		BoundingSphere(float newRadius = 1.0f) {
			type = BoundingType::Sphere;
			this->radius = newRadius;
		}
		~BoundingSphere() {}

		float GetRadius() const { return radius; }

		Vector3 origin;
		float radius;
	};
}

