#pragma once
#include "BoundingVolume.h"

namespace NCL {
	class BoundingOOBB : BoundingVolume
	{
	public:
		BoundingOOBB() { 
			type = BoundingType::OOBB; 
		}
		~BoundingOOBB() {}

		Vector3 origin;
		Vector3 halfSizes;
	};
}

