#pragma once
#define PI 3.14159265359f
#include <iostream>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include "ray.hpp"
#include "HittableObjectList.hpp"
#include "Sphere.hpp"
#include <limits>
#include "Camera.hpp"
#include <random>
#include <time.h>
#include "Material.h"
#include "Lambert.hpp"
#include "Metal.hpp"
#include "Dielectric.hpp"
using namespace glm;

vec3 GetColor(const ray& r, HittableObject* world, int depth)
{
	HitRecord rec;
	if (world->hit(r, .001f, std::numeric_limits<float>::max(), rec))
	{
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.matPtr->Scatter(r, rec, attenuation, scattered))
		{
			return attenuation * GetColor(scattered, world, depth + 1);
		}
		else return vec3(0.f);
	}
	else
	{
		vec3 nDirection = normalize(r.direction);
		float t = .5f * (nDirection.y + 1.f);
		return (1.f - t) * vec3(1.f) + t * vec3(.5f, .7f, 1.f);
	}
}

HittableObject* TestScene()
{
	int n = 500;
	HittableObject** list = new HittableObject*[n + 1];
	list[0] = new Sphere(vec3(0.f, -1000.f, 0.f), 1000.f, new Lambert(vec3(.5f, .5f, .5f)));
	int i = 1;
	float chooseMat;
	vec3 center, centerTest;
	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			chooseMat = (rand() / (RAND_MAX + 1.0));
			center = vec3(a + .9f*(rand() / (RAND_MAX + 1.0)), .2f, b + .9f*(rand() / (RAND_MAX + 1.0)));
			centerTest = center - vec3(4.f, .2f, 0.f);
			if (sqrtf(centerTest.x*centerTest.x + centerTest.y*centerTest.y + centerTest.z*centerTest.z) > .9f)
			{
				if (chooseMat < .75f) list[i++] = new Sphere(center, .2f, new Lambert(vec3((rand() / (RAND_MAX + 1.0))*(rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))*(rand() / (RAND_MAX + 1.0)), (rand() / (RAND_MAX + 1.0))*(rand() / (RAND_MAX + 1.0)))));
				else if (chooseMat < .95f) list[i++] = new Sphere(center, .2f, new Metal(vec3(.5f * (1 + (rand() / (RAND_MAX + 1.0))), .5f*(1 + (rand() / (RAND_MAX + 1.0))), .5f * (rand() / (RAND_MAX + 1.0))), (rand() / (RAND_MAX + 1.0))));
				else list[i++] = new Sphere(center, .2f, new Dielectric(1.5f));
			}
		}
	}

	list[i++] = new Sphere(vec3(0.f, 1.f, 0.f), 1.f, new Dielectric(1.5f));
	list[i++] = new Sphere(vec3(-4.f, 1.f, 0.f), 1.f, new Lambert(vec3(.4f, .2f, .1f)));
	list[i++] = new Sphere(vec3(4.f, 1.f, 0.f), 1.f, new Metal(vec3(.7f, .6f, .5f), 0.f));

	return new HittableObjectList(list, i);
}

int main()
{
	srand((unsigned int)time(NULL));
	int nx = 1280;
	int ny = 720;
	int ns = 100;
	std::ofstream streamWriter;
	streamWriter.open("raytrace.ppm");
	if (!streamWriter.is_open()) return 1;
	streamWriter << "P3\n" << nx << " " << ny << "\n255\n";
	HittableObject* world = TestScene();
	vec3 col;
	//vec3 p;
	ray r;
	vec3 eye = vec3(13.f, 2.f, 3.f);
	vec3 lookAt = vec3(0.f, 0.f, 0.f);
	vec3 fD = eye - lookAt;
	Camera camera(eye, lookAt, vec3(0.f,1.f,0.f), 20.f, (float)(nx)/(float)(ny), .1f, 10.f);
	float u, v;
	int ir, ig, ib;
	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			col = vec3(0.f);
			for (int k = 0; k < ns; k++)
			{
				u = (float)(i + (rand() / (RAND_MAX + 1.0))) / (float)nx;
				v = (float)(j + (rand() / (RAND_MAX + 1.0))) / (float)ny;
				r = camera.GetRay(u, v);
				//p = r.PointAtT(2.f);
				col += GetColor(r, world, 0);
			}
			col /= (float)ns;
			col.x = powf(col.x, 0.45454545f);
			col.y = powf(col.y, 0.45454545f);
			col.z = powf(col.z, 0.45454545f);
			ir = (int)(255.99f*col.r);
			ig = (int)(255.99f*col.g);
			ib = (int)(255.99f*col.b);
			streamWriter << ir << " " << ig << " " << ib << "\n";
		}
	}
	streamWriter.close();
}