#pragma once
#include <glm/glm.hpp>
using namespace glm;

class ray
{
public:
	vec3 origin, direction;

	ray() {}
	ray(const vec3& pOrigin, const vec3& pDirection) { origin = pOrigin; direction = normalize(pDirection); };
	vec3 PointAtT(float t) const { return origin + t * direction; };
};