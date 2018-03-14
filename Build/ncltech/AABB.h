#pragma once
#include <nclgl\Vector3.h>
#include "BoundingBox.h"
#include "PhysicsNode.h"
#include "CommonUtils.h"

// Sphere cube Intersection, based on https://stackoverflow.com/questions/4578967/cube-sphere-intersection-test

inline float squared(float v) { return v * v; }
bool doesCubeIntersectSphere(Vector3 C1, Vector3 C2, Vector3 S, float R)
{
	float dist_squared = R * R;
	/* assume C1 and C2 are element-wise sorted, if not, do that now */
	if (S.x < C1.x)
		dist_squared -= squared(S.x - C1.x);
	else if (S.x > C2.x)
		dist_squared -= squared(S.x - C2.x);

	if (S.y < C1.y)
		dist_squared -= squared(S.y - C1.y);
	else if (S.y > C2.y)
		dist_squared -= squared(S.y - C2.y);

	// Changed z
	if (S.z > C1.z)
		dist_squared -= squared(S.z - C1.z);
	else if (S.z < C2.z)
		dist_squared -= squared(S.z - C2.z);

	return (dist_squared > 0);
}

bool SphereCubeIntersection(BoundingBox cube, PhysicsNode object) {
	Vector3 C1 = cube._min;
	Vector3 C2 = cube._max;
	Vector3 S = object.GetPosition;
	float R = object.GetSphereRadius();
	return doesCubeIntersectSphere(C1, C2, S, R);
}