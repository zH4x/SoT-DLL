//Made by @ImShotZz
//https://www.unknowncheats.me/forum/members/2116480.html

//d3d11 w2s for ut4 engine games by n7
//credits to @Griizz

#include <vector>
#include <sstream>
#include <chrono>
#include <memory>
#include <string>
#include <iomanip>

#include <d3d11.h>
#include <D3D11Shader.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")

#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h"

#include "global.h"
#include "bones.h"
#include "hooks.h"
#include "renderer.h"

#include "main.h"
#include "util.h"
#include <map>

#define FONT_SIZE 11.0f
#define FONT_OFFSET 16.0f
#define FONT_TYPE L"Verdana"
#define RED_COLOR Color{ 1.0f, 0.0f, 0.0f, 1.0f }
#define GREEN_COLOR Color{ 0.0f, 1.0f, 0.0f, 1.0f }
#define BLACK_COLOR Color{ 0.0f, 0.0f, 0.0f, 0.9f }
#define YELLOW_COLOR Color{ 1.0f, 1.0f, 0.0f, 1.0f }
#define TRANSPARENT_COLOR Color{ 1.0f, 1.0f, 1.0f, 0.1f }

void GetAddresses()
{
	Global::BaseAddress = (DWORD_PTR)GetModuleHandle(NULL);
	GetModuleInformation(GetCurrentProcess(), (HMODULE)Global::BaseAddress, &Global::info, sizeof(Global::info));

	auto AddrUWorld = Util::FindPattern((PBYTE)Global::BaseAddress, Global::info.SizeOfImage, (PBYTE)"\x48\x89\x05\x00\x00\x00\x00\x0F\x28\xD6", "xxx????xxx", 0);
	auto OffUWorld = *reinterpret_cast<uint32_t*>(AddrUWorld + 3);
	Global::m_UWorld = reinterpret_cast<SDK::UWorld**>(AddrUWorld + 7 + OffUWorld);

	auto AddrGObj = Util::FindPattern((PBYTE)Global::BaseAddress, Global::info.SizeOfImage, (PBYTE)"\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x89\x43\x04\x48\x83\xC4\x20\x5B\xC3", "xxx????x????xxxxxxxxx", 0);
	auto OffGObj = *reinterpret_cast<uint32_t*>(AddrGObj + 3);
	SDK::UObject::GObjects = reinterpret_cast<SDK::FUObjectArray*>(AddrGObj + 7 + OffGObj);

	auto AddrGName = Util::FindPattern((PBYTE)Global::BaseAddress, Global::info.SizeOfImage, (PBYTE)"\x48\x8B\x35\x00\x00\x00\x00\x41\x0F\xB7\xC4\x4D\x8D\xA5\x00\x00\x00\x00\x49", "xxx????xxxxxxx????x", 0);
	auto OffGName = *reinterpret_cast<uint32_t*>(AddrGName + 3);
	SDK::FName::GNames = *reinterpret_cast<SDK::TNameEntryArray**>(AddrGName + 7 + OffGName);

	Util::Engine::boneAddress = (DWORD_PTR)Util::FindPattern((PBYTE)Global::BaseAddress, Global::info.SizeOfImage, (PBYTE)"\x40\x53\x55\x57\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x33\xED\x4D\x63\xF0", "xxxxxxxxx????xxxxx", 0);
}

DWORD WINAPI UpdateThread(LPVOID)
{
	GetAddresses();

	printf("UWorld: 0x%X\n", Global::m_UWorld);
	printf("GObjects: 0x%X\n", SDK::UObject::GObjects);
	printf("GNames: 0x%X\n", SDK::FName::GNames);
	printf("Bones: 0x%X\n\n", Util::Engine::boneAddress);

	while (true)
	{
		if ((*Global::m_UWorld) == nullptr)
			continue;

		Global::m_PersistentLevel = (*Global::m_UWorld)->PersistentLevel;
		Global::m_LocalPlayer = (*Global::m_UWorld)->OwningGameInstance->LocalPlayers[0];
		Global::m_AActors = &Global::m_PersistentLevel->AActors;

		Sleep(1);
	}
	return NULL;
}

std::unique_ptr<Renderer> renderer;
void DrawEsp()
{
	if (Global::m_PersistentLevel != nullptr && Global::m_LocalPlayer != nullptr && Global::m_LocalPlayer->PlayerController->AcknowledgedPawn != nullptr)
	{
		SDK::TArray<SDK::AActor*> actors = Global::m_PersistentLevel->AActors;
		for (int i = 0; i < actors.Num(); i++)
		{
			SDK::AActor* actor = Global::m_PersistentLevel->AActors[i];
			if (actor == nullptr || actor->RootComponent == nullptr)
				continue;

			if (actor->IsA(SDK::ABP_PlayerPirate_C::StaticClass()))
			{
				SDK::ABP_PlayerPirate_C* pawn = static_cast<SDK::ABP_PlayerPirate_C*>(actor);
				if (pawn == nullptr || pawn->PlayerState == nullptr)
					continue;

				if (Util::IsLocalPlayer(actor))
					continue;

				SDK::APlayerState* state = static_cast<SDK::APlayerState*>(pawn->PlayerState);
				if (!state->PlayerName.IsValid() || pawn->RootComponent == nullptr)
					continue;

				// text size
				Vec2 size;

				// Get Name
				std::wstring playerName = state->PlayerName.c_str();

				// Draw Box
				SDK::FVector HeadPos, LeftFootPos, RightFootPos, BottomPos;
				Util::Engine::GetBoneLocation(pawn->Mesh, &HeadPos, Head);
				Util::Engine::GetBoneLocation(pawn->Mesh, &LeftFootPos, Foot_l);
				Util::Engine::GetBoneLocation(pawn->Mesh, &RightFootPos, Foot_r);

				BottomPos = Util::Vector::Divide(Util::Vector::Add(LeftFootPos, RightFootPos), 2);

				HeadPos.Z += 20.f;
				BottomPos.Z -= 20.f;

				SDK::FVector2D ScreenHead, ScreenBottom;
				Global::m_LocalPlayer->PlayerController->ProjectWorldLocationToScreen(HeadPos, &ScreenHead);
				Global::m_LocalPlayer->PlayerController->ProjectWorldLocationToScreen(BottomPos, &ScreenBottom);
				//Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, HeadPos, &ScreenHead);
				//Util::Engine::WorldToScreen(Global::m_LocalPlayer->PlayerController, BottomPos, &ScreenBottom);

				if (ScreenHead.X == 0 && ScreenHead.Y == 0)
					continue;

				float height = ScreenBottom.Y - ScreenHead.Y;
				float width = height * 0.6f;

				renderer->drawOutlinedRect(Vec4(ScreenHead.X - width / 2, ScreenHead.Y, width, height), 2.f, RED_COLOR, TRANSPARENT_COLOR);

				// Get Distance
				float distance = Util::GetDistance(Global::m_LocalPlayer->PlayerController->AcknowledgedPawn->RootComponent->RelativeLocation, HeadPos);
				std::wstring distanceText = Util::DistanceToString(distance);

				//Draw Name
				size = renderer->getTextExtent(playerName, FONT_SIZE, FONT_TYPE);
				renderer->drawText(Vec2(ScreenHead.X - size.x / 2, ScreenHead.Y - 13.f), playerName, RED_COLOR, 0, FONT_SIZE, FONT_TYPE);

				// Draw Distance
				size = renderer->getTextExtent(distanceText, FONT_SIZE, FONT_TYPE);
				renderer->drawText(Vec2(ScreenHead.X - size.x / 2, ScreenHead.Y + height + 1.f), distanceText, RED_COLOR, 0, FONT_SIZE, FONT_TYPE);
			}
			/*else if (actor->IsA(SDK::ABP__C::StaticClass()))
			{
				SDK::ABMXTest_C* pawn = static_cast<SDK::ABMXTest_C*>(actor);
				if (pawn == nullptr)
					continue;

				if (pawn->RootComponent == nullptr)
					continue;

				SDK::FVector pos = pawn->RootComponent->RelativeLocation;
				SDK::FVector2D screenPos;
				Global::m_LocalPlayer->PlayerController->ProjectWorldLocationToScreen(pos, &screenPos);

				if (screenPos.X == 0 && screenPos.Y == 0)
					continue;

				// Get Distance
				float distance = Util::GetDistance(Global::m_LocalPlayer->PlayerController->AcknowledgedPawn->RootComponent->RelativeLocation, pos);
				std::wstring distanceText = Util::DistanceToString(distance);

				// Draw Distance
				Vec2 size = renderer->getTextExtent(distanceText, FONT_SIZE, FONT_TYPE);
				renderer->drawText(Vec2(screenPos.X - size.x / 2, screenPos.Y), L"Bike [" + distanceText + L"]", YELLOW_COLOR, 0, FONT_SIZE, FONT_TYPE);
			}*/
		}
	}
}

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (firstTime)
	{
		//get device
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}

		//create depthstencilstate
		D3D11_DEPTH_STENCIL_DESC stencilDesc;
		stencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		stencilDesc.StencilEnable = true;
		stencilDesc.StencilReadMask = 0xFF;
		stencilDesc.StencilWriteMask = 0xFF;
		stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		stencilDesc.DepthEnable = true;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::ENABLED)]);

		stencilDesc.DepthEnable = false;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::DISABLED)]);

		stencilDesc.DepthEnable = false;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		stencilDesc.StencilEnable = false;
		stencilDesc.StencilReadMask = UINT8(0xFF);
		stencilDesc.StencilWriteMask = 0x0;
		pDevice->CreateDepthStencilState(&stencilDesc,
		                                 &myDepthStencilStates[static_cast<int>(eDepthState::NO_READ_NO_WRITE)]);

		stencilDesc.DepthEnable = true;
		stencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //
		stencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		stencilDesc.StencilEnable = false;
		stencilDesc.StencilReadMask = UINT8(0xFF);
		stencilDesc.StencilWriteMask = 0x0;

		stencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		stencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

		stencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
		stencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
		pDevice->CreateDepthStencilState(&stencilDesc, &myDepthStencilStates[static_cast<int>(eDepthState::READ_NO_WRITE)]);

		//use the back buffer address to create the render target
		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&RenderTargetTexture)))
		{
			pDevice->CreateRenderTargetView(RenderTargetTexture, nullptr, &RenderTargetView);
			RenderTargetTexture->Release();
		}

		renderer = std::make_unique<Renderer>(pDevice);
		firstTime = false;
	}

	// shaders
	if (!psRed)
	{
		GenerateShader(pDevice, &psRed, 0.85f, 0.0f, 0.0f);
	}

	if (!psGreen)
	{
		GenerateShader(pDevice, &psGreen, 0.0f, 0.85f, 0.0f);
	}

	// viewport
	pContext->RSGetViewports(&vps, &viewport);
	ScreenCenterX = viewport.Width / 2.0f;
	ScreenCenterY = viewport.Height / 2.0f;

	// call before you draw
	pContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

	renderer->begin();
	renderer->drawText(Vec2(14.0f, 10.0f), L"Made by Zanzo", RED_COLOR, 0, 12.0f, FONT_TYPE);

	if ((*Global::m_UWorld) != nullptr)
		DrawEsp();

	renderer->draw();
	renderer->end();

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

//==========================================================================================================================

void __stdcall hookD3D11DrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation,
                                    INT BaseVertexLocation)
{
	//get stride & vdesc.ByteWidth
	pContext->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
	if (veBuffer)
		veBuffer->GetDesc(&vedesc);
	if (veBuffer != nullptr)
	{
		veBuffer->Release();
		veBuffer = nullptr;
	}

	//get indesc.ByteWidth
	pContext->IAGetIndexBuffer(&inBuffer, &inFormat, &inOffset);
	if (inBuffer)
		inBuffer->GetDesc(&indesc);
	if (inBuffer != nullptr)
	{
		inBuffer->Release();
		inBuffer = nullptr;
	}

	//get pscdesc.ByteWidth
	pContext->PSGetConstantBuffers(pscStartSlot, 1, &pcsBuffer);
	if (pcsBuffer)
		pcsBuffer->GetDesc(&pscdesc);
	if (pcsBuffer != nullptr)
	{
		pcsBuffer->Release();
		pcsBuffer = nullptr;
	}

	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

//==========================================================================================================================

void __stdcall hookD3D11PSSetShaderResources(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews,
                                             ID3D11ShaderResourceView*const * ppShaderResourceViews)
{
	pssrStartSlot = StartSlot;

	for (UINT j = 0; j < NumViews; j++)
	{
		//resources loop
		ID3D11ShaderResourceView* pShaderResView = ppShaderResourceViews[j];
		if (pShaderResView)
		{
			pShaderResView->GetDesc(&Descr);

			ID3D11Resource* Resource;
			pShaderResView->GetResource(&Resource);
			auto* Texture = (ID3D11Texture2D *)Resource;
			Texture->GetDesc(&texdesc);
		}
	}

	return phookD3D11PSSetShaderResources(pContext, StartSlot, NumViews, ppShaderResourceViews);
}

//==========================================================================================================================

void __stdcall hookD3D11CreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC* pQueryDesc, ID3D11Query** ppQuery)
{
	return phookD3D11CreateQuery(pDevice, pQueryDesc, ppQuery);
}

//==========================================================================================================================

void __stdcall hookD3D11DrawIndexedInstanced(ID3D11DeviceContext* pContext, UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation)
{
	if (GetAsyncKeyState(VK_F9) & 1)

	return phookD3D11DrawIndexedInstanced(pContext, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

//==========================================================================================================================

void __stdcall hookD3D11Draw(ID3D11DeviceContext* pContext, UINT VertexCount, UINT StartVertexLocation)
{
	if (GetAsyncKeyState(VK_F9) & 1)

	return phookD3D11Draw(pContext, VertexCount, StartVertexLocation);
}

//==========================================================================================================================

void __stdcall hookD3D11DrawInstanced(ID3D11DeviceContext* pContext, UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation)
{
	if (GetAsyncKeyState(VK_F9) & 1)

	return phookD3D11DrawInstanced(pContext, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

//==========================================================================================================================

void __stdcall hookD3D11DrawInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	if (GetAsyncKeyState(VK_F9) & 1)

	return phookD3D11DrawInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

//==========================================================================================================================

void __stdcall hookD3D11DrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
{
	if (GetAsyncKeyState(VK_F9) & 1)

	return phookD3D11DrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
}

//==========================================================================================================================

void __stdcall hookD3D11VSSetConstantBuffers(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumBuffers, ID3D11Buffer*const * ppConstantBuffers)
{
	if (ppConstantBuffers != nullptr)
		vsStartSlot = StartSlot;

	return phookD3D11VSSetConstantBuffers(pContext, StartSlot, NumBuffers, ppConstantBuffers);
}

//==========================================================================================================================

void __stdcall hookD3D11PSSetSamplers(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumSamplers, ID3D11SamplerState*const * ppSamplers)
{
	if (ppSamplers != nullptr)
		psStartSlot = StartSlot;

	return phookD3D11PSSetSamplers(pContext, StartSlot, NumSamplers, ppSamplers);
}

//=========================================================================================================================

const int MultisampleCount = 1; // Set to 1 to disable multisampling
LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD __stdcall InitializeHook(LPVOID)
{
	HMODULE hDXGIDLL = nullptr;
	do
	{
		hDXGIDLL = GetModuleHandle("dxgi.dll");
		Sleep(100);
	}
	while (!hDXGIDLL);
	Sleep(100);

	CreateThread(nullptr, 0, UpdateThread, nullptr, 0, nullptr);

	IDXGISwapChain* pSwapChain;

	WNDCLASSEXA wc = {
		sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(nullptr), nullptr, nullptr, nullptr, nullptr,
		"DX", nullptr
	};
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", nullptr, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, nullptr, nullptr, wc.hInstance,
		nullptr);

	D3D_FEATURE_LEVEL requestedLevels[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1};
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = MultisampleCount;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;

	IDXGISwapChain* d3dSwapChain = nullptr;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		MessageBox(hWnd, "Failed to create directX device and swapchain!", "Error", MB_ICONERROR);
		return NULL;
	}


	pSwapChainVtable = (DWORD_PTR*)pSwapChain;
	pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

	pContextVTable = (DWORD_PTR*)pContext;
	pContextVTable = (DWORD_PTR*)pContextVTable[0];

	pDeviceVTable = (DWORD_PTR*)pDevice;
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	if (MH_Initialize() != MH_OK)
	{
		return 1;
	}
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hookD3D11Present,
	                  reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK)
	{
		return 1;
	}
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK)
	{
		return 1;
	}
	if (MH_CreateHook((DWORD_PTR*)pContextVTable[12], hookD3D11DrawIndexed,
	                  reinterpret_cast<void**>(&phookD3D11DrawIndexed)) != MH_OK)
	{
		return 1;
	}
	if (MH_EnableHook((DWORD_PTR*)pContextVTable[12]) != MH_OK)
	{
		return 1;
	}

	DWORD dwOld;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	while (true)
	{
		Sleep(10);
	}

	pDevice->Release();
	pContext->Release();
	pSwapChain->Release();

	return NULL;
}

//==========================================================================================================================

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(nullptr, 0, InitializeHook, nullptr, 0, nullptr);
		break;

	case DLL_PROCESS_DETACH:
		if (MH_Uninitialize() != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pContextVTable[12]) != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pContextVTable[13]) != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pContextVTable[8]) != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pContextVTable[7]) != MH_OK)
			return 1;
		if (MH_DisableHook((DWORD_PTR*)pContextVTable[10]) != MH_OK)
			return 1;
		break;
	}
	return TRUE;
}