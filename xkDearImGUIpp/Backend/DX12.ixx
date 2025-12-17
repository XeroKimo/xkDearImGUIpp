module;

#include <imgui_impl_dx12.h>

export module xk.ImGuipp.Backend.DX12;
import xk.ImGuipp;

namespace xk::ImGuipp
{
	export class BackendDX12 : public Backend
	{
	public:
		BackendDX12(ImGui_ImplDX12_InitInfo info)
		{
			ImGui_ImplDX12_Init(&info);
		}

		~BackendDX12() override
		{
			ImGui_ImplDX12_Shutdown();
		}

	public:
		void NewFrame() override
		{
			ImGui_ImplDX12_NewFrame();
		}

		void Draw(ImDrawData* data, void* backendSpecific) override
		{
			ImGui_ImplDX12_RenderDrawData(data, static_cast<ID3D12GraphicsCommandList*>(backendSpecific));
		}
	};
}