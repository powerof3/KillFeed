#pragma once

#include "API/FUCK_API.h"
#include "API/NND_API.h"

enum EventSource : std::uint8_t;

class ModAPIHandler
{
public:
	static ModAPIHandler* GetSingleton()
	{
		return &instance;
	}

	struct DisplayTweaks
	{
		void  LoadSettings();
		float GetResolutionScale() const;

		float resolutionScale{ 1.0f };
		bool  borderlessUpscale{ false };
	};

	struct NND
	{
		void        GetAPI();
		std::string GetReferenceName(RE::TESObjectREFR* a_ref) const;

		NND_API::IVNND2* api{};
		bool             usesEnglish{ false };
	};

	struct FUCKTool : FUCK::ITool
	{
		const char*               PluginName() const override { return "KillFeed"; }
		[[nodiscard]] const char* Name() const override { return "$KF_KillFeed"_T; }
		void                      OnClose() override;
		void                      OnOpen() override;

		void Draw() override;
	};

	void LoadModSettings();
	void LoadAPIs();

	void OnDataLoad();

	float       GetResolutionScale() const;
	std::string GetReferenceName(const RE::TESObjectREFRPtr& a_ref, const RE::ActorPtr& a_commander = nullptr) const;

	template <std::size_t N>
	std::vector<const char*> Translated(const std::array<const char*, N>& a_keys)
	{
		std::vector<const char*> out;
		out.reserve(N);
		for (const auto* k : a_keys) {
			out.emplace_back(FUCK::Translate(k));
		}
		return out;
	}

	DisplayTweaks displayTweaks;
	NND           nnd;
	FUCKTool      fuckTool;

	static ModAPIHandler instance;
};

inline constinit ModAPIHandler ModAPIHandler::instance;
