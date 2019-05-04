#pragma once
#include "AssetManager.h"
#include "D11Graphics.h"
#include "Structs.h"
#define PI           3.14159265358979323846f

class Engine
{
public:
	Engine();
	~Engine();

	void Run();

private:
	void Initialize();
	void GameSetup();
	void Update();
	void Draw();
	void ShutDown();

	void CalculateProjectionMatrix();
	void CalculateProjViewMatrix();
	void CalculateProjViewWorldMatrix(GameEntity* entity);
	void CalculateProjectorProjectionMatrix(XMFLOAT4X4* dest, float pFov, float aspect);
	void CalculateProjectorViewMatrix(XMFLOAT4X4* dest, XMFLOAT3* position, XMFLOAT3* lookAt, XMFLOAT3* up);
	D11Graphics* graphics;
	AssetManager* assetManager;
	std::vector<GameEntity*> entities;
	Camera* camera;
	Projector* projector1;
	PShaderConstants pixelShaderConstants;
	SkyboxVShaderConstants skyboxVShaderConstants;

	ID3D11Buffer* pixelShaderConstantBuffer = nullptr;
	ID3D11Buffer* skyboxVShaderConstantBuffer = nullptr;

	float fov = .25f * PI;

	/* Timing info */
	double perfCounterSeconds;
	float totalTime;
	float deltaTime;
	__int64 startTime;
	__int64 currentTime;
	__int64 previousTime;

	void UpdateTimer();
};