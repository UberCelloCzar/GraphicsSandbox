// Code written by Trevor Walden, some code referenced from prior work with Chris Cascioli
#include "D11Graphics.h"
#include <d3dcompiler.h>
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include <sstream>
#include <atlcomcli.h>

D11Graphics* D11Graphics::D11GraphicsInstance = 0;


D11Graphics::D11Graphics()
{
}


D11Graphics::~D11Graphics()
{
}

void D11Graphics::UpdateStats(float totalTime)
{
	fpsFrameCount++;

	float timeDiff = totalTime - fpsTimeElapsed; // Only calc FPS and update title bar once per second
	if (timeDiff < 1.0f) return;

	float mspf = 1000.0f / (float)fpsFrameCount; // Frame time

	std::ostringstream output;
	output.precision(6);
	output << titleBarText <<
		"    Width: " << windowWidth <<
		"    Height: " << windowHeight <<
		"    FPS: " << fpsFrameCount <<
		"    Frame Time: " << mspf << "ms";

	SetWindowText(hWnd, output.str().c_str());
	fpsFrameCount = 0;
	fpsTimeElapsed += 1.0f;
}

bool D11Graphics::Initialize()
{
	D11GraphicsInstance = this;
	SetupWindow();
	fpsFrameCount = 0;
	fpsTimeElapsed = 0.0f;
	aspectRatio = (float)windowWidth / (float)windowHeight;
	rotateBy = XMFLOAT2(0, 0);

	unsigned int deviceFlags = 0; // DX init options

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG; // Get more detailed roprts from the dx api
#endif

	HRESULT result = S_OK;

	DXGI_SWAP_CHAIN_DESC swapDesc = {}; // Swap chain creation
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Width = windowWidth;
	swapDesc.BufferDesc.Height = windowHeight;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60; // 60Hz
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = hWnd;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;

	// Init DirectX
	result = D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, 0, 0, D3D11_SDK_VERSION, &swapDesc, &swapChain, &device, &dxFeatureLevel, &context);
	if (FAILED(result))
	{
		printf("d3d initialization failed");
		return false;
	}

	ID3D11Texture2D* backBufferTexture; // Grab the texture the init created then make an RTV for it
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
	device->CreateRenderTargetView(backBufferTexture, 0, &backBufferRTV);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc = {}; // Depth buffer texture setup
	depthStencilDesc.Width = windowWidth;
	depthStencilDesc.Height = windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* depthBufferTexture; // Make the depth buffer
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
	device->CreateDepthStencilView(depthBufferTexture, 0, &depthStencilView);
	depthBufferTexture->Release();

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView); // Bind the views

	viewport = {}; // Set up the viewport
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)windowWidth;
	viewport.Height = (float)windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	context->RSSetViewports(1, &viewport); // Bind the viewport


	D3D11_SAMPLER_DESC samplerDesc = {}; // Create the sampler
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &sampler);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/* Skybox states */
	D3D11_RASTERIZER_DESC rd = {}; // Rasterizer state for drawing the inside of the cube
	rd.CullMode = D3D11_CULL_FRONT;
	rd.FrontCounterClockwise = false;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.DepthClipEnable = true;
	device->CreateRasterizerState(&rd, &skyRasterState);

	rd.CullMode = D3D11_CULL_BACK;
	device->CreateRasterizerState(&rd, &normalRasterState); // A normal state to go back to

	D3D11_DEPTH_STENCIL_DESC ds = {}; // Depth stencil state for ignoring obscured pixels and drawing at max depth
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	/* Set up gbuffer */
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC textureDesc = {}; // Standard fare RTVs, just set up the textures, then make the rtv and srv, and throw away the ref to the texture
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < 3; i++)
	{
		device->CreateTexture2D(&textureDesc, NULL, &texture);
		device->CreateRenderTargetView(texture, &rtvDesc, &gBufferRTVs[i]);
		device->CreateShaderResourceView(texture, &srvDesc, &gBufferSRVs[i]);
		texture->Release();
	}

	return true;
}

bool D11Graphics::HandleLowLevelEvents(bool quit)
{
	if (quit) PostMessage(hWnd, WM_CLOSE, NULL, NULL); // Quit if the esc key is hit

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) // Check for OS messages
	{
		TranslateMessage(&msg); // Ready and dispatch for interpretation
		DispatchMessage(&msg);
	}

	return quit;
}

void D11Graphics::BeginNewFrame()
{
	context->RSSetState(normalRasterState);
	context->OMSetDepthStencilState(0, 0);
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	context->RSSetViewports(1, &viewport);

	const float color[4] = { 0,1,1,0 };
	context->ClearRenderTargetView(backBufferRTV, color); // Will remove this once skyboxes work
	context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->PSSetSamplers(0, 1, &sampler);
}

void D11Graphics::EndFrame()
{
	swapChain->Present(0, 0);
}

void D11Graphics::DestroyGraphics()
{
	sampler->Release();
	normalRasterState->Release();
	skyRasterState->Release();
	skyDepthState->Release();

	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	for (int i = 0; i < 3; i++)
	{
		gBufferRTVs[i]->Release();
		gBufferSRVs[i]->Release();
	}

	if (swapChain) { swapChain->Release(); }
	if (context) { context->Release(); }
#ifdef _DEBUG
	CComPtr<ID3D11Debug> pDebug;
	HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&pDebug));
#endif
	if (device) { device->Release(); }
#ifdef _DEBUG
	if (pDebug != nullptr)
	{
		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		pDebug = nullptr;
	}
#endif

	D11GraphicsInstance = nullptr;
}

ID3D11Buffer* D11Graphics::CreateVertexBuffer(Vertex* vertices, uint32_t numVerts)
{
	ID3D11Buffer* vertexBuffer;

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * numVerts;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialVertexData;
	initialVertexData.pSysMem = vertices;
	HRESULT result = device->CreateBuffer(&vbd, &initialVertexData, &vertexBuffer);
	if (result != S_OK)
	{
		printf("vertex buffer creation failed\n");
		return nullptr;
	}

	return vertexBuffer;
}

ID3D11Buffer* D11Graphics::CreateIndexBuffer(uint32_t* indices, uint32_t numInds)
{
	ID3D11Buffer* indexBuffer;

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(uint32_t) * numInds; // Number of indices
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initialIndexData;
	initialIndexData.pSysMem = indices;
	HRESULT result = device->CreateBuffer(&ibd, &initialIndexData, &indexBuffer);
	if (result != S_OK)
	{
		printf("index buffer creation failed\n");
		return nullptr;
	}

	return indexBuffer;
}

ID3D11Buffer* D11Graphics::CreateConstantBuffer(void* constantData, size_t size)
{
	ID3D11Buffer* constantBuffer;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = size;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresourceData; // The description of the data to fill it with
	subresourceData.pSysMem = constantData; // If we're creating it every frame we might as well copy the data while we have it
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	// Create the buffer.
	HRESULT result = device->CreateBuffer(&cbDesc, &subresourceData, &constantBuffer);

	if (FAILED(result))
	{
		printf("constant buffer creation failed\n");
		return nullptr;
	}

	return constantBuffer;
}

VertexShader D11Graphics::LoadVertexShader(const char* filepath)
{
	ID3DBlob* shaderBin;
	ID3D11VertexShader* vertexShader;

	std::string fp(filepath);
	std::wstring wfp(fp.begin(), fp.end());
	wfp = L"../x64/Debug/" + wfp + L".cso";

	HRESULT result = D3DReadFileToBlob(wfp.c_str(), &shaderBin);
	if (result != S_OK)
	{
		printf("failed to read vertex shader file\n");
		return VertexShader();
	}

	result = device->CreateVertexShader(shaderBin->GetBufferPointer(), shaderBin->GetBufferSize(), 0, &vertexShader);

	if (result != S_OK)
	{
		printf("vertex shader creation failed\n");
		shaderBin->Release();
		return VertexShader();
	}

	// Ordinarily here I'd set up a shader reflection and grab the info from the shader to be modular, but I know our shaders will always have this layout
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11InputLayout* inputLayout;
	result = device->CreateInputLayout(inputDesc, 4, shaderBin->GetBufferPointer(), shaderBin->GetBufferSize(), &inputLayout);
	if (result != S_OK)
	{
		printf("input layout creation failed\n");
		shaderBin->Release();
		return VertexShader();
	}

	shaderBin->Release();
	VertexShader vs = {};
	vs.shaderPointer = vertexShader;
	vs.inputLayout = inputLayout;

	return vs;
}

ID3D11PixelShader* D11Graphics::LoadPixelShader(const char* filepath)
{
	ID3DBlob* shaderBin;
	ID3D11PixelShader* pixelShader;

	std::string fp(filepath);
	std::wstring wfp(fp.begin(), fp.end());
	wfp = L"../x64/Debug/" + wfp + L".cso";

	HRESULT result = D3DReadFileToBlob(wfp.c_str(), &shaderBin);
	if (result != S_OK)
	{
		printf("failed to read vertex shader file\n");
		return nullptr;
	}

	result = device->CreatePixelShader(shaderBin->GetBufferPointer(), shaderBin->GetBufferSize(), 0, &pixelShader);

	if (result != S_OK)
	{
		printf("vertex shader creation failed\n");
		shaderBin->Release();
		return nullptr;
	}

	shaderBin->Release();

	return pixelShader;
}

ID3D11ShaderResourceView* D11Graphics::LoadWICTexture(const char* filepath)
{
	std::string fp(filepath);
	fp = "../Textures/" + fp;
	std::wstring wfp(fp.begin(), fp.end());

	ID3D11ShaderResourceView* textureSRV = nullptr;
	HRESULT result = DirectX::CreateWICTextureFromFile(device, context, wfp.c_str(), 0, &textureSRV);
	if (result != S_OK)
	{
		printf("wic texture creation failed: %s\n", std::to_string(result).c_str());
		return nullptr;
	}

	return textureSRV;
}

ID3D11ShaderResourceView* D11Graphics::LoadDDSTexture(const char* filepath)
{
	std::string fp(filepath);
	fp = "../Textures/" + fp + ".dds";
	std::wstring wfp(fp.begin(), fp.end());

	ID3D11ShaderResourceView* textureSRV = nullptr;
	HRESULT result = DirectX::CreateDDSTextureFromFile(device, context, wfp.c_str(), 0, &textureSRV);

	if (result != S_OK)
	{
		printf("dds texture creation failed: %s\n", std::to_string(result).c_str());
		return nullptr;
	}

	return textureSRV;
}

void D11Graphics::SetVertexShader(VertexShader* shader)
{
	ID3D11VertexShader* vertexShader = shader->shaderPointer;
	ID3D11InputLayout* inputLayout = shader->inputLayout;
	context->IASetInputLayout(inputLayout);
	context->VSSetShader(vertexShader, 0, 0);
}

void D11Graphics::SetPixelShader(ID3D11PixelShader* shader)
{
	ID3D11PixelShader* pixelShader = shader;
	context->PSSetShader(pixelShader, 0, 0);
}

void D11Graphics::SetTexture(ID3D11ShaderResourceView* texture, int shaderSlot)
{
	context->PSSetShaderResources(shaderSlot, 1, &texture);
}

void D11Graphics::SetConstantBufferVS(ID3D11Buffer* buffer, void* constantData, size_t size)
{
	context->UpdateSubresource(buffer, 0, 0, constantData, 0, 0);
	context->VSSetConstantBuffers(0, 1, &buffer);
}

void D11Graphics::SetConstantBufferPS(ID3D11Buffer* buffer, void* constantData, size_t size)
{
	context->UpdateSubresource(buffer, 0, 0, constantData, 0, 0);
	context->PSSetConstantBuffers(0, 1, &buffer);
}

void D11Graphics::DrawMesh(Mesh* mesh)
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &(mesh->vertexBuffer), &stride, &offset);

	context->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	context->DrawIndexed(mesh->numIndices, 0, 0);
}

void D11Graphics::DrawSkybox(Mesh* unitCube)
{
	context->RSSetState(skyRasterState);
	context->OMSetDepthStencilState(skyDepthState, 0);
	DrawMesh(unitCube);
}

void D11Graphics::SetupWindow()
{
	// Our temp console info struct
	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	// Get the console info and set the number of lines
	AllocConsole();
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	coninfo.dwSize.X = 120;
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Right = 120;
	rect.Bottom = 32;
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);

	FILE *stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);

	// Prevent accidental console window close
	HWND consoleHandle = GetConsoleWindow();
	HMENU hmenu = GetSystemMenu(consoleHandle, FALSE);
	EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);

	/*Window*/
	WNDCLASS wndClass = {}; // Window definition struct
	wndClass.style = CS_HREDRAW | CS_VREDRAW;	// Redraw on horizontal or vertical movement/adjustment
	wndClass.lpfnWndProc = WindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "D3DWindowClass";

	if (!RegisterClass(&wndClass)) // Register the window class
	{
		DWORD error = GetLastError();
		if (error != ERROR_CLASS_ALREADY_EXISTS)
		{
			printf("window class register error: %s\n", std::to_string(HRESULT_FROM_WIN32(error)).c_str());
			return;
		}
	}

	RECT clientRect; // Set the size properly
	SetRect(&clientRect, 0, 0, windowWidth, windowHeight);
	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, false);

	RECT desktopRect; // Center the window
	GetClientRect(GetDesktopWindow(), &desktopRect);
	int centeredX = (desktopRect.right / 2) - (clientRect.right / 2);
	int centeredY = (desktopRect.bottom / 2) - (clientRect.bottom / 2);

	// Actually ask Windows to create the window itself
	// using our settings so far.  This will return the
	// handle of the window, which we'll keep around for later
	hWnd = CreateWindow(wndClass.lpszClassName, "DFP", WS_OVERLAPPEDWINDOW, centeredX, centeredY, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, 0, 0, hInstance, 0);
	if (hWnd == NULL)
	{
		DWORD error = GetLastError();
		printf("window creation error: %s\n", std::to_string(HRESULT_FROM_WIN32(error)).c_str());
		return;
	}

	ShowWindow(hWnd, SW_SHOW); // Tell windows to present the window

	// Return an "everything is ok" HRESULT value
	printf("window created successfully\n");

}

LRESULT D11Graphics::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return D11GraphicsInstance->ProcessMessage(hWnd, uMsg, wParam, lParam);
}

LRESULT D11Graphics::ProcessMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) // Handle incoming messages
	{
	case WM_DESTROY:
		PostQuitMessage(0); // Close app when X button is clicked
		return 0;

	case WM_MENUCHAR: // No beeps on alt-enter
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO: // Maintain minimum size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

		// Sent when the window size changes
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) return 0;
		windowWidth = LOWORD(lParam); // Save new dimensions
		windowHeight = HIWORD(lParam);
		aspectRatio = (float)windowWidth / (float)windowHeight;

		if (device) Resize(); // Change render size
		return 0;

	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void D11Graphics::Resize() // TODO: deferred rendering support
{
	if (depthStencilView) { depthStencilView->Release(); }
	if (backBufferRTV) { backBufferRTV->Release(); }

	for (int i = 0; i < 3; i++)
	{
		gBufferRTVs[i]->Release();
		gBufferSRVs[i]->Release();
	}

	swapChain->ResizeBuffers(1, windowWidth, windowHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	ID3D11Texture2D* backBufferTexture;
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBufferTexture));
	device->CreateRenderTargetView(backBufferTexture, 0, &backBufferRTV);
	backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = windowWidth;
	depthStencilDesc.Height = windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;

	ID3D11Texture2D* depthBufferTexture;
	device->CreateTexture2D(&depthStencilDesc, 0, &depthBufferTexture);
	device->CreateDepthStencilView(depthBufferTexture, 0, &depthStencilView);
	depthBufferTexture->Release();

	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)windowWidth;
	viewport.Height = (float)windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	/* Set up gbuffer */
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC textureDesc = {}; // Standard fare RTVs, just set up the textures, then make the rtv and srv, and throw away the ref to the texture
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < 3; i++)
	{
		device->CreateTexture2D(&textureDesc, NULL, &texture);
		device->CreateRenderTargetView(texture, &rtvDesc, &gBufferRTVs[i]);
		device->CreateShaderResourceView(texture, &srvDesc, &gBufferSRVs[i]);
		texture->Release();
	}
}

void D11Graphics::OnMouseMove(WPARAM buttonState, int x, int y)
{
	if (prevMousePos.x != NULL && buttonState & 0x0001)
	{
		rotateBy.x = (x - prevMousePos.x) * .005f;
		rotateBy.y = (y - prevMousePos.y) * .005f; // Rotate the camera
		//printf("%f, %f\n", rotateBy.x, rotateBy.y);
	}
	
	prevMousePos.x = x;
	prevMousePos.y = y;
}
