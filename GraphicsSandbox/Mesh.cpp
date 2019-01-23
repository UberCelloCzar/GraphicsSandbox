#include "Mesh.h"

Mesh::Mesh(ID3D11Buffer* vBuffer, ID3D11Buffer* iBuffer, uint32_t nIndices)
{
	vertexBuffer = vBuffer;
	indexBuffer = iBuffer;
	numIndices = nIndices;
}


Mesh::~Mesh()
{
	vertexBuffer->Release();
	indexBuffer->Release();
}
