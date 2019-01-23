#pragma once
#include <vector>
#include "Mesh.h"

class Model
{
public:
	Model();
	Model(std::vector<Mesh*> meshList);
	~Model();

	std::vector<Mesh*> meshes;
};

