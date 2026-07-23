#include "Compatibility.h"

#include "CauseOfDeath.h"
#include "DeathData.h"
#include "Manager.h"
#include "Settings.h"

float ModAPIHandler::DisplayTweaks::GetResolutionScale() const
{
	return borderlessUpscale ?
	           resolutionScale :
	           RE::BSGraphics::Renderer::GetScreenSize().height / 1080.0f;
}

void ModAPIHandler::DisplayTweaks::LoadSettings()
{
	Settings::GetSingleton()->Load(FileType::kDisplayTweaks, [this](auto& a_ini) {
		resolutionScale = static_cast<float>(a_ini.GetDoubleValue("Render", "ResolutionScale", resolutionScale));
		borderlessUpscale = a_ini.GetBoolValue("Render", "BorderlessUpscale", borderlessUpscale);
	});
}

void ModAPIHandler::FUCKTool::OnClose()
{
	Manager::GetSingleton()->OnFUCKMenuClose();
}

void ModAPIHandler::FUCKTool::OnOpen()
{
	Manager::GetSingleton()->OnFUCKMenuOpen();
}

void ModAPIHandler::FUCKTool::Draw()
{
	auto mgr = Manager::GetSingleton();

	if (FUCK::BeginTabBar("$KF_KillFeed"_T)) {
		if (FUCK::BeginTabItem("$KF_Notifications_Page"_T)) {
			mgr->DrawFUCKNotificationsPage();
			FUCK::EndTabItem();
		}
		if (FUCK::BeginTabItem("$KF_Appearance_Page"_T)) {
			mgr->DrawFUCKAppearancePage();
			FUCK::EndTabItem();
		}
		if (FUCK::BeginTabItem("$KF_Layout_Page"_T)) {
			mgr->DrawFUCKLayoutPage();
			FUCK::EndTabItem();
		}
		FUCK::EndTabBar();
	}
}

void ModAPIHandler::LoadModSettings()
{
	displayTweaks.LoadSettings();
}

void ModAPIHandler::LoadAPIs()
{
	if (FUCK::Connect("KillFeed")) {
		FUCK::RegisterTool(&fuckTool);
		logger::info("Kill Feed FUCK Menu registered");
	} else {
		logger::error("Failed to connect to FUCK API");
	}
}

float ModAPIHandler::GetResolutionScale() const
{
	return displayTweaks.GetResolutionScale();
}
