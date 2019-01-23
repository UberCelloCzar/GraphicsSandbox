#pragma once
#include "D11Graphics.h"
#include "Model.h"
#include "Structs.h"
#include <unordered_map>
#include <d3d11.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class AssetManager
{
public:
	AssetManager();
	~AssetManager();
	void Destroy();

	void LoadVertexShader(char* name, const char* filepath, D11Graphics* graphics);
	void LoadPixelShader(char* name, const char* filepath, D11Graphics* graphics);
	void LoadWICTexture(char* name, const char* filepath, D11Graphics* graphics);
	void LoadDDSTexture(char* name, const char* filepath, D11Graphics* graphics);
	void LoadModel(char* name, const char* filepath, D11Graphics* graphics);

	Model* GetModel(std::string modelKey);
	VertexShader* GetVertexShader(std::string shaderKey);
	ID3D11PixelShader* GetPixelShader(std::string shaderKey);
	ID3D11ShaderResourceView* GetTexture(std::string textureKey);

private:
	std::unordered_map<std::string, Model*> models;
	std::unordered_map<std::string, VertexShader> vertexShaders;
	std::unordered_map<std::string, ID3D11PixelShader*> pixelShaders;
	std::unordered_map<std::string, ID3D11ShaderResourceView*> textures;

	void ProcessNode(std::vector<Mesh*>* meshes, aiNode *node, const aiScene *scene, D11Graphics* graphics);
	Mesh* ProcessMesh(aiMesh *mesh, const aiScene *scene, D11Graphics* graphics);
	void CalculateTangents(Vertex* verts, uint32_t numVerts, uint32_t* indices, uint32_t numIndices);
};

