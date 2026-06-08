#include "Manager.h"

#include "CauseOfDeath.h"
#include "Compatibility.h"
#include "ImGui/FontStyles.h"
#include "Settings.h"

void Manager::CategorySettings::load_mcm_settings(CSimpleIniA& a_ini, const char* a_section)
{
	ImGui::LoadColor(a_ini, textColor, a_section, "sTextColor");

	auto visValue = visibilityKills.underlying();
	ini::get_value(a_ini, visValue, a_section, "iVisibilityKills");
	visibilityKills = static_cast<Visibility>(visValue);

	visValue = visibilityReanimation.underlying();
	ini::get_value(a_ini, visValue, a_section, "iVisibilityReanimation");
	visibilityReanimation = static_cast<Visibility>(visValue);
}

void Manager::CategorySettings::sync_settings(CSimpleIniA& a_ini, const char* a_section, SyncMode a_mode)
{
	Settings::Visit(a_ini, textColor, a_section, "sTextColor", a_mode);
}

std::pair<bool, bool> Manager::CategorySettings::should_display(const RE::TESObjectREFRPtr& a_victim, EventType a_event) const
{
	const auto& visibility = (a_event == EventType::kKill) ? visibilityKills : visibilityReanimation;

	if (visibility == Visibility::kNone) {
		return { false, false };
	}

	bool inView = false;

	if (auto root = a_victim->Get3D()) {
		if (auto camera = RE::Main::WorldRootCamera()) {
			if (camera->PointInFrustum(root->worldBound.center, root->worldBound.radius)) {
				bool viewPick = false;
				inView = RE::PlayerCharacter::GetSingleton()->HasLineOfSight(a_victim.get(), viewPick);
			}
		}
	}

	if (visibility == Visibility::kBoth) {
		return { true, inView };
	}

	return { inView ? visibility.any(Visibility::kVisible) : visibility.any(Visibility::kHidden), inView };
}

void Manager::OnDataLoad()
{
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESDeathEvent>(this);
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink<RE::TESLoadGameEvent>(this);
	RE::BSInputDeviceManager::GetSingleton()->AddEventSink<RE::InputEvent*>(this);
}

void Manager::OnMCMOpen()
{
	inMCM.store(true, std::memory_order_relaxed);
	SetVisible(true);
	if (killFeed.size() < 2) {
		PushDummyData();
	}
}

void Manager::OnMCMClose()
{
	LoadMCMSettings(true);
	killFeed.clear_dummy_data();
	inMCM.store(false, std::memory_order_relaxed);

	Settings::GetSingleton()->SyncMCMToStyles();
}

void Manager::LoadGenericSettings(CSimpleIniA& a_ini)
{
	ini::get_value(a_ini, enableDebug, "KillFeed", "bDebug");
	ini::get_value(a_ini, maxDistance, "KillFeed", "fMaxDistance");
	ini::get_value(a_ini, numEntries, "Entries", "iNumEntries");
	ini::get_value(a_ini, verticalSpacingMult, "Entries", "fVerticalSpacing");
	ini::get_value(a_ini, enableIconTintOverride, "Icons", "bIconTintOverride");

	killFeed.load_mcm_settings(a_ini);
	format.load_generic_settings(a_ini);

	CauseOfDeathManager::GetSingleton()->LoadMCMSettings(a_ini);
}

void Manager::SyncGenericSettings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, numEntries, "Entries", "iNumEntries", a_mode);
	Settings::Visit(a_ini, verticalSpacingMult, "Entries", "fVerticalSpacing", a_mode);
	Settings::Visit(a_ini, enableIconTintOverride, "Icons", "bIconTintOverride", a_mode);

	killFeed.sync_settings(a_ini, a_mode);
	format.sync_settings(a_ini, a_mode);

	CauseOfDeathManager::GetSingleton()->SyncSettings(a_ini, a_mode);
}

void Manager::LoadScaledSettings(CSimpleIniA& a_ini)
{
	width.load(a_ini, "Window", "fWidth");
	height.load(a_ini, "Window", "fHeight");
	posX.load(a_ini, "Window", "fPosX");
	posY.load(a_ini, "Window", "fPosY");

	fontSize.load(a_ini, "Text", "fTextSize");
}

void Manager::SyncScaledSettings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, width.value, "Window", "fWidth", a_mode);
	Settings::Visit(a_ini, height.value, "Window", "fHeight", a_mode);
	Settings::Visit(a_ini, posX.value, "Window", "fPosX", a_mode);
	Settings::Visit(a_ini, posY.value, "Window", "fPosY", a_mode);

	Settings::Visit(a_ini, fontSize.value, "Text", "fTextSize", a_mode);
}

void Manager::LoadCategorySettings(CSimpleIniA& a_ini)
{
	for (std::size_t i = 0; i < categories.size(); ++i) {
		categories[i].load_mcm_settings(a_ini, categoryNames[i]);
	}

	ImGui::LoadColor(a_ini, textColor, "Text", "sTextColor");
	ImGui::LoadColor(a_ini, iconTint, "Icons", "sIconTint");

	format.load_color_settings(a_ini);
}

void Manager::SyncCategorySettings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	for (std::size_t i = 0; i < categories.size(); ++i) {
		categories[i].sync_settings(a_ini, categoryNames[i], a_mode);
	}

	Settings::Visit(a_ini, textColor, "Text", "sTextColor", a_mode);
	Settings::Visit(a_ini, iconTint, "Icons", "sIconTint", a_mode);

	format.sync_color_settings(a_ini, a_mode);
}

void Manager::UpdateVerticalSpacing() const
{
	if (!ImGui::GetCurrentContext()) {
		return;
	}

	// store original value
	static const float baseItemSpacingY = ImGui::GetStyle().ItemSpacing.y;
	ImGui::GetStyle().ItemSpacing.y = baseItemSpacingY * verticalSpacingMult;
}

void Manager::LoadMCMSettings(bool a_afterDataLoad)
{
	bool oldEnableDebug = enableDebug;

	Settings::GetSingleton()->Load(FileType::kMCM, [&](auto& a_ini) {
		LoadGenericSettings(a_ini);
		LoadCategorySettings(a_ini);
		LoadScaledSettings(a_ini);
	});

	ApplyGenericSettings(oldEnableDebug);

	if (a_afterDataLoad) {
		ApplyScaledSettings();
	}
}

void Manager::LoadMCMGenericSettings()
{
	bool oldEnableDebug = enableDebug;

	Settings::GetSingleton()->Load(FileType::kMCM, [&](auto& a_ini) {
		LoadGenericSettings(a_ini);
	});

	ApplyGenericSettings(oldEnableDebug);
}

void Manager::LoadMCMCategorySettings()
{
	Settings::GetSingleton()->Load(FileType::kMCM, [&](auto& a_ini) {
		LoadCategorySettings(a_ini);
	});
}

void Manager::LoadMCMScaledSettings()
{
	Settings::GetSingleton()->Load(FileType::kMCM, [&](auto& a_ini) {
		killFeed.load_scaled_settings(a_ini);
		LoadScaledSettings(a_ini);
	});

	ApplyScaledSettings();
}

void Manager::ApplyGenericSettings(bool a_oldDebugValue)
{
	killFeed.set_capacity(numEntries);

	if (a_oldDebugValue != enableDebug) {
		auto level = enableDebug ? spdlog::level::debug : spdlog::level::info;
		spdlog::set_level(level);
		spdlog::flush_on(level);
	}

	UpdateVerticalSpacing();
}

void Manager::ApplyScaledSettings()
{
	auto resolutionScale = ModAPIHandler::GetSingleton()->GetResolutionScale();

	width.apply_scale("fWidth:Window", resolutionScale);
	height.apply_scale("fHeight:Window", resolutionScale);
	posX.apply_scale("fPosX:Window", resolutionScale);
	posY.apply_scale("fPosY:Window", resolutionScale);

	fontSize.apply_scale("fTextSize:Text", resolutionScale);

	killFeed.apply_scaled_settings(resolutionScale);
}

void Manager::SyncAllSettings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	SyncGenericSettings(a_ini, a_mode);
	SyncCategorySettings(a_ini, a_mode);
	SyncScaledSettings(a_ini, a_mode);
}

void Manager::LoadIconTintOverrides()
{
	static std::filesystem::path jsonPath{ R"(Data\Interface\KillFeed\IconTintOverrides.json)" };

	std::error_code err;
	if (std::filesystem::exists(jsonPath, err)) {
		std::string buffer;
		auto        ec = glz::read_file_json(iconTintOverrides, jsonPath.string(), buffer);
		if (ec) {
			logger::info("Failed to read icon tint override file (error: {})", glz::format_error(ec, buffer));
		}
	} else {
		logger::info("Failed to load icon tint overrides file (error: {})", err.message());
	}

	for (auto& [cause, tint] : iconTintOverrides) {
		logger::info("Loaded icon tint override for cause {}: ({})", glz::get_enum_name(cause), ImGui::FontStyles::ToString(tint, true));
	}
}

std::pair<bool, bool> Manager::ShouldDisplay(const RE::TESObjectREFRPtr& a_source, const RE::TESObjectREFRPtr& a_victim, EventType a_event, OptCause a_cause) const
{
	auto sourceType = GetEventSourceType(a_source, a_victim, a_cause);
	return GetCategory(sourceType).should_display(a_victim, a_event);
}

std::pair<bool, float> Manager::IsWithinDistance(const RE::TESObjectREFRPtr& a_source) const
{
	auto distance = a_source->GetDistance(RE::PlayerCharacter::GetSingleton());
	return { maxDistance == 0.0f || distance <= maxDistance, distance };
}

void Manager::Draw()
{
	const auto windowPos = ImVec2(posX.get(), posY.get());
	const auto windowSize = ImVec2(width.get(), height.get());

	ImGui::SetNextWindowPos(windowPos, 0);
	ImGui::SetNextWindowSize(windowSize, 0);

	ImGui::PushFont(nullptr, fontSize.get());

	ImGui::Begin("##Main", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
	{
		// green screen
		/*ImGui::GetWindowDrawList()->AddRectFilled(
			windowPos,
			ImVec2(windowPos.x + windowSize.x, windowPos.y + windowSize.y),
			IM_COL32(0, 255, 0, 255),
			0.0f,
			0);*/

		killFeed.draw([this](DeathData& data, float opacity, const ImVec2& offset) {
			if (!visible) {  // queue should still run if game is paused
				return;
			}

			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);
			{
				auto finalOffset = data.primaryCause == CAUSE_OF_DEATH::kRaiseZombie ? -offset : offset;
				ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + finalOffset);

				data.Draw(format, inMCM.load(std::memory_order::relaxed));
			}
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();
		});
	}
	ImGui::End();

	ImGui::PopFont();
}

void Manager::PushDummyData()
{
	killFeed.push(EventSource::kPlayer, EventSource::kAlly, CAUSE_OF_DEATH::kOneHandSword, CAUSE_OF_DEATH::kFire);
	killFeed.push(EventSource::kFollower, EventSource::kHostile, CAUSE_OF_DEATH::kRaiseZombie);
}

void Manager::PushDemoData()
{
	killFeed.push("Bandit", "Mjoll the Lioness", CAUSE_OF_DEATH::kOneHandSword, CAUSE_OF_DEATH::kFrost);
	killFeed.push("Necromancer", "J'zargo", CAUSE_OF_DEATH::kSpellCast, CAUSE_OF_DEATH::kShock);
	killFeed.push("Skeleton", "Lydia", CAUSE_OF_DEATH::kOneHandMace, CAUSE_OF_DEATH::kFire);
}

EventSource Manager::GetEventSourceType(const RE::TESObjectREFRPtr& a_ref)
{
	auto actor = a_ref ? a_ref->As<RE::Actor>() : nullptr;
	if (!actor) {
		return EventSource::kEnvironmental;  // traps
	}

	if (actor->IsPlayerRef()) {
		return EventSource::kPlayer;
	}

	if (actor->IsPlayerTeammate()) {
		return EventSource::kFollower;
	}

	if (actor->IsHostileToActor(RE::PlayerCharacter::GetSingleton())) {
		return EventSource::kHostile;
	}

	return EventSource::kAlly;
}

EventSource Manager::GetEventSourceType(const RE::TESObjectREFRPtr& a_source, const RE::TESObjectREFRPtr& a_victim, OptCause a_cause)
{
	if (a_cause) {
		switch (*a_cause) {
		case CAUSE_OF_DEATH::kLava:
		case CAUSE_OF_DEATH::kTrapAxe:
		case CAUSE_OF_DEATH::kTrapSpinningBlades:
		case CAUSE_OF_DEATH::kTrapBatteringRam:
		case CAUSE_OF_DEATH::kTrapMace:
		case CAUSE_OF_DEATH::kTrapSpikes:
		case CAUSE_OF_DEATH::kTrapRocks:
		case CAUSE_OF_DEATH::kTrapGeneric:
			return EventSource::kEnvironmental;
		case CAUSE_OF_DEATH::kSummonExpired:
			{
				const auto commandingActor = a_victim->As<RE::Actor>()->GetCommandingActor();
				return GetEventSourceType(commandingActor ? commandingActor : a_victim);  // summons are hostile on death?
			}
		case CAUSE_OF_DEATH::kFalling:
		case CAUSE_OF_DEATH::kGeneric:
			return GetEventSourceType(a_victim);
		default:
			return GetEventSourceType(a_source);
		}
	}

	return GetEventSourceType(a_source);
}

const ImVec4& Manager::GetTextColor(EventSource a_source) const
{
	return GetCategory(a_source).get_text_color();
}

const ImVec4& Manager::GetIconTint(CAUSE_OF_DEATH a_cause) const
{
	if (enableIconTintOverride) {
		if (auto it = iconTintOverrides.find(a_cause); it != iconTintOverrides.end()) {
			return it->second;
		}
	}
	return iconTint;
}

RE::BSEventNotifyControl Manager::ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*)
{
	if (!a_event) {
		return RE::BSEventNotifyControl::kContinue;
	}

	const auto& victim = a_event->actorDying;
	if (!victim) {
		return RE::BSEventNotifyControl::kContinue;
	}

	if (!a_event->dead) {
		if (victim->Is3DLoaded()) {
			auto [withinDistance, distance] = IsWithinDistance(victim);
			if (!withinDistance) {
				return RE::BSEventNotifyControl::kContinue;
			}
			auto [data, actualKiller] = CauseOfDeathManager::GetSingleton()->CreateDeathData(victim, a_event->actorKiller, distance);
			if (auto [shouldDisplay, inView] = ShouldDisplay(actualKiller, victim, EventType::kKill, data.primaryCause.get_cause()); shouldDisplay) {
				data.set_offscreen(!inView);
				PushData(std::move(data));
			}
		}
	} else {
		CauseOfDeathManager::GetSingleton()->Erase(victim->GetFormID());
		if (victim->IsDynamicForm() && spawnedDebugActors.contains(victim->GetFormID())) {
			victim->Disable();
			victim->SetDelete(true);
			spawnedDebugActors.erase(victim->GetFormID());
		}
	}

	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Manager::ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>*)
{
	if (!a_event) {
		return RE::BSEventNotifyControl::kContinue;
	}

	if (!killFeed.empty()) {
		killFeed.clear();
	}

	CauseOfDeathManager::GetSingleton()->ClearMaps();
	spawnedDebugActors.clear();

	//PushDemoData();

	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Manager::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*)
{
	if (!a_event || !inMCM.load(std::memory_order::relaxed)) {
		return RE::BSEventNotifyControl::kContinue;
	}

	for (auto event = *a_event; event; event = event->next) {
		auto button = event->AsButtonEvent();
		if (button && stl::any_of(button->GetDevice(), RE::INPUT_DEVICE::kKeyboard, RE::INPUT_DEVICE::kMouse) && button->IsDown()) {
			RE::SetActiveTextInputMaxChars();
		}
	}

	return RE::BSEventNotifyControl::kContinue;
}
