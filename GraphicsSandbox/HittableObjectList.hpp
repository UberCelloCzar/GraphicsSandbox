#pragma once
#include "HittableObject.hpp"

class HittableObjectList : public HittableObject
{
public:
	HittableObject** list;
	int size;

	HittableObjectList() {}
	HittableObjectList(HittableObject **l, int n) { list = l; size = n; }
	virtual bool hit(const ray& r, float tmin, float tmax, HitRecord& rec) const;
};

bool HittableObjectList::hit(const ray& r, float tmin, float tmax, HitRecord& rec) const
{
	HitRecord tempRec;
	bool hitSomething = false;
	float closestT = tmax;

	for (int i = 0; i < size; i++)
	{
		if (list[i]->hit(r, tmin, closestT, tempRec))
		{
			hitSomething = true;
			closestT = tempRec.t;
			rec = tempRec;
		}
	}
	return hitSomething;
}
