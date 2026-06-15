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

void ModAPIHandler::NND::GetAPI()
{
	logger::info("Retrieving NPCs Names Distributor API...");
	api = reinterpret_cast<NND_API::IVNND2*>(NND_API::RequestPluginAPI());
	if (api) {
		logger::info("\tNPCs Names Distributor API is up to date!");
	} else {
		logger::info("\tUnable to acquire NPCs Names Distributor API");
	}
}

std::string ModAPIHandler::NND::GetReferenceName(RE::TESObjectREFR* a_ref) const
{
	if (!a_ref) {
		return {};
	}

	std::string name;

	if (api) {
		if (auto actor = a_ref->As<RE::Actor>()) {
			if (auto nndName = api->GetName(actor, NND_API::NameContext::kEnemyHUD); !nndName.empty()) {
				name = { nndName.data(), nndName.size() };
			}
		}
	}

	if (name.empty()) {
		name = a_ref->GetDisplayFullName();
		if (name.empty()) {  // FEC blanks out display name for frozen corpses
			name = a_ref->GetName();
		}
	}

	return name;
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
	nnd.GetAPI();

	if (FUCK::Connect(fuckTool.PluginName())) {
		FUCK::RegisterTool(&fuckTool);
		logger::info("Kill Feed FUCK Menu registered");
	} else {
		logger::error("Failed to connect to FUCK API");
	}
}

void ModAPIHandler::OnDataLoad()
{
	nnd.usesEnglish = ("sLanguage:General"_ini == RE::FixedStrings::GetSingleton()->english);
}

float ModAPIHandler::GetResolutionScale() const
{
	return displayTweaks.GetResolutionScale();
}

std::string ModAPIHandler::GetReferenceName(const RE::TESObjectREFRPtr& a_ref, const RE::ActorPtr& a_commander) const
{
	if (a_commander) {
		return std::format("{}{}{}", nnd.GetReferenceName(a_commander.get()), nnd.usesEnglish ? "'s " : " - ", nnd.GetReferenceName(a_ref.get()));
	} else {
		return nnd.GetReferenceName(a_ref.get());
	}
}
