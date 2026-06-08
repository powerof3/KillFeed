#include "Renderer.h"

#include "CauseOfDeath.h"
#include "FontStyles.h"
#include "Manager.h"

namespace ImGui::Renderer
{
	struct CreateD3DAndSwapChain
	{
		static void thunk()
		{
			func();

			if (const auto renderer = RE::BSGraphics::Renderer::GetSingleton()) {
				const auto swapChain = reinterpret_cast<IDXGISwapChain*>(renderer->data.renderWindows[0].swapChain);
				if (!swapChain) {
					logger::error("couldn't find swapChain");
					return;
				}

				DXGI_SWAP_CHAIN_DESC desc{};
				if (FAILED(swapChain->GetDesc(std::addressof(desc)))) {
					logger::error("IDXGISwapChain::GetDesc failed.");
					return;
				}

				const auto device = reinterpret_cast<ID3D11Device*>(renderer->data.forwarder);
				const auto context = reinterpret_cast<ID3D11DeviceContext*>(renderer->data.context);

				logger::info("Initializing ImGui..."sv);

				ImGui::CreateContext();

				auto& io = ImGui::GetIO();
				io.ConfigFlags = ImGuiConfigFlags_None;
				io.IniFilename = nullptr;

				if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
					logger::error("ImGui initialization failed (Win32)");
					return;
				}
				if (!ImGui_ImplDX11_Init(device, context)) {
					logger::error("ImGui initialization failed (DX11)"sv);
					return;
				}

				logger::info("ImGui initialized.");

				ImGui::FontStyles::GetSingleton()->LoadFontStyles();
				CauseOfDeathManager::GetSingleton()->LoadIcons();

				initialized.store(true);
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};

	void DrawKillFeed()
	{
		// Skip if Imgui is not loaded
		if (!initialized.load() || Manager::GetSingleton()->IsFeedEmpty()) {
			return;
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		{
			//trick imgui into rendering at game's real resolution (ie. if upscaled with Display Tweaks)
			static const auto [width, height] = RE::BSGraphics::Renderer::GetScreenSize();

			auto& io = ImGui::GetIO();
			io.DisplaySize.x = static_cast<float>(width);
			io.DisplaySize.y = static_cast<float>(height);
		}
		ImGui::NewFrame();
		{
			CauseOfDeathManager::GetSingleton()->ReloadIconsOnDemand();
			Manager::GetSingleton()->Draw();
		}
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	// IMenu::PostDisplay
	struct HUDMenu_PostDisplay
	{
		static void thunk(RE::HUDMenu* a_menu)
		{
			DrawKillFeed();

			func(a_menu);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx{ 0x6 };
	};

	struct JournalMenu_PostDisplay
	{
		static void thunk(RE::JournalMenu* a_menu)
		{
			DrawKillFeed();

			func(a_menu);
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx{ 0x6 };
	};

	void Install()
	{
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(75595, 77226), OFFSET(0x9, 0x275) };  // BSGraphics::InitD3D
		stl::write_thunk_call<CreateD3DAndSwapChain>(target.address());

		stl::write_vfunc<RE::HUDMenu, HUDMenu_PostDisplay>();
		stl::write_vfunc<RE::JournalMenu, JournalMenu_PostDisplay>();
	}
}
