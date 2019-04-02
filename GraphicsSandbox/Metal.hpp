#pragma once
#include "Material.h"

class Metal : public Material
{
public:
	Metal(const vec3& a, float fuzziness) : albedo(a) { if (fuzziness < 1) fuzz = fuzziness; else fuzz = 1.f; }
	
	virtual bool Scatter(const ray& r_in, const HitRecord& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 reflected = reflect(r_in.direction, rec.normal);
		scattered = ray(rec.p, reflected+fuzz*RandomUnitSphereDir());
		attenuation = albedo;
		return (dot(scattered.direction, rec.normal) > 0);
	};

	vec3 albedo;
	float fuzz;
};