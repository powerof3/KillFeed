#pragma once

#include "API/NND_API.h"

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

	void LoadModSettings();
	void LoadAPIs();

	void OnDataLoad();

	float       GetResolutionScale() const;
	std::string GetReferenceName(const RE::TESObjectREFRPtr& a_ref, const RE::ActorPtr& a_commander = nullptr) const;

	DisplayTweaks displayTweaks;
	NND           nnd;

	static ModAPIHandler instance;
};

inline constinit ModAPIHandler ModAPIHandler::instance;
