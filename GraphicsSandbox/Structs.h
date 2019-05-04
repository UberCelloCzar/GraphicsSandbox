// Code written by Trevor Walden and Robert Bailey
#pragma once
#include <cstdint>
#include <DirectXMath.h>
#include <string>
#include <algorithm>
using namespace DirectX;

struct Camera
{
	XMFLOAT4X4 projection;
	XMFLOAT4X4 view;
	XMFLOAT4X4 projView;
	XMFLOAT4 rotationQuaternion;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT3 up;
	XMFLOAT2 rotationRads;

	void Move(float forward, float strafe, float vert) // Move relative to the direction faced
	{
		XMVECTOR total = XMVectorScale(XMLoadFloat3(&direction), forward); // Add in the direction faced
		total = XMVectorAdd(total, XMVectorScale(XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&direction)), strafe)); // Calculate the right vector then scale it and add it
		XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&position), total));
		position.y += vert;
	}

	void Rotate(float x, float y)
	{
		rotationRads.x += x;
		rotationRads.y += y;
		//printf("%f, %f\n", rotationRads.x, rotationRads.y);

		XMVECTOR quat = XMQuaternionRotationRollPitchYaw(rotationRads.y, rotationRads.x, 0.f); // Update the quat
		XMStoreFloat3(&direction, XMVector3Rotate(XMVectorSet(0.f, 0.f, 1.f, 0.f), quat)); // Update direction
		XMStoreFloat4(&rotationQuaternion, quat);
	}
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT2 uv;

	bool operator==(const Vertex& other) const
	{
		int8_t equal = 1;
		equal = equal & static_cast<int8_t>(XMVector3Equal(XMLoadFloat3(&position), XMLoadFloat3(&other.position)));
		equal = equal & static_cast<int8_t>(XMVector3Equal(XMLoadFloat3(&normal), XMLoadFloat3(&other.normal)));
		equal = equal & static_cast<int8_t>(XMVector2Equal(XMLoadFloat2(&uv), XMLoadFloat2(&other.uv)));
		return static_cast<bool>(equal); // Tangents are calculated based on the other values, so they're equal if all else is
	}
};

struct VShaderConstants
{
	XMFLOAT4X4 projViewWorld;
	XMFLOAT4X4 world;
	XMFLOAT4X4 projectorView1;
	XMFLOAT4X4 projectorProj1;
};

struct PShaderConstants
{
	XMFLOAT3A cameraPosition;
	XMFLOAT3A lightPos1;
	XMFLOAT3A lightPos2;
	XMFLOAT3A lightPos3;
	XMFLOAT3A lightColor1;
	XMFLOAT3A lightColor2;
	XMFLOAT3A lightColor3;
};


struct SkyboxVShaderConstants
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
};

struct GameEntity
{
	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT4 rotationQuaternion;
	VShaderConstants vertexShaderConstants;
	std::string albedoKey;
	std::string normalKey;
	std::string metallicKey;
	std::string roughnessKey;
	std::string aoKey;
	std::string modelKey;
	ID3D11Buffer* vertexShaderConstantBuffer;
};

struct VertexShader
{
	ID3D11VertexShader* shaderPointer;
	ID3D11InputLayout* inputLayout;

	void Release()
	{
		shaderPointer->Release();
		inputLayout->Release();
	}
};

struct Projector
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	XMFLOAT3 up;
	std::string albedoKey;
};

//namespace glm {
//	namespace detail
//	{
//		GLM_INLINE void hash_combine(size_t &seed, size_t hash)
//		{
//			hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
//			seed ^= hash;
//		}
//	}
//}

//namespace std // Referenced from prior work done alongside the vulkan tutorial from vulkan-tutorial.com
//{
//	template<typename T, glm::qualifier Q> // Copied from gtx hash, because using that full set requires stl dependency
//	struct hash<glm::vec<2, T, Q> >
//	{
//		GLM_FUNC_DECL size_t operator()(glm::vec<2, T, Q> const& v) const;
//	};
//
//	template<typename T, glm::qualifier Q>
//	struct hash<glm::vec<3, T, Q> >
//	{
//		GLM_FUNC_DECL size_t operator()(glm::vec<3, T, Q> const& v) const;
//	};
//
//	template<typename T, glm::qualifier Q>
//	GLM_FUNC_QUALIFIER size_t hash<glm::vec<2, T, Q>>::operator()(glm::vec<2, T, Q> const& v) const
//	{
//		size_t seed = 0;
//		hash<T> hasher;
//		glm::detail::hash_combine(seed, hasher(v.x));
//		glm::detail::hash_combine(seed, hasher(v.y));
//		return seed;
//	}
//
//	template<typename T, glm::qualifier Q>
//	GLM_FUNC_QUALIFIER size_t hash<glm::vec<3, T, Q>>::operator()(glm::vec<3, T, Q> const& v) const
//	{
//		size_t seed = 0;
//		hash<T> hasher;
//		glm::detail::hash_combine(seed, hasher(v.x));
//		glm::detail::hash_combine(seed, hasher(v.y));
//		glm::detail::hash_combine(seed, hasher(v.z));
//		return seed;
//	}
//
//	template<> struct hash<Vertex> // Tangents are calculated based on the other values, so we can exclude it from the hash
//	{
//		size_t operator()(Vertex const& vertex) const
//		{
//			return (((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1));
//		}
//	};
//}
