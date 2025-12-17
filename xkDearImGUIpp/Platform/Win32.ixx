module;

#include <Windows.h>
#include <imgui_impl_win32.h>

export module xk.DearImGuipp.Platform.Win32;
import xk.DearImGuipp;

namespace xk::ImGuipp
{
	export class PlatformWin32 : public Platform
	{
	public:
		PlatformWin32(HWND handle)
		{
			ImGui_ImplWin32_Init(handle);
		}

		~PlatformWin32() override
		{
			ImGui_ImplWin32_Shutdown();
		}

	public:
		void NewFrame() override
		{
			ImGui_ImplWin32_NewFrame();
		}
	};
}

extern "C++" IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);