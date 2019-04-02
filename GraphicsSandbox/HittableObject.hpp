#pragma once
#include "ray.hpp"

class Material;

struct HitRecord
{
	float t;
	vec3 p, normal;
	Material* matPtr;
};

class HittableObject
{
public:
	virtual bool hit(const ray& r, float tmin, float tmax, HitRecord& rec) const = 0;
};