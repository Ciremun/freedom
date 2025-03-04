#include <windows.h>

#include "minhook/minhook.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>

#include "ui/ui.h"
#include "ui/config.h"
#include "ui/debug_log.h"

#include "lazer/scan.h"

typedef HRESULT(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

HWND g_hwnd = NULL;
HANDLE g_process = NULL;
HMODULE g_module = NULL;
LPVOID g_config_path = NULL;

bool init = false;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* pRenderTargetView = NULL;

Present oPresent;
Present pPresent;
ResizeBuffers oResizeBuffers;
ResizeBuffers pResizeBuffers;

static inline void imgui_new_frame()
{
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // process_hitobject();

    if (ImGui::IsKeyPressed(ImGuiKey_F7, false))
    {
        cfg_mod_menu_visible = !cfg_mod_menu_visible;
        ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
    }

    draw_debug_log();
    ImGui::SetMouseCursor(ImGui::GetIO().WantCaptureMouse ? ImGuiMouseCursor_Arrow : ImGuiMouseCursor_None);

    update_ui();
    if (!cfg_mod_menu_visible && !cfg_show_debug_log)
        ImGui::SetMouseCursor(ImGuiMouseCursor_None);

    ImGui::EndFrame();
    ImGui::Render();
}

bool get_present_pointer()
{
    HMODULE d3d11 = GetModuleHandle(TEXT("d3d11.dll"));
    if (d3d11 == 0) {
        FR_ERROR("GetModuleHandle d3d11.dll");
        return false;
    }

    void *D3D11CreateDeviceAndSwapChain = GetProcAddress(d3d11, "D3D11CreateDeviceAndSwapChain");
    if (!D3D11CreateDeviceAndSwapChain) {
        FR_ERROR("D3D11CreateDeviceAndSwapChain");
        return false;
    }

    WNDCLASSEX windowClass;
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = NULL;
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = TEXT("dx_window_cls");
    windowClass.hIconSm = NULL;
    RegisterClassEx(&windowClass);
    HWND window = CreateWindow(windowClass.lpszClassName, TEXT("dx_window"), WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

    DXGI_RATIONAL refreshRate;
    refreshRate.Numerator = 60;
    refreshRate.Denominator = 1;

    DXGI_MODE_DESC bufferDesc;
    bufferDesc.Width = 100;
    bufferDesc.Height = 100;
    bufferDesc.RefreshRate = refreshRate;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc = sampleDesc;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = window;
    swapChainDesc.Windowed = 1;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* context;

    if (((long(__stdcall*)(
        IDXGIAdapter*,
        D3D_DRIVER_TYPE,
        HMODULE,
        UINT,
        const D3D_FEATURE_LEVEL*,
        UINT,
        UINT,
        const DXGI_SWAP_CHAIN_DESC*,
        IDXGISwapChain**,
        ID3D11Device**,
        D3D_FEATURE_LEVEL*,
        ID3D11DeviceContext**))(D3D11CreateDeviceAndSwapChain))(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, &featureLevel, &context) < 0)
    {
        DestroyWindow(window);
        UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
        return false;
    }

    void **p_vtable = *reinterpret_cast<void***>(swapChain);
    pPresent = (Present)p_vtable[8];
    pResizeBuffers = (ResizeBuffers)p_vtable[13];

    swapChain->Release();
    device->Release();
    context->Release();

    DestroyWindow(window);
    UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
    return true;
}

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!init)
    {
        if (FAILED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
            return oPresent(pSwapChain, SyncInterval, Flags);
        pDevice->GetImmediateContext(&pContext);
        DXGI_SWAP_CHAIN_DESC sd;
        pSwapChain->GetDesc(&sd);
        g_hwnd = sd.OutputWindow;
        ID3D11Texture2D* pBackBuffer;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
        pBackBuffer->Release();
        oWndProc = (WNDPROC)SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
        g_process = GetCurrentProcess();
        init_ui(pDevice, pContext);
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init_hooks, 0, 0, 0));
        init = true;
    }
    ImGui_ImplDX11_NewFrame();
    imgui_new_frame();
    pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return oPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (pRenderTargetView)
    {
        pContext->OMSetRenderTargets(0, 0, 0);
        pRenderTargetView->Release();
        pRenderTargetView = nullptr;
    }

    HRESULT hr = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (SUCCEEDED(hr))
    {
        ID3D11Texture2D* pBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
        pDevice->CreateRenderTargetView(pBuffer, NULL, &pRenderTargetView);
        pBuffer->Release();
        pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<FLOAT>(Width);
        vp.Height = static_cast<FLOAT>(Height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        pContext->RSSetViewports(1, &vp);
    }

    return hr;
}

DWORD WINAPI freedom_main(HMODULE hModule)
{
    g_module = hModule;

#ifdef FR_LOG_TO_CONSOLE
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
#endif // FR_LOG_TO_CONSOLE

    if (!get_present_pointer()) {
        FR_ERROR("get_present_pointer");
        return 1;
    }

    MH_STATUS status = MH_Initialize();
    if (status != MH_OK) {
        FR_ERROR("MH_Initialize");
        return 1;
    }

    if (MH_CreateHook(reinterpret_cast<void**>(pPresent), &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) {
        FR_ERROR("MH_CreateHook pPresent");
        return 1;
    }

    if (MH_EnableHook(pPresent) != MH_OK) {
        FR_ERROR("MH_EnableHook pPresent");
        return 1;
    }

    if (MH_CreateHook(reinterpret_cast<void**>(pResizeBuffers), &hkResizeBuffers, reinterpret_cast<void**>(&oResizeBuffers)) != MH_OK) {
        FR_ERROR("MH_CreateHook pResizeBuffers");
        return 1;
    }

    if (MH_EnableHook(pResizeBuffers) != MH_OK) {
        FR_ERROR("MH_EnableHook pResizeBuffers");
        return 1;
    }

    // if (MH_DisableHook(MH_ALL_HOOKS) != MH_OK) {
    //     return 1;
    // }
    //
    // if (MH_Uninitialize() != MH_OK) {
    //     return 1;
    // }

    // ImGui_ImplDX11_Shutdown();
    // ImGui_ImplWin32_Shutdown();
    // ImGui::DestroyContext();

    // if (pRenderTargetView) { pRenderTargetView->Release(); pRenderTargetView = NULL; }
    // if (pContext) { pContext->Release(); pContext = NULL; }
    // if (pDevice) { pDevice->Release(); pDevice = NULL; }
    // SetWindowLongPtr(g_hwnd, GWLP_WNDPROC, (LONG_PTR)(oWndProc));

// #ifdef FR_LOG_TO_CONSOLE
//     FreeConsole();
// #endif // FR_LOG_TO_CONSOLE

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        g_config_path = lpReserved;
        CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)freedom_main, hModule, 0, 0));
    }
    return TRUE;
}
