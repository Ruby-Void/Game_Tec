#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class AABBVolume : CollisionVolume
	{
	public:
		AABBVolume(const Vector3& halfDims) : halfSizes(halfDims) {
			type = VolumeType::AABB;
		}
		~AABBVolume() {}

		Vector3 GetHalfDimensions() const { return halfSizes; }

	protected:
		Vector3 halfSizes;
	};
}
