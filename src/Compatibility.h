#pragma once

#include "API/FUCK_API.h"

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

	float GetResolutionScale() const;

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
	FUCKTool      fuckTool;

	static ModAPIHandler instance;
};

inline constinit ModAPIHandler ModAPIHandler::instance;
