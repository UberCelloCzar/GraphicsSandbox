#pragma once
#include <d3d11.h>
#include <cstdint>

class Mesh
{
public:
	Mesh(ID3D11Buffer* vBuffer, ID3D11Buffer* iBuffer, uint32_t nIndices);
	~Mesh();

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	uint32_t numIndices;
};

