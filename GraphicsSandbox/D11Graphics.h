#pragma once
#define NOMINMAX
#include <Windows.h>
#include <WindowsX.h>
#include <d3d11.h>
#include <string>
#include "Structs.h"
#include "Mesh.h"
#define DISPLAY_WIDTH				1280
#define DISPLAY_HEIGHT				720

class D11Graphics
{
public:
	D11Graphics();
	~D11Graphics();

	float aspectRatio;
	XMFLOAT2 rotateBy; // Holds the rotation for the camera

	void UpdateStats(float totalTime);

	bool Initialize();
	bool HandleLowLevelEvents(bool quit);
	void BeginNewFrame();
	void EndFrame();
	void PerformBloomBlurPass(ID3D11PixelShader* verticalBlurShader, ID3D11PixelShader* horizontalBlurShader);
	void PerformFinalCombine();
	void DestroyGraphics();

	ID3D11Buffer* CreateVertexBuffer(Vertex* vertices, uint32_t numVerts);
	ID3D11Buffer* CreateIndexBuffer(uint32_t* indices, uint32_t numInds);
	ID3D11Buffer* CreateConstantBuffer(void* constantData, size_t size);
	VertexShader LoadVertexShader(const char* filepath);
	ID3D11PixelShader* LoadPixelShader(const char* filepath);
	ID3D11ShaderResourceView* LoadWICTexture(const char* filepath);
	ID3D11ShaderResourceView* LoadDDSTexture(const char* filepath);

	void SetVertexShader(VertexShader* shader);
	void SetPixelShader(ID3D11PixelShader* shader);
	void SetTexture(ID3D11ShaderResourceView* texture, int shaderSlot);
	void SetConstantBufferVS(ID3D11Buffer* buffer, void* constantData, size_t size);
	void SetConstantBufferPS(ID3D11Buffer* buffer, void* constantData, size_t size);
	void DrawMesh(Mesh* mesh);
	void DrawSkybox(Mesh* unitCube);

	void OnMouseMove(WPARAM buttonState, int x, int y);

private:
	void SetupWindow();

	static D11Graphics* D11GraphicsInstance;
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam); // Handles OS-level inputs so the app doesn't freeze
	void Resize();


	HINSTANCE hInstance; // App and window handles
	HWND hWnd;
	MSG msg = {};
	D3D_FEATURE_LEVEL		dxFeatureLevel; // Base DX variables
	IDXGISwapChain*			swapChain;
	ID3D11Device*			device;
	ID3D11DeviceContext*	context;

	ID3D11RenderTargetView* sceneAndBloomRTVs[2];
	ID3D11RenderTargetView* bloomPongRTV;
	ID3D11ShaderResourceView* bloomSRV; // Ping and pong textures
	ID3D11ShaderResourceView* bloomPongSRV;
	ID3D11ShaderResourceView* sceneSRV;
	ID3D11Buffer* blurPixelShaderConstantBuffer = nullptr;

	ID3D11ShaderResourceView* blankSRVs[7];

	ID3D11RenderTargetView* backBufferRTV;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11DepthStencilState* depthWriteOffState;
	D3D11_VIEWPORT viewport;


	ID3D11SamplerState* sampler;
	ID3D11RasterizerState* skyRasterState;
	ID3D11DepthStencilState* skyDepthState;
	ID3D11RasterizerState* normalRasterState;

	uint16_t windowWidth = DISPLAY_WIDTH;
	uint16_t windowHeight = DISPLAY_HEIGHT;

	int fpsFrameCount;
	float fpsTimeElapsed;
	std::string titleBarText;
	POINT prevMousePos;
};

