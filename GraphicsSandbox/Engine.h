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
	D11Graphics* graphics;
	AssetManager* assetManager;
	std::vector<GameEntity*> entities;
	Camera* camera;
	PShaderConstants pixelShaderConstants;
	SkyboxVShaderConstants skyboxVShaderConstants;

	ID3D11Buffer* pixelShaderConstantBuffer = nullptr;
	ID3D11Buffer* skyboxVShaderConstantBuffer = nullptr;

	XMFLOAT4X4 shadowMapProj;
	XMFLOAT4X4 shadowMapView;
	XMFLOAT4X4 shadowMapProjView;
	void CalculateShadowMapProjectionMatrix();
	void CalculateShadowMapProjViewMatrix();
	void CalculateShadowMapProjViewWorldMatrix(GameEntity* entity);

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