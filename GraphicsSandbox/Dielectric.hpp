#pragma once
#include "Material.h"
#include <random>


bool Refract(const vec3& i, const vec3& n, float eta, vec3& refracted)
{
	vec3 unitI = normalize(i);
	float dt = dot(unitI, n);
	float discriminant = 1.f - eta * eta * (1 - dt * dt);
	if (discriminant > 0.f)
	{
		refracted = eta * (unitI - n * dt) - n * sqrtf(discriminant);
		return true;
	}
	else return false;
}

float Schlick(float cosine, float refIndex)
{
	float r0 = (1 - refIndex) / (1 + refIndex);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

class Dielectric : public Material
{
public:
	Dielectric(float refractiveIndex) : refIndex(refractiveIndex) {}

	virtual bool Scatter(const ray& r_in, const HitRecord& rec, vec3& attenuation, ray& scattered) const
	{
		vec3 outerNorm, refracted;
		float ni_over_nt, reflectProb, cosine;
		vec3 reflected = reflect(r_in.direction, rec.normal);
		attenuation = vec3(1.f);

		if (dot(r_in.direction, rec.normal) > 0)
		{
			outerNorm = -rec.normal;
			ni_over_nt = refIndex;
			cosine = refIndex * dot(r_in.direction, rec.normal); // No need to divide by length, as I know it's 1 since I normalize the ray directions
		}
		else
		{
			outerNorm = rec.normal;
			ni_over_nt = 1.f / refIndex;
			cosine = -dot(r_in.direction, rec.normal);
		}

		if (Refract(r_in.direction, outerNorm, ni_over_nt, refracted)) reflectProb = Schlick(cosine, refIndex); // GLM's refract doesn't quite do what I need it to for the scales we're using
		else reflectProb = 1.f;

		if ((rand() / (RAND_MAX + 1.0)) < reflectProb) scattered = ray(rec.p, reflected);
		else scattered = ray(rec.p, refracted);

		return true;
	};

	float refIndex;
};