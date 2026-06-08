#pragma once

#include "CauseOfDeathEnum.h"
#include "DeathData.h"
#include "KillFeed.h"

class Manager :
	public REX::Singleton<Manager>,
	public RE::BSTEventSink<RE::TESDeathEvent>,
	public RE::BSTEventSink<RE::TESLoadGameEvent>,
	public RE::BSTEventSink<RE::InputEvent*>

{
public:
	enum class Visibility
	{
		kNone,
		kVisible = 1 << 0,
		kHidden = 1 << 1,
		kBoth = kVisible | kHidden
	};

	void OnDataLoad();
	void OnMCMOpen();
	void OnMCMClose();
	void LoadMCMSettings(bool a_afterDataLoad = false);

	void LoadMCMGenericSettings();
	void LoadMCMCategorySettings();
	void LoadMCMScaledSettings();

	void ApplyGenericSettings(bool a_oldDebugValue);
	void ApplyScaledSettings();

	void SyncAllSettings(CSimpleIniA& a_ini, SyncMode a_mode);

	void          LoadIconTintOverrides();
	const ImVec4& GetIconTint(CAUSE_OF_DEATH a_cause) const;
	const ImVec4& GetTextColor(EventSource a_source) const;

	void UpdateVerticalSpacing() const;

	static EventSource GetEventSourceType(const RE::TESObjectREFRPtr& a_ref);

	bool                   IsFeedEmpty() const { return killFeed.empty(); }
	std::pair<bool, bool>  ShouldDisplay(const RE::TESObjectREFRPtr& a_source, const RE::TESObjectREFRPtr& a_victim, EventType a_event, OptCause a_cause = std::nullopt) const;  // [should display, inView]
	std::pair<bool, float> IsWithinDistance(const RE::TESObjectREFRPtr& a_source) const;                                                                                         // [result, distance]

	void SetVisible(bool a_visible) { visible = a_visible; }
	void Draw();

	void PushData(DeathData a_data) { killFeed.push(std::move(a_data)); }
	void PushDummyData();
	void PushDemoData();

	void AddSpawnedDebugActor(const RE::TESObjectREFR* a_ref) { spawnedDebugActors.insert(a_ref->GetFormID()); }

private:
	struct CategorySettings
	{
		void load_mcm_settings(CSimpleIniA& a_ini, const char* a_section);
		void sync_settings(CSimpleIniA& a_ini, const char* a_section, SyncMode a_mode);

		std::pair<bool, bool> should_display(const RE::TESObjectREFRPtr& a_victim, EventType a_event) const;  // [should display, inView]

		const ImVec4& get_text_color() const noexcept { return textColor; }

	private:
		// members
		ImVec4                                 textColor{};
		REX::EnumSet<Visibility, std::uint8_t> visibilityKills{ Visibility::kBoth };
		REX::EnumSet<Visibility, std::uint8_t> visibilityReanimation{ Visibility::kBoth };
	};

	void LoadGenericSettings(CSimpleIniA& a_ini);
	void SyncGenericSettings(CSimpleIniA& a_ini, SyncMode a_mode);

	void LoadScaledSettings(CSimpleIniA& a_ini);
	void SyncScaledSettings(CSimpleIniA& a_ini, SyncMode a_mode);

	void LoadCategorySettings(CSimpleIniA& a_ini);
	void SyncCategorySettings(CSimpleIniA& a_ini, SyncMode a_mode);

	const CategorySettings& GetCategory(EventSource a_source) const noexcept { return categories[a_source]; }
	static EventSource      GetEventSourceType(const RE::TESObjectREFRPtr& a_source, const RE::TESObjectREFRPtr& a_victim, OptCause a_cause);

	RE::BSEventNotifyControl ProcessEvent(const RE::TESDeathEvent* a_event, RE::BSTEventSource<RE::TESDeathEvent>*) override;
	RE::BSEventNotifyControl ProcessEvent(const RE::TESLoadGameEvent* a_event, RE::BSTEventSource<RE::TESLoadGameEvent>*) override;
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>* a_eventSource) override;

	// members
	KillFeed                                          killFeed{};
	std::uint32_t                                     numEntries{ 4 };
	float                                             maxDistance{ 4096.0f };
	ScaledSetting                                     width{ 750.0f };
	ScaledSetting                                     height{ 300.0f };
	ScaledSetting                                     fontSize{ 28.5f };
	ScaledSetting                                     posX{ 50.0f };
	ScaledSetting                                     posY{ 525.0f };
	ImVec4                                            iconTint;
	ImVec4                                            textColor;
	float                                             verticalSpacingMult{ 0.75f };
	Format                                            format;
	std::array<CategorySettings, EventSource::kTotal> categories;
	bool                                              visible{ true };
	bool                                              enableDebug{ false };
	std::atomic<bool>                                 inMCM{ false };
	bool                                              enableIconTintOverride{ true };
	FlatMap<CAUSE_OF_DEATH, ImVec4>                   iconTintOverrides;
	FlatSet<RE::FormID>                               spawnedDebugActors{};

	static constexpr std::array<const char*, EventSource::kTotal> categoryNames{
		"Player",
		"Followers",
		"Allies",
		"Hostiles",
		"Environmental",
	};
};
