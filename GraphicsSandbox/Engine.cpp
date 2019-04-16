#include "Engine.h"
#include "D11Graphics.h"


Engine::Engine()
{
}


Engine::~Engine()
{
}


void Engine::Initialize()
{
	assetManager = new AssetManager();

	graphics = new D11Graphics(); // Create whichever graphics system we're using and initialize it
	graphics->Initialize();

	GameSetup();

	__int64 perfFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&perfFreq);
	perfCounterSeconds = 1.0 / (double)perfFreq;

	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	startTime = now;
	currentTime = now;
	previousTime = now;
}

void Engine::GameSetup()
{
	/* Load Shaders */
	assetManager->LoadVertexShader((char*)"BaseVS", "BaseVertexShader", graphics);
	assetManager->LoadPixelShader((char*)"BasePS", "BasePixelShader", graphics);
	assetManager->LoadVertexShader((char*)"SkyboxVS", "SkyboxVertexShader", graphics);
	assetManager->LoadPixelShader((char*)"SkyboxPS", "SkyboxPixelShader", graphics);
	assetManager->LoadVertexShader((char*)"ShadowVS", "ShadowVertexShader", graphics);
	assetManager->LoadPixelShader((char*)"ShadowPS", "ShadowPixelShader", graphics);

	/* Initialize Camera */
	camera = new Camera();
	camera->position = XMFLOAT3(0.f, 0.f, -10.f);
	camera->direction = XMFLOAT3(0.f, 0.f, 1.f);
	camera->up = XMFLOAT3(0.f, 1.f, 0.f);
	camera->rotationRads = XMFLOAT2(0.f, 0.f);
	//camera->rotationMatrix = glm::rotate(glm::rotate(glm::mat4(1.f), -.1f, glm::vec3(1, 0, 0)), .1f, glm::vec3(0,1,0)); // Identity matrix
	XMStoreFloat4(&camera->rotationQuaternion, XMQuaternionIdentity());
	CalculateProjectionMatrix();
	CalculateProjViewMatrix();

	/* Load Models */
	assetManager->LoadModel((char*)"Cube", "cube.obj", graphics);
	assetManager->LoadModel((char*)"Cone", "cone.obj", graphics);
	assetManager->LoadModel((char*)"Sphere", "sphere.obj", graphics);
	assetManager->LoadModel((char*)"Cerberus", "Cerberus.fbx", graphics);

	/* Load Textures */
	{
		assetManager->LoadDDSTexture((char*)"SM_EnvMap", "SnowMachineEnv", graphics);
		assetManager->LoadDDSTexture((char*)"SM_IrrMap", "SnowMachineIrr", graphics);
		assetManager->LoadDDSTexture((char*)"SM_SpecMap", "SnowMachineSpec", graphics);

		assetManager->LoadDDSTexture((char*)"BRDF_LUT", "SnowMachineBrdf", graphics);

		assetManager->LoadWICTexture((char*)"M_100Metal", "solidgoldmetal.png", graphics);
		assetManager->LoadWICTexture((char*)"M_0Metal", "nometallic.png", graphics);

		assetManager->LoadWICTexture((char*)"A_Gold", "solidgoldbase.png", graphics);
		assetManager->LoadWICTexture((char*)"N_Plain", "solidgoldnormal.png", graphics);
		assetManager->LoadWICTexture((char*)"R_Gold", "solidgoldroughness.png", graphics);

		assetManager->LoadWICTexture((char*)"A_Snow", "Snow_001_COLOR.png", graphics);
		assetManager->LoadWICTexture((char*)"N_Snow", "Snow_001_NORM.png", graphics);
		assetManager->LoadWICTexture((char*)"R_Snow", "Snow_001_ROUGH.png", graphics);
		assetManager->LoadWICTexture((char*)"AO_Snow", "Snow_001_OCC.png", graphics);

		assetManager->LoadWICTexture((char*)"A_Rock", "holey-rock1-albedo.png", graphics);
		assetManager->LoadWICTexture((char*)"AO_Rock", "holey-rock1-ao.png", graphics);
		assetManager->LoadWICTexture((char*)"N_Rock", "holey-rock1-normal-ue.png", graphics);
		assetManager->LoadWICTexture((char*)"R_Rock", "holey-rock1-roughness.png", graphics);

		assetManager->LoadWICTexture((char*)"A_Cerberus", "Cerberus_A.jpg", graphics);
		assetManager->LoadWICTexture((char*)"N_Cerberus", "Cerberus_N.jpg", graphics);
		assetManager->LoadWICTexture((char*)"M_Cerberus", "Cerberus_M.jpg", graphics);
		assetManager->LoadWICTexture((char*)"R_Cerberus", "Cerberus_R.jpg", graphics);
		assetManager->LoadWICTexture((char*)"AO_Cerberus", "Cerberus_AO.jpg", graphics);
	}

	/* Create Game Objects */
	GameEntity* cube = new GameEntity(); // Entity 0 should always be a unit cube
	cube->position = XMFLOAT3(0.f, 0.f, 0.f);
	cube->scale = XMFLOAT3(1.f, 1.f, 1.f);
	XMStoreFloat4(&cube->rotationQuaternion, XMQuaternionIdentity());
	cube->modelKey = "Cube";
	cube->albedoKey = "A_Gold";
	cube->normalKey = "N_Plain";
	cube->metallicKey = "M_100Metal";
	cube->roughnessKey = "R_Gold";
	cube->aoKey = "M_100Metal";
	cube->vertexShaderConstants = {};


	GameEntity* snowball = new GameEntity();
	snowball->position = XMFLOAT3(0.f, 2.f, 5.0f);
	snowball->scale = XMFLOAT3(1.f, 1.f, 1.f);
	XMStoreFloat4(&snowball->rotationQuaternion, XMQuaternionIdentity());
	snowball->modelKey = "Sphere";
	snowball->albedoKey = "A_Snow";
	snowball->normalKey = "N_Snow";
	snowball->metallicKey = "M_0Metal";
	snowball->roughnessKey = "R_Snow";
	snowball->aoKey = "AO_Snow";
	snowball->vertexShaderConstants = {};

	GameEntity* floor = new GameEntity();
	floor->scale = XMFLOAT3(30.f, .1f, 30.f);
	floor->position = XMFLOAT3(0.f, -.5f, 0.0f);
	XMStoreFloat4(&floor->rotationQuaternion, XMQuaternionRotationRollPitchYaw(-XM_PI, 0, 0));
	floor->modelKey = "Cube";
	floor->albedoKey = "A_Gold";
	floor->normalKey = "N_Plain";
	floor->metallicKey = "M_0Metal";
	floor->roughnessKey = "M_100Metal";
	floor->aoKey = "M_100Metal";
	floor->vertexShaderConstants = {};

	GameEntity* block = new GameEntity();
	block->scale = XMFLOAT3(1.f, 1.f, 1.f);
	block->position = XMFLOAT3(-1.f, 2.f, -1.0f);
	XMStoreFloat4(&block->rotationQuaternion, XMQuaternionIdentity());
	block->modelKey = "Cube";
	block->albedoKey = "A_Gold";
	block->normalKey = "N_Plain";
	block->metallicKey = "M_100Metal";
	block->roughnessKey = "R_Gold";
	block->aoKey = "M_100Metal";
	block->vertexShaderConstants = {};

	GameEntity* cerberus = new GameEntity();
	cerberus->scale = XMFLOAT3(.1f, .1f, .1f);
	cerberus->position = XMFLOAT3(-5.f, 6.f, 5.0f);
	XMStoreFloat4(&cerberus->rotationQuaternion, XMQuaternionRotationRollPitchYaw(-XM_PIDIV2, 0, 0));
	cerberus->modelKey = "Cerberus";
	cerberus->albedoKey = "A_Cerberus";
	cerberus->normalKey = "N_Cerberus";
	cerberus->metallicKey = "M_Cerberus";
	cerberus->roughnessKey = "R_Cerberus";
	cerberus->aoKey = "AO_Cerberus";
	cerberus->vertexShaderConstants = {};

	entities.push_back(cube);
	entities.push_back(snowball);
	entities.push_back(floor);
	entities.push_back(block);
	entities.push_back(cerberus);

	// Create my shadow map matrices
	CalculateShadowMapProjectionMatrix();
	CalculateShadowMapProjViewMatrix();

	for (auto& e : entities)
	{
		CalculateProjViewWorldMatrix(e);
		e->vertexShaderConstantBuffer = graphics->CreateConstantBuffer(&(e->vertexShaderConstants), sizeof(VShaderConstants));
	}

	/* Create Constant Buffers */
	pixelShaderConstants.cameraPosition.x = camera->position.x;
	pixelShaderConstants.cameraPosition.y = camera->position.y;
	pixelShaderConstants.cameraPosition.z = camera->position.z;
	pixelShaderConstants.lightPos1 = XMFLOAT3A(0.f, 15.f, 0.f);
	pixelShaderConstants.lightPos2 = XMFLOAT3A(2.f, 4.f, 2.f);
	pixelShaderConstants.lightPos3 = XMFLOAT3A(0.f, 10.f, 7.f);
	pixelShaderConstants.lightColor1 = XMFLOAT3A(.95f, 0.f, 0.f);
	pixelShaderConstants.lightColor2 = XMFLOAT3A(0.f, 0.1f, 0.f);
	pixelShaderConstants.lightColor3 = XMFLOAT3A(0.f, 0.f, 0.1f);
	skyboxVShaderConstants.projection = camera->projection;
	skyboxVShaderConstants.view = camera->view;

	skyboxVShaderConstants.projection = camera->projection;
	skyboxVShaderConstants.view = camera->view;
	pixelShaderConstantBuffer = graphics->CreateConstantBuffer(&pixelShaderConstants, sizeof(PShaderConstants));
	skyboxVShaderConstantBuffer = graphics->CreateConstantBuffer(&skyboxVShaderConstants, sizeof(SkyboxVShaderConstants));

}

void Engine::Run()
{
	Initialize();
	bool quit = false;
	while (!quit)
	{
		UpdateTimer();
		graphics->UpdateStats(totalTime);
		Update();
		Draw();
		quit = graphics->HandleLowLevelEvents(GetAsyncKeyState(VK_ESCAPE));
	}
	ShutDown();
}

void Engine::UpdateTimer()
{
	__int64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now); // Gat current time
	currentTime = now;

	deltaTime = std::max((float)((currentTime - previousTime) * perfCounterSeconds), 0.0f); // Calc delta time, ensure it's not less than zero

	totalTime = (float)((currentTime - startTime) * perfCounterSeconds); // Calculate the total time from start to now

	previousTime = currentTime; // Save current time for next frame
}

void Engine::Update()
{
	//printf("%f, %f\n", graphics->rotateBy.x, graphics->rotateBy.y);
	camera->Rotate(graphics->rotateBy.x, graphics->rotateBy.y);
	graphics->rotateBy = XMFLOAT2(0, 0);
	float right = 0;
	float forward = 0;
	float vert = 0;
	if (GetAsyncKeyState('A') & 0x8000) right -= 5 * deltaTime;
	if (GetAsyncKeyState('D') & 0x8000) right += 5 * deltaTime;
	if (GetAsyncKeyState('W') & 0x8000) forward += 5 * deltaTime;
	if (GetAsyncKeyState('S') & 0x8000) forward -= 5 * deltaTime;
	if (GetAsyncKeyState('Q') & 0x8000) vert -= 5 * deltaTime;
	if (GetAsyncKeyState('E') & 0x8000) vert += 5 * deltaTime;
	if (forward != 0 || right != 0 || vert != 0) camera->Move(forward, right, vert);

	right = 0;
	forward = 0;
	vert = 0;
	if (GetAsyncKeyState('H') & 0x8000) right -= 5 * deltaTime;
	if (GetAsyncKeyState('K') & 0x8000) right += 5 * deltaTime;
	if (GetAsyncKeyState('U') & 0x8000) forward += 5 * deltaTime;
	if (GetAsyncKeyState('J') & 0x8000) forward -= 5 * deltaTime;
	if (GetAsyncKeyState('Y') & 0x8000) vert -= 5 * deltaTime;
	if (GetAsyncKeyState('I') & 0x8000) vert += 5 * deltaTime;

	XMVECTOR total = XMVectorScale(XMVectorSet(1.f,-.5f,1.f,0.f), forward); // Add in the direction faced
	total = XMVectorAdd(total, XMVectorScale(XMVector3Cross(XMVectorSet(.5f,2.f,.5f,0.f), XMVectorSet(1.f, -.5f, 1.f, 0.f)), right)); // Calculate the right vector then scale it and add it
	XMStoreFloat3(&pixelShaderConstants.lightPos1, XMVectorAdd(XMLoadFloat3(&pixelShaderConstants.lightPos1), total));
	pixelShaderConstants.lightPos1.y += vert;
}

void Engine::Draw()
{
	CalculateProjViewMatrix();
	CalculateShadowMapProjectionMatrix();
	CalculateShadowMapProjViewMatrix();
	graphics->SetVertexShader(assetManager->GetVertexShader(std::string("ShadowVS")));
	graphics->SetPixelShader(assetManager->GetPixelShader(std::string("ShadowPS")));
	graphics->BeginShadowPrepass();

	for (size_t i = 1; i < entities.size(); ++i)
	{
		CalculateProjViewWorldMatrix(entities[i]); // Update the main matrix in the vertex shader constants area of the entity
		CalculateShadowMapProjViewWorldMatrix(entities[i]);

		graphics->SetConstantBufferVS(entities[i]->vertexShaderConstantBuffer, &(entities[i]->vertexShaderConstants), sizeof(VShaderConstants));

		Model* model = assetManager->GetModel(entities[i]->modelKey);

		for (size_t j = 0; j < model->meshes.size(); ++j)
		{
			graphics->DrawMesh(model->meshes[j]); // If we have a bunch of the same object and we have time, I can add a render for instancing
		}
	}

	graphics->SetVertexShader(assetManager->GetVertexShader(std::string("BaseVS")));
	graphics->SetPixelShader(assetManager->GetPixelShader(std::string("BasePS")));
	pixelShaderConstants.cameraPosition.x = camera->position.x;
	pixelShaderConstants.cameraPosition.y = camera->position.y;
	pixelShaderConstants.cameraPosition.z = camera->position.z;
	graphics->RunShadowAA();
	graphics->BeginNewFrame();
	for (size_t i = 1; i < entities.size(); ++i)
	{
		graphics->SetConstantBufferVS(entities[i]->vertexShaderConstantBuffer, &(entities[i]->vertexShaderConstants), sizeof(VShaderConstants));
		graphics->SetConstantBufferPS(pixelShaderConstantBuffer, &pixelShaderConstants, sizeof(PShaderConstants));
		graphics->SetTexture(assetManager->GetTexture(entities[i]->albedoKey), 0);
		graphics->SetTexture(assetManager->GetTexture(entities[i]->normalKey), 1);
		graphics->SetTexture(assetManager->GetTexture(entities[i]->metallicKey), 2);
		graphics->SetTexture(assetManager->GetTexture(entities[i]->roughnessKey), 3);
		graphics->SetTexture(assetManager->GetTexture(entities[i]->aoKey), 4);
		graphics->SetTexture(assetManager->GetTexture("BRDF_LUT"), 5);
		graphics->SetTexture(assetManager->GetTexture("SM_IrrMap"), 6);
		graphics->SetTexture(assetManager->GetTexture("SM_SpecMap"), 7);

		Model* model = assetManager->GetModel(entities[i]->modelKey);

		for (size_t j = 0; j < model->meshes.size(); ++j)
		{
			graphics->DrawShadowedMesh(model->meshes[j]); // If we have a bunch of the same object and we have time, I can add a render for instancing
		}
	}

	/* Draw the Skybox Last */
	graphics->SetVertexShader(assetManager->GetVertexShader(std::string("SkyboxVS")));
	graphics->SetPixelShader(assetManager->GetPixelShader(std::string("SkyboxPS")));
	CalculateProjViewWorldMatrix(entities[0]); // Update the main matrix in the vertex shader constants area of the entity

	skyboxVShaderConstants.projection = camera->projection;
	skyboxVShaderConstants.view = camera->view;
	graphics->SetConstantBufferVS(skyboxVShaderConstantBuffer, &skyboxVShaderConstants, sizeof(SkyboxVShaderConstants));
	graphics->SetTexture(assetManager->GetTexture("SM_EnvMap"), 0);

	Model* model = assetManager->GetModel(entities[0]->modelKey);
	graphics->DrawSkybox(model->meshes[0]);

	graphics->EndFrame();
}

void Engine::ShutDown()
{
	pixelShaderConstantBuffer->Release();
	skyboxVShaderConstantBuffer->Release();
	for (auto& e : entities)
	{
		e->vertexShaderConstantBuffer->Release();
		delete e;
	}
	delete camera;
	assetManager->Destroy();
	graphics->DestroyGraphics();
	delete assetManager;
	delete graphics;
}

void Engine::CalculateProjectionMatrix()
{
	XMStoreFloat4x4(&camera->projection, XMMatrixTranspose(XMMatrixPerspectiveFovLH(fov, graphics->aspectRatio, 0.1f, 1000.0f))); // Update with new w/h
}

void Engine::CalculateProjViewMatrix()
{
	XMMATRIX view = XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&camera->position), XMLoadFloat3(&camera->direction), XMLoadFloat3(&camera->up)));
	XMStoreFloat4x4(&camera->view, view);
	XMStoreFloat4x4(&camera->projView, XMLoadFloat4x4(&camera->projection) * view);
}

void Engine::CalculateProjViewWorldMatrix(GameEntity* entity)
{
	XMMATRIX world = XMMatrixTranspose(XMMatrixScaling(entity->scale.x, entity->scale.y, entity->scale.z) * XMMatrixRotationQuaternion(XMLoadFloat4(&entity->rotationQuaternion)) * XMMatrixTranslation(entity->position.x, entity->position.y, entity->position.z));
	XMStoreFloat4x4(&entity->vertexShaderConstants.world, world);
	XMStoreFloat4x4(&entity->vertexShaderConstants.projViewWorld, XMLoadFloat4x4(&camera->projView) * world); // This result is already transpose, as the individual matrices are transpose and the mult order is reversed
}

void Engine::CalculateShadowMapProjectionMatrix()
{
	XMStoreFloat4x4(&shadowMapProj, XMMatrixTranspose(XMMatrixOrthographicLH(80.0f, 45.0f, 0.1f, 100.0f))); // Update with new w/h
}

void Engine::CalculateShadowMapProjViewMatrix()
{
	XMMATRIX view = XMMatrixTranspose(XMMatrixLookAtLH(XMLoadFloat3(&pixelShaderConstants.lightPos1), XMVectorSet(0.f, 0.f, 0.5f, 0.f), XMVectorSet(1.f, 0.f, 0.f, 0.f)));
	XMStoreFloat4x4(&shadowMapView, view);
	XMStoreFloat4x4(&shadowMapProjView, XMLoadFloat4x4(&shadowMapProj) * view);
}

void Engine::CalculateShadowMapProjViewWorldMatrix(GameEntity* entity)
{
	XMMATRIX world = XMMatrixTranspose(XMMatrixScaling(entity->scale.x, entity->scale.y, entity->scale.z) * XMMatrixRotationQuaternion(XMLoadFloat4(&entity->rotationQuaternion)) * XMMatrixTranslation(entity->position.x, entity->position.y, entity->position.z));
	//XMStoreFloat4x4(&entity->vertexShaderConstants.world, world);
	XMStoreFloat4x4(&entity->vertexShaderConstants.shadowProjViewWorld, XMLoadFloat4x4(&shadowMapProjView) * world); // This result is already transpose, as the individual matrices are transpose and the mult order is reversed
}
