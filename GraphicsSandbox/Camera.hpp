#pragma once

#include "ray.hpp"

vec3 RandomInUnitDisk()
{
	vec3 p;
	do
	{
		p = 2.f * vec3((rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0)), 0.f) - vec3(1.f, 1.f, 0.f);
	} while (dot(p, p) >= 1.f);
	return p;
}

class Camera
{
public:
	Camera(vec3 eye, vec3 lookAt, vec3 up, float vfov, float aspect, float aperture, float focusDist)
	{
		lensRadius = aperture / 2.f;
		float theta = vfov * PI / 180.f;
		float halfHeight = tanf(theta / 2.f);
		float halfWidth = aspect * halfHeight;

		origin = eye;
		w = normalize(eye - lookAt);
		u = normalize(cross(up, w));
		v = cross(w, u);
		lowerLeft = origin - halfWidth * focusDist * u - halfHeight * focusDist * v - focusDist * w;
		horizontal = 2.f * halfWidth * focusDist * u;
		vertical = 2.f * halfHeight * focusDist * v;
	}

	ray GetRay(float s, float t) 
	{ 
		vec3 rd = lensRadius * RandomInUnitDisk();
		vec3 offset = u * rd.x + v * rd.y;
		return ray(origin + offset, lowerLeft + s*horizontal + t*vertical - origin - offset); 
	}

	vec3 origin;
	vec3 lowerLeft;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	float lensRadius;
};