#include "Compatibility.h"
#include "Debug.h"
#include "Hooks.h"
#include "ImGui/Renderer.h"
#include "Manager.h"
#include "Papyrus.h"

void OnInit(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kPostLoad:
		{
			logger::info("{:*^50}", "POST LOAD");

			auto manager = Manager::GetSingleton();

			Settings::GetSingleton()->RegisterSchema(
				[](CSimpleIniA& a_ini, SyncMode a_mode) {
					Manager::GetSingleton()->SyncAllSettings(a_ini, a_mode);
				});

			ModAPIHandler::GetSingleton()->LoadModSettings();
			manager->LoadMCMSettings();
			manager->LoadIconTintOverrides();

			Hooks::Install();
		}
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		{
			logger::info("{:*^50}", "POST POST LOAD");
			ModAPIHandler::GetSingleton()->LoadAPIs();
		}
		break;
	case SKSE::MessagingInterface::kDataLoaded:
		{
			logger::info("{:*^50}", "DATA LOADED");

			const auto manager = Manager::GetSingleton();

			manager->OnDataLoad();
			ModAPIHandler::GetSingleton()->OnDataLoad();
			Debug::Install();
		}
		break;
	case SKSE::MessagingInterface::kNewGame:
	case SKSE::MessagingInterface::kPostLoadGame:
		{
			const bool newGame = a_msg->type == SKSE::MessagingInterface::kNewGame;
			logger::info("{:*^50}", newGame ? "NEW GAME" : "POST LOAD GAME");

			auto manager = Manager::GetSingleton();
			auto settings = Settings::GetSingleton();

			if (settings->SyncStylesToMCMIfNewer()) {
				manager->LoadMCMSettings(false);
			} else {
				settings->SyncMCMToStyles();
			}

			manager->ApplyScaledSettings();
			if (newGame) {
				CauseOfDeathManager::GetSingleton()->ClearMaps();
			}
		}
		break;
	default:
		break;
	}
}

#ifdef SKYRIM_AE
extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(Version::MAJOR);
	v.PluginName("KillFeed");
	v.AuthorName("powerofthree");
	v.UsesAddressLibrary();
	v.UsesUpdatedStructs();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });

	return v;
}();
#else
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "KillFeed";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		logger::critical("Unsupported runtime version {}", ver.string());
		return false;
	}

	return true;
}
#endif

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log", Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();

	logger::info("Game version : {}", a_skse->RuntimeVersion().string());

	SKSE::Init(a_skse, false);

	SKSE::AllocTrampoline(14 * 2);

	ImGui::Renderer::Install();

	const auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener("SKSE", OnInit);

	SKSE::GetPapyrusInterface()->Register(Papyrus::Register);

	return true;
}
