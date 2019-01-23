#include "Model.h"

Model::Model()
{
}

Model::Model(std::vector<Mesh*> meshList)
{
	meshes = meshList;
}

Model::~Model()
{
	for (auto& m : meshes)
	{
		delete m;
	}
}
