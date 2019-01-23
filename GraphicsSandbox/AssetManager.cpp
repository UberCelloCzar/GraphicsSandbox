#include "AssetManager.h"
#include <iostream>

AssetManager::AssetManager()
{
}


AssetManager::~AssetManager()
{
}

void AssetManager::Destroy()
{
	for (auto& m : models)
	{
		delete m.second;
	}
	for (auto& vs : vertexShaders)
	{
		vs.second.Release();
	}
	for (auto& ps : pixelShaders)
	{
		ps.second->Release();
	}
	for (auto& t : textures)
	{
		t.second->Release();
	}
}


void AssetManager::LoadVertexShader(char* name, const char* filepath, D11Graphics* graphics)
{
	if (vertexShaders.count(name) == 0) vertexShaders[name] = graphics->LoadVertexShader(filepath);
	else printf("a vertex shader with this key already exists: %s\n", name);
}

void AssetManager::LoadPixelShader(char* name, const char* filepath, D11Graphics* graphics)
{
	if (pixelShaders.count(name) == 0) pixelShaders[name] = graphics->LoadPixelShader(filepath);
	else printf("a pixel shader with this key already exists: %s\n", name);
}

void AssetManager::LoadWICTexture(char* name, const char* filepath, D11Graphics* graphics)
{
	if (textures.count(name) == 0) textures[name] = graphics->LoadWICTexture(filepath);
	else printf("a texture with this key already exists: %s\n", name);
}

void AssetManager::LoadDDSTexture(char* name, const char* filepath, D11Graphics* graphics)
{
	if (textures.count(name) == 0) textures[name] = graphics->LoadDDSTexture(filepath);
	else printf("a texture with this key already exists: %s\n", name);
}

void AssetManager::LoadModel(char* name, const char* filepath, D11Graphics* graphics)
{
	std::string modelKey(name);
	if (models.count(modelKey) == 0) // Model loading goes here
	{
		std::vector<Mesh*> meshes;

		Assimp::Importer importer;
		std::string fp(filepath);
		fp = "../Models/" + fp;
		{
			const aiScene *scene = importer.ReadFile(fp.c_str(), aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // If we don't have a scene, the scene is incomplete, or we have no root node
			{
				printf("ERROR::ASSIMP::%s", importer.GetErrorString());
				return;
			}

			ProcessNode(&meshes, scene->mRootNode, scene, graphics); // Starts the recursive process of loading the model
		}

		models[modelKey] = new Model(meshes);
	}
	else printf("A model already exists with this key: %s\n", modelKey.c_str());
}

Model* AssetManager::GetModel(std::string modelKey)
{
	if (models.count(modelKey) == 1) return models[modelKey];
	else
	{
		printf("No model with this key exists: %s\n", modelKey.c_str());
		return nullptr;
	}
}

VertexShader* AssetManager::GetVertexShader(std::string shaderKey)
{
	if (vertexShaders.count(shaderKey) == 1) return &vertexShaders[shaderKey];
	else
	{
		printf("No vertex shader with this key exists: %s\n", shaderKey.c_str());
		return nullptr;
	}
}

ID3D11PixelShader* AssetManager::GetPixelShader(std::string shaderKey)
{
	if (pixelShaders.count(shaderKey) == 1) return pixelShaders[shaderKey];
	else
	{
		printf("No pixel shader with this key exists: %s\n", shaderKey.c_str());
		return nullptr;
	}
}

ID3D11ShaderResourceView* AssetManager::GetTexture(std::string textureKey)
{
	if (textures.count(textureKey) == 1) return textures[textureKey];
	else
	{
		printf("No texture with this key exists: %s\n", textureKey.c_str());
		return nullptr;
	}
}

void AssetManager::CalculateTangents(Vertex* verts, uint32_t numVerts, uint32_t* indices, uint32_t numIndices) // Code adapted from: http://www.terathon.com/code/tangent.html
{
	// Calculate tangents one whole triangle at a time
	for (uint32_t i = 0; i < numVerts;)
	{
		// Grab indices and vertices of first triangle
		uint32_t i1 = indices[i++];
		uint32_t i2 = indices[i++];
		uint32_t i3 = indices[i++];
		Vertex* v1 = &verts[i1];
		Vertex* v2 = &verts[i2];
		Vertex* v3 = &verts[i3];
		v1->tangent = XMFLOAT3(0, 0, 0); // Reset tangents
		v2->tangent = XMFLOAT3(0, 0, 0);
		v3->tangent = XMFLOAT3(0, 0, 0);

		// Calculate vectors relative to triangle positions
		float x1 = v2->position.x - v1->position.x;
		float y1 = v2->position.y - v1->position.y;
		float z1 = v2->position.z - v1->position.z;

		float x2 = v3->position.x - v1->position.x;
		float y2 = v3->position.y - v1->position.y;
		float z2 = v3->position.z - v1->position.z;

		// Do the same for vectors relative to triangle uv's
		float s1 = v2->uv.x - v1->uv.x;
		float t1 = v2->uv.y - v1->uv.y;

		float s2 = v3->uv.x - v1->uv.x;
		float t2 = v3->uv.y - v1->uv.y;

		// Create vectors for tangent calculation
		float r = 1.0f / (s1 * t2 - s2 * t1);

		float tx = (t2 * x1 - t1 * x2) * r;
		float ty = (t2 * y1 - t1 * y2) * r;
		float tz = (t2 * z1 - t1 * z2) * r;

		// Adjust tangents of each vert of the triangle
		v1->tangent.x += tx;
		v1->tangent.y += ty;
		v1->tangent.z += tz;

		v2->tangent.x += tx;
		v2->tangent.y += ty;
		v2->tangent.z += tz;

		v3->tangent.x += tx;
		v3->tangent.y += ty;
		v3->tangent.z += tz;
	}

	// Ensure all of the tangents are orthogonal to the normals
	for (uint32_t i = 0; i < numVerts; i++)
	{
		XMVECTOR normal = XMLoadFloat3(&verts[i].normal);
		XMVECTOR tangent = XMLoadFloat3(&verts[i].tangent);

		tangent = XMVector3Normalize(tangent - normal * XMVector3Dot(normal, tangent)); // Gram-Schmidt orthogonalization

		XMStoreFloat3(&verts[i].tangent, tangent);
	}
}

void AssetManager::ProcessNode(std::vector<Mesh*>* meshes, aiNode *node, const aiScene *scene, D11Graphics* graphics)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) // Bring in the node's meshes
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		meshes->push_back(ProcessMesh(mesh, scene, graphics));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++) // Recursively process child nodes until done
	{
		ProcessNode(meshes, node->mChildren[i], scene, graphics);
	}
}

Mesh* AssetManager::ProcessMesh(aiMesh *mesh, const aiScene *scene, D11Graphics* graphics)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	//std::cout << "NumVerts: " << mesh->mNumVertices << std::endl;

	for (size_t i = 0; i < mesh->mNumVertices; i++) // Process vertex data
	{
		Vertex vertex;
		vertex.position = XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//std::cout << "Vertex: " << vertex.Position.x << ", " << vertex.Position.y << ", " << vertex.Position.z << std::endl;
		vertex.normal = XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//vertex.tangent = XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		if (mesh->mTextureCoords[0]) // Meshes can straight up not have texture coords
		{
			vertex.uv = XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else vertex.uv = XMFLOAT2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}

	for (size_t i = 0; i < mesh->mNumFaces; i++) // Grab the indices
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
			//std::cout << "Index: " << face.mIndices[j] << std::endl;
		}
	}
	//for (int i = 0; i < vertices.size(); i++)
	//{
	//	std::cout << "Vertex " << i << std::endl;
	//	std::cout << "Pos: " << vertices[i].position.x << ", " << vertices[i].position.y << ", " << vertices[i].position.z << std::endl;
	//	std::cout << "Nor: " << vertices[i].normal.x << ", " << vertices[i].normal.y << ", " << vertices[i].normal.z << std::endl;
	//	std::cout << "UV : " << vertices[i].uv.x << ", " << vertices[i].uv.y << std::endl;
	//}
	//for (int i = 0; i < indices.size(); i++)
	//{
	//	std::cout << "Index: " << indices[i] << std::endl;
	//}
	//std::cout << "Vert size: " << vertices.size() << std::endl;
	//std::cout << "Ind size: " << vertices.size() << std::endl;
	CalculateTangents(&vertices[0], vertices.size(), &indices[0], indices.size());

	//std::unordered_map<Vertex, uint32_t> uniqueVertices = {}; // Do a pass to cut duplicated vertices
	//std::vector<Vertex> uniquenessVerts;
	//std::vector<uint32_t> uniquenessInds;
	//for (uint32_t i = 0; i < vertices.size(); ++i)
	//{
	//	if (uniqueVertices.count(vertices[i]) == 0)
	//	{
	//		uniqueVertices[vertices[i]] = static_cast<uint32_t>(uniquenessVerts.size());
	//		uniquenessVerts.push_back(vertices[i]);
	//	}
	//	uniquenessInds.push_back(uniqueVertices[vertices[i]]);
	//}

	ID3D11Buffer* vertexBuffer = graphics->CreateVertexBuffer(&vertices[0], vertices.size());
	ID3D11Buffer* indexBuffer = graphics->CreateIndexBuffer(&indices[0], indices.size());
	return new Mesh(vertexBuffer, indexBuffer, (uint32_t)indices.size());
}
