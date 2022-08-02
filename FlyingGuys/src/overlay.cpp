#include "overlay.h"

bool ready = true;
extern bool value_changed;

extern float maxspeed_multiplier;
extern float gravity_multiplier;
extern bool no_collision;

int width;
int height;
bool k_leftclick = false;
bool k_ins = false;
bool show_menu = false;
visuals v;

ImFont* font1;
ImFont* font2;

extern bool IsKeyDown(int vk);

LONG nv_default = WS_POPUP | WS_CLIPSIBLINGS;
LONG nv_default_in_game = nv_default | WS_DISABLED;
LONG nv_edit = nv_default_in_game | WS_VISIBLE;

LONG nv_ex_default = WS_EX_TOOLWINDOW;
LONG nv_ex_edit = nv_ex_default | WS_EX_LAYERED | WS_EX_TRANSPARENT;
LONG nv_ex_edit_menu = nv_ex_default | WS_EX_TRANSPARENT;

static DWORD WINAPI StaticMessageStart(void* Param)
{
	Overlay* ov = (Overlay*)Param;
	ov->CreateOverlay();
	return 0;
}

// Data
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

void Overlay::RenderMenu()
{
	ImGui::SetNextWindowPos(ImVec2(400, 400));
	ImGui::SetNextWindowSize(ImVec2(490, 215));
	ImGui::GetStyle().WindowRounding = 7.0f;

	ImGui::Begin(XorStr("FlyingGuys"), (bool*)true, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

	ImGui::Text(XorStr("Max Speed Multiplier"));
	ImGui::SliderFloat(XorStr("##1"), &maxspeed_multiplier, 100.0f, 800.0f, "%.2f%%");

	ImGui::NewLine();

	ImGui::Text(XorStr("Gravity Multiplier"));
	ImGui::SliderFloat(XorStr("##2"), &gravity_multiplier, -500.0f, 500.0f, "%.2f%%");

	ImGui::NewLine();

	ImGui::Checkbox(XorStr("No Collision (EXPIRMENTAL)"), &no_collision);

	ImGui::NewLine();

	ImGui::Text(XorStr("Overlay FPS: %.3f ms/frame (%.1f FPS)"), 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::End();
}

void Overlay::RenderInfo()
{
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(1000, 250));

	ImGui::Begin(XorStr("##info"), (bool*)true, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::PushFont(font2);
	ImGui::TextColored(WHITE, "FlyingGuys / Max Speed Multiplier: %.2f%% / Gravity Multiplier: %.2f%%", maxspeed_multiplier, gravity_multiplier);
	ImGui::PopFont();

	ImGui::End();
}

void Overlay::ClickThrough(bool v)
{
	if (v)
	{
		nv_edit = nv_default_in_game | WS_VISIBLE;
		if (GetWindowLong(overlayHWND, GWL_EXSTYLE) != nv_ex_edit)
			SetWindowLong(overlayHWND, GWL_EXSTYLE, nv_ex_edit);
	}
	else
	{
		nv_edit = nv_default | WS_VISIBLE;
		if (GetWindowLong(overlayHWND, GWL_EXSTYLE) != nv_ex_edit_menu)
			SetWindowLong(overlayHWND, GWL_EXSTYLE, nv_ex_edit_menu);
	}
}

DWORD Overlay::CreateOverlay()
{
	const MARGINS margins = { -1 ,-1, -1, -1 };
	const wchar_t g_szClassName[] = L"overlay";

	WNDCLASSEX wc;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(RGB(0, 0, 0));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wc);

	overlayHWND = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT,
		g_szClassName,
		g_szClassName,
		WS_POPUP | WS_VISIBLE,
		0, 0, 1920, 1080,
		NULL, NULL, wc.hInstance, NULL);

	SetLayeredWindowAttributes(overlayHWND, RGB(0, 0, 0), 255, LWA_ALPHA);
	DwmExtendFrameIntoClientArea(overlayHWND, &margins);

	HDC hDC = ::GetWindowDC(NULL);
	width = ::GetDeviceCaps(hDC, HORZRES);
	height = ::GetDeviceCaps(hDC, VERTRES);

	running = true;

	// Initialize Direct3D
	if (!CreateDeviceD3D(overlayHWND))
	{
		CleanupDeviceD3D();
		return 1;
	}

	// Show the window
	::ShowWindow(overlayHWND, SW_SHOWDEFAULT);
	::UpdateWindow(overlayHWND);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowMinSize = ImVec2(1, 1);

	font1 = io.Fonts->AddFontDefault();
	font2 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\courbd.ttf", 18.0f);

	io.Fonts->Build();

	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(overlayHWND);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	ClickThrough(true);
	while (running)
	{
		HWND wnd = GetWindow(GetForegroundWindow(), GW_HWNDPREV);

		if (wnd != overlayHWND)
		{
			SetWindowPos(overlayHWND, wnd, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE);
			::UpdateWindow(overlayHWND);
		}

		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (IsKeyDown(VK_LBUTTON) && !k_leftclick)
		{
			io.MouseDown[0] = true;
			k_leftclick = true;
		}
		else if (!IsKeyDown(VK_LBUTTON) && k_leftclick)
		{
			io.MouseDown[0] = false;
			k_leftclick = false;
		}

		if (IsKeyDown(VK_INSERT) && !k_ins && ready)
		{
			show_menu = !show_menu;
			ClickThrough(!show_menu);
			k_ins = true;
		}
		else if (!IsKeyDown(VK_INSERT) && k_ins)
		{
			k_ins = false;
		}

		if (show_menu)
			RenderMenu();
		else
			RenderInfo();
		

		value_changed = ImGui::IsAnyItemActive();
		
		// Rendering
		ImGui::EndFrame();
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // Present with vsync

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	ClickThrough(true);

	CleanupDeviceD3D();
	::DestroyWindow(overlayHWND);
	return 0;
}

void Overlay::Start()
{
	DWORD ThreadID;
	CreateThread(NULL, 0, StaticMessageStart, (void*)this, 0, &ThreadID);
}

void Overlay::Clear()
{
	running = 0;
	Sleep(50);
}

int Overlay::getWidth()
{
	return width;
}

int Overlay::getHeight()
{
	return height;
}

// Helper functions

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (pBackBuffer)
	{
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
		pBackBuffer->Release();
	}
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void Overlay::DrawLine(ImVec2 a, ImVec2 b, ImColor color, float width)
{
	ImGui::GetWindowDrawList()->AddLine(a, b, color, width);
}

void Overlay::DrawBox(ImColor color, float x, float y, float w, float h)
{
	DrawLine(ImVec2(x, y), ImVec2(x + w, y), color, 1.0f);
	DrawLine(ImVec2(x, y), ImVec2(x, y + h), color, 1.0f);
	DrawLine(ImVec2(x + w, y), ImVec2(x + w, y + h), color, 1.0f);
	DrawLine(ImVec2(x, y + h), ImVec2(x + w, y + h), color, 1.0f);
}

void Overlay::Text(ImVec2 pos, ImColor color, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
	ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, color, text_begin, text_end, wrap_width, cpu_fine_clip_rect);
}

void Overlay::String(ImVec2 pos, ImColor color, const char* text)
{
	Text(pos, color, text, text + strlen(text), 200, 0);
}

void Overlay::RectFilled(float x0, float y0, float x1, float y1, ImColor color, float rounding, int rounding_corners_flags)
{
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
}

void Overlay::ProgressBar(float x, float y, float w, float h, int value, int v_max)
{
	ImColor barColor = ImColor(
		min(510 * (v_max - value) / 100, 255),
		min(510 * value / 100, 255),
		25,
		255
	);

	RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), barColor, 0.0f, 0);
}