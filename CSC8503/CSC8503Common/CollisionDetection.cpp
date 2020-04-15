#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "../../Common/Vector2.h"
#include "../../Common/Window.h"
#include "../../Common/Maths.h"

#include <list>

#include "../CSC8503Common/Simplex.h"

#include "Debug.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	return false;
}

bool CollisionDetection::RayIntersection(const Ray& r, GameObject& object, RayCollision& collision) {
	const Transform& transform = object.GetConstTransform();
	const CollisionVolume* volume = object.GetBoundingVolume();

	if (!volume  || !(r.GetMask() & object.GetLayer())) { return false; }

	switch (volume->type) {
		case VolumeType::AABB: return RayAABBIntersection(r, transform, (const AABBVolume&)*volume, collision);
		case VolumeType::OOBB: return RayOBBIntersection(r, transform, (const OBBVolume&)*volume, collision);
		case VolumeType::Sphere: return RaySphereIntersection(r, transform, (const SphereVolume&)*volume, collision);
	}
	return false;
}

bool CollisionDetection::RayBoxIntersection(const Ray& r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
	Vector3 boxMin = boxPos - boxSize, boxMax = boxPos + boxSize, rayPos = r.GetPosition(), rayDir = r.GetDirection(), tVals(-1, -1, -1);

	for (uint8_t i = 0; i < 3; ++i) { // Get best 3 intersections
		if (rayDir[i] > 0) { 
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i]; 
		}
		else if (rayDir[i] < 0) { 
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i]; 
		}
	}
	float bestT = tVals.GetMaxElement();
	
	if (bestT < 0.0f) { return false; } // No backwards rays!

	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f; // an amount of leeway in our calcs
	for (uint8_t i = 0; i < 3; ++i) {
		if (intersection[i] + epsilon < boxMin[i] || intersection[i] - epsilon > boxMax[i]) {
			return false; // best intersection doesn 't touch the box!
		}
	}
	collision.collidedAt = intersection;
	collision.rayDistance = bestT;
	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray& r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
	return RayBoxIntersection(r, worldTransform.GetWorldPosition(), volume.GetHalfDimensions(), collision);
}


bool CollisionDetection::RayOBBIntersection(const Ray& r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {
	Quaternion orientation = worldTransform.GetWorldOrientation();
	Vector3 position = worldTransform.GetWorldPosition();
	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());
	Vector3 localRayPos = r.GetPosition() - position;
	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());
	bool collided = RayBoxIntersection(tempRay, Vector3(), volume.GetHalfDimensions(), collision);
	if (collided) { collision.collidedAt = transform * collision.collidedAt + position; }
	return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray& r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {
	Vector3 spherePos = worldTransform.GetWorldPosition();
	float sphereRadius = volume.GetRadius();

	// Get the direction between the ray origin and the sphere origin
	Vector3 dir = (spherePos - r.GetPosition());

	// Then project the sphere 's origin onto our ray direction vector
	float sphereProj = Vector3::Dot(dir, r.GetDirection());

	// Point is behind the ray!
	if (sphereProj < 0.0f) { return false; }

	// Get closest point on ray line to sphere
	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);

	float sphereDist = (point - spherePos).Length();

	if (sphereDist > sphereRadius) { return false; }

	float offset = sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

	collision.rayDistance = sphereProj - (offset);
	collision.collidedAt = r.GetPosition() + (r.GetDirection() * collision.rayDistance);
	return true;
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) { return false; }

	collisionInfo.a = a;
	collisionInfo.b = b;

	const Transform& transformA = a->GetConstTransform();
	const Transform& transformB = b->GetConstTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}
	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA, totalSize = halfSizeA + halfSizeB;
	return (abs(delta.x) < totalSize.x && abs(delta.y) < totalSize.y && abs(delta.z) < totalSize.z) ? true : false;
}

// AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA, 
										  const AABBVolume& volumeB, const Transform& worldTransformB, 
										  CollisionInfo& collisionInfo) {
	Vector3 boxAPos = worldTransformA.GetWorldPosition(), boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBPos = worldTransformB.GetWorldPosition(), boxBSize = volumeB.GetHalfDimensions();

	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);
	if (overlap) {
		static const Vector3 faces[6] = { 
			Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f),
			Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f),
		};

		Vector3 maxA = boxAPos + boxASize, minA = boxAPos - boxASize, 
				maxB = boxBPos + boxBSize, minB = boxBPos - boxBSize;

		float distances[6] = {
			(maxB.x - minA.x),	// Distance of box 'b' to the 'left'   of box 'a'
			(maxA.x - minB.x),	// Distance of box 'b' to the 'right'  of box 'a'
			(maxB.y - minA.y),	// Distance of box 'b' to the 'bottom' of box 'a'
			(maxA.y - minB.y),	// Distance of box 'b' to the 'top'    of box 'a'
			(maxB.z - minA.z),	// Distance of box 'b' to the 'far'    of box 'a'
			(maxA.z - minB.z)	// Distance of box 'b' to the 'near'   of box 'a'
		};
		float penetration = FLT_MAX;
		Vector3 bestAxis;

		for (uint8_t i = 0; i < 6; ++i) {
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];
			}
		}

		collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
		return true;
	}
	return false;
}

// Sphere/Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA, 
											const SphereVolume& volumeB, const Transform& worldTransformB, 
											CollisionInfo& collisionInfo) {
	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetWorldPosition() - worldTransformA.GetWorldPosition();
	float deltaLength = delta.LengthSquared();

	if (deltaLength < (radii * radii)) {
		float penetration = (radii - delta.Length());
		Vector3 normal = delta.Normalised();
		Vector3 localA = normal * volumeA.GetRadius(), localB = -normal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		return true; // Colliding!
	}
	return false;
}

// AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA, 
												const SphereVolume& volumeB, const Transform& worldTransformB, 
												CollisionInfo& collisionInfo) {
	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = worldTransformB.GetWorldPosition() - worldTransformA.GetWorldPosition();
	Vector3 localPoint = delta - Maths::Clamp(delta, -boxSize, boxSize); // Delta - ClosestPointOnBox
	float distance = (localPoint).Length();

	if (distance < volumeB.GetRadius()) { // Colliding!
		Vector3 collisionNormal = localPoint.Normalised();
		Vector3 localA = Vector3(), localB = -collisionNormal * volumeB.GetRadius();
		collisionInfo.AddContactPoint(localA, localB, collisionNormal, volumeB.GetRadius() - distance);
		return true;
	}
	return false;
}

// OBB/OBB Collision
bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA, const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	//// AABB Start
	//Vector3 boxAPos = worldTransformA.GetWorldPosition(), boxASize = volumeA.GetHalfDimensions();
	//Vector3 boxBPos = worldTransformB.GetWorldPosition(), boxBSize = volumeB.GetHalfDimensions();

	//bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);

	//if (overlap) {
	//	static const Vector3 faces[6] = {
	//		Vector3(-1.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f),
	//		Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 0.0f, -1.0f), Vector3(0.0f, 0.0f, 1.0f),
	//	};

	//	Vector3 maxA = boxAPos + boxASize, minA = boxAPos - boxASize, 
	//			maxB = boxBPos + boxBSize, minB = boxBPos - boxBSize;

	//	float distances[6] = {
	//		(maxB.x - minA.x),	// Distance of box 'b' to the 'left'   of box 'a'
	//		(maxA.x - minB.x),	// Distance of box 'b' to the 'right'  of box 'a'
	//		(maxB.y - minA.y),	// Distance of box 'b' to the 'bottom' of box 'a'
	//		(maxA.y - minB.y),	// Distance of box 'b' to the 'top'    of box 'a'
	//		(maxB.z - minA.z),	// Distance of box 'b' to the 'far'    of box 'a'
	//		(maxA.z - minB.z)	// Distance of box 'b' to the 'near'   of box 'a'
	//	};

	//	float penetration = FLT_MAX;
	//	Vector3 bestAxis;

	//	for (uint8_t i = 0; i < 6; ++i) {
	//		if (distances[i] < penetration) {
	//			penetration = distances[i];
	//			bestAxis = faces[i];
	//		}
	//	}

	//	collisionInfo.AddContactPoint(Vector3(), Vector3(), bestAxis, penetration);
	//	return true;
	//}	
	//// AABB End
	return false;
}

// It's helper functions for generating rays from here on out:

Matrix4 GenerateInverseView(const Camera &c) {
	return Matrix4::Translation(c.GetPosition()) * Matrix4::Rotation(-c.GetYaw(), Vector3(0, -1, 0)) * Matrix4::Rotation(-c.GetPitch(), Vector3(-1, 0, 0));
}

Matrix4 GenerateInverseProjection(float aspect, float nearPlane, float farPlane, float fov) {
	Matrix4 m;
	float t = tan(fov * PI_OVER_360);

	m.array[0] = aspect / (1.0f / t);
	m.array[5] = t;
	m.array[10] = 0.0f;
	m.array[11] = (nearPlane - farPlane) / (2 * (farPlane * nearPlane));	// + PI_OVER_360
	m.array[14] = -1.0f;
	m.array[15] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const Camera& cam) {
	Vector2 screenSize = Window::GetWindow()->GetScreenSize();

	float aspect = screenSize.x / screenSize.y, fov = cam.GetFieldOfVision(), nearPlane = cam.GetNearPlane(), farPlane  = cam.GetFarPlane();

	// Create our inverted matrix! Note how that to get a correct inverse matrix, the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane), test1 = GenerateInverseView(cam), 
			test2 = cam.BuildViewMatrix().Inverse(), proj = cam.BuildProjectionMatrix(aspect), test4 = cam.BuildProjectionMatrix(aspect).Inverse(), 
			test3 = GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	/* Our mouse position x and y values are in 0 to screen dimensions range, so we need to turn them into the -1 to 1 axis range of clip space.
	We can do that by dividing the mouse values by the width and height of the screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	and then subtracting 1 (-1.0 to 1.0). */
	Vector4 clipSpace = Vector4((screenPos.x / (float)screenSize.x) * 2.0f - 1.0f, (screenPos.y / (float)screenSize.y) * 2.0f - 1.0f, (screenPos.z), 1.0f);

	// Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	// Our transformed w coordinate is now the 'inverse' perspective divide, so we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const Camera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition(), screenSize = Window::GetWindow()->GetScreenSize();

	// We remove the y axis mouse position from height as OpenGL is 'upside down', and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x, screenSize.y - screenMouse.y, -0.99999f);

	// We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this causes the unproject function to go a bit weird.
	Vector3 farPos = Vector3(screenMouse.x, screenSize.y - screenMouse.y, 0.99999f);

	Vector3 a = Unproject(nearPos, cam), b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	//std::cout << "Ray Direction:" << c << std::endl;

	return Ray(cam.GetPosition(), c);
}

// http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;
	float t = tan(fov * PI_OVER_360);

	m.array[0]  = aspect / (1.0f / t);
	m.array[5]  = t;
	m.array[10] = 0.0f;
	m.array[11] = 1.0f / (2.0f * (nearPlane * farPlane) / (nearPlane - farPlane));
	m.array[14] = 1.0f / -1.0f;
	m.array[15] = -((farPlane + nearPlane) / (nearPlane - farPlane)) / ((2.0f * (nearPlane * farPlane) / (nearPlane - farPlane)) * -1.0f);

	return m;
}

// And here's how we generate an inverse view matrix. It's pretty much an exact inversion of the BuildViewMatrix function of the Camera class!
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	return Matrix4::Translation(c.GetPosition()) * Matrix4::Rotation(c.GetYaw(), Vector3(0, 1, 0)) * Matrix4::Rotation(c.GetPitch(), Vector3(1, 0, 0));
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the projection matrix of our scene, and the camera used to form the view matrix.
*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const Camera &c) {
	// Create our inverted matrix! Note how that to get a correct inverse matrix, the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());
	Vector2 screenSize = Window::GetWindow()->GetScreenSize();

	/* Our mouse position x and y values are in 0 to screen dimensions range, so we need to turn them into the -1 to 1 axis range of clip space.
	We can do that by dividing the mouse values by the width and height of the screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	and then subtracting 1 (-1.0 to 1.0). */
	Vector4 clipSpace = Vector4((position.x / (float)screenSize.x) * 2.0f - 1.0f, (position.y / (float)screenSize.y) * 2.0f - 1.0f, (position.z) - 1.0f, 1.0f);

	// Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	// Our transformed w coordinate is now the 'inverse' perspective divide, so we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}