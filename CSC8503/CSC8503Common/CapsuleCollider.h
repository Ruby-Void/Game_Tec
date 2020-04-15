#pragma once
#include "CollisionVolume.h"

namespace NCL {

	class CapsuleCollider : CollisionVolume {
	public:
		CapsuleCollider(float height, float radius = 1.0f) {
			type = VolumeType::Capsule;
		}
		~CapsuleCollider(void) {}

		float GetHeight() const { return height; }
		float GetRadius() const { return radius; }

	protected:
		float height, radius;
	};
}