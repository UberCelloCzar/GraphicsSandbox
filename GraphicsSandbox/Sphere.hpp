#pragma once
#include "HittableObject.hpp"

class Sphere : public HittableObject
{
public:
	vec3 center;
	float radius;
	Material* material;

	Sphere() {}
	Sphere(vec3 cen, float r, Material* mat) : center(cen), radius(r), material(mat) {};
	virtual bool hit(const ray& r, float tmin, float tmax, HitRecord& rec) const;
};

bool Sphere::hit(const ray& r, float tmin, float tmax, HitRecord& rec) const
{
	rec.matPtr = material;
	vec3 oc = r.origin - center;
	float a = dot(r.direction, r.direction);
	float b = dot(oc, r.direction);
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a*c;
	if (discriminant > 0)
	{
		float temp = (-b - sqrtf(discriminant)) / a;
		if (temp < tmax && temp > tmin)
		{
			rec.t = temp;
			rec.p = r.PointAtT(rec.t);
			rec.normal = normalize((rec.p - center) / radius);
			return true;
		}

		temp = (-b + sqrtf(discriminant)) / a;
		if (temp < tmax && temp > tmin)
		{
			rec.t = temp;
			rec.p = r.PointAtT(rec.t);
			rec.normal = normalize((rec.p - center) / radius);
			return true;
		}
	}
	return false;
}