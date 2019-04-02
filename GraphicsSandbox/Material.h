#pragma once
#include "ray.hpp"
#include "HittableObject.hpp"

class Material
{
public:
	virtual bool Scatter(const ray& r_in, const HitRecord& rec, vec3& attenuation, ray& scattered) const = 0;

	vec3 RandomUnitSphereDir() const
	{
		vec3 p;
		do
		{
			p = 2.f * vec3((rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))) - vec3(1.f);
		} while ((p.x*p.x + p.y*p.y + p.z*p.z) >= 1.f);

		return p;
	};
};