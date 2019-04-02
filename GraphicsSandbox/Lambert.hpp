#pragma once
#include "Material.h"

class Lambert : public Material
{
public:
	Lambert(const vec3& a) : albedo(a) {}
	virtual bool Scatter(const ray& r_in, const HitRecord& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 target = rec.p + rec.normal + RandomUnitSphereDir();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	};

	vec3 albedo;
};