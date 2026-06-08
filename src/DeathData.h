#pragma once

#include "CauseOfDeathEnum.h"
#include "ImGui/Graphics.h"

enum class CAUSE_OF_DEATH : std::uint8_t;
enum class SyncMode : std::uint8_t;

enum class EventType : std::uint8_t
{
	kKill,
	kReanimation
};

enum EventSource : std::uint8_t
{
	kPlayer,
	kFollower,
	kAlly,
	kHostile,
	kEnvironmental,  // only used for vis checks

	kTotal
};

struct Format
{
	enum class BackgroundFade
	{
		kNone = 0,
		kLeft = 1,
		kRight = 2,
		kBoth = 3,
	};

	void load_generic_settings(CSimpleIniA& a_ini);
	void sync_settings(CSimpleIniA& a_ini, SyncMode a_mode);
	void load_color_settings(CSimpleIniA& a_ini);
	void sync_color_settings(CSimpleIniA& a_ini, SyncMode a_mode);

	// members
	ImVec4                                     backgroundColor{ 0.0f, 0.0f, 0.0f, 0.68f };
	bool                                       showDistance{ true };
	bool                                       showDirection{ true };
	bool                                       showBackground{ true };
	REX::EnumSet<BackgroundFade, std::uint8_t> backgroundFade;
};

struct DeathData
{
	struct Combatant
	{
		Combatant() = default;
		Combatant(EventSource a_type);

		Combatant(EventSource a_type, std::string a_name);

		explicit operator bool() const { return !name.empty(); }

		const std::string& get_name() { return name; }
		void               set_name(const RE::TESObjectREFRPtr& a_ref, const RE::ActorPtr& a_commander = nullptr);
		void               update_color();

		bool Draw(ImDrawList* a_drawList, float a_posY, float a_entryH, float a_textH) const;

		// members
		EventSource type{};
		std::string name{};
		ImVec4      textColor{};
	};

	struct Icon
	{
		Icon() = default;
		explicit Icon(CAUSE_OF_DEATH a_cause);

		explicit operator bool() const { return texture != nullptr; }
		bool operator==(CAUSE_OF_DEATH a_cause) const { return cause == a_cause; }

		CAUSE_OF_DEATH get_cause() const { return cause; }
		float          get_height() const { return texture ? texture->size.y : 0.0f; }
		float          get_width() const { return texture ? texture->size.x : 0.0f; }
		void           update_tint();

		void Draw(float a_posY, float a_entryH) const;

		// members
		CAUSE_OF_DEATH            cause{};
		const ImGui::IconTexture* texture{ nullptr };
		ImVec4                    iconTint;
	};

	enum class Flags
	{
		kNone = 0,
		kOffscreen = 1 << 0,
		kDummyData = 1 << 1,
	};

	DeathData() = default;
	DeathData(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer, float a_distance, CAUSE_OF_DEATH a_primaryCause, std::optional<CAUSE_OF_DEATH> a_secondaryCause = std::nullopt, const RE::ActorPtr& a_victimCommander = nullptr);
	DeathData(EventSource a_victimSource, EventSource a_killerSource, CAUSE_OF_DEATH a_cause, std::optional<CAUSE_OF_DEATH> a_secondaryCause = std::nullopt);
	DeathData(const char* a_victimName, const char* a_killerName, CAUSE_OF_DEATH a_cause, std::optional<CAUSE_OF_DEATH> a_secondaryCause = std::nullopt);

	void Draw(const Format& a_format, bool a_needRealTimeUpdate);

	[[nodiscard]] bool is_dummy_data() const noexcept { return flags.any(Flags::kDummyData); }

	void               set_offscreen(bool a_isOffscreen) noexcept { flags.set(a_isOffscreen, Flags::kOffscreen); }
	[[nodiscard]] bool is_offscreen() const noexcept { return flags.any(Flags::kOffscreen); }

	// members
	Icon                              primaryCause{};
	std::optional<Icon>               secondaryCause{};
	REX::EnumSet<Flags, std::uint8_t> flags{ Flags::kNone };
	Combatant                         victim{};
	Combatant                         killer{};
	std::string                       distance{};
	std::array<float, 6>              compassAngle{};

private:
	DeathData(std::string a_victimName, std::string a_killerName, EventSource a_victimType, EventSource a_killerType, CAUSE_OF_DEATH a_primaryCause, std::optional<CAUSE_OF_DEATH> a_secondaryCause);

	bool drawIcons(float a_posY, float a_entryH) const;
	void drawDistance(ImDrawList* a_drawList, float a_posY, float a_entryH, float a_textH) const;
	void drawCompass(float a_posY, float a_textH, float a_entryH) const;
	void drawBackground(ImDrawList* a_drawList, const Format& a_format) const;

	static const char* get_event_source_name(EventSource a_source);
	void               cache_compass_angle(const RE::TESObjectREFRPtr& a_victim);

	static constexpr auto scalingFactor = 0.75f;
};
