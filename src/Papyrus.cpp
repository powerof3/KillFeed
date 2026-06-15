#include "Papyrus.h"

#include "ImGui/FontStyles.h"
#include "Manager.h"

namespace Papyrus
{
	void OnMCMOpen(RE::TESQuest*)
	{
		Manager::GetSingleton()->OnMCMOpen();
	}

	void OnConfigClose(RE::TESQuest*)
	{
		Manager::GetSingleton()->OnMCMClose();
	}

	std::int32_t ColorStringToInt(RE::TESQuest*, std::string a_str)
	{
		return ImGui::FontStyles::ToVar<std::int32_t>(a_str).first;
	}

	void LoadMCMCategorySettings(RE::TESQuest*)
	{
		Manager::GetSingleton()->LoadMCMCategorySettings();
	}

	void LoadMCMScaledSettings(RE::TESQuest*)
	{
		Manager::GetSingleton()->LoadMCMScaledSettings();
	}

	void LoadMCMGenericSettings(RE::TESQuest*)
	{
		Manager::GetSingleton()->LoadMCMGenericSettings();
	}

	bool Register(RE::BSScript::IVirtualMachine* a_vm)
	{
		if (!a_vm) {
			return false;
		}

		a_vm->RegisterFunction("OnMCMOpen", MCM, OnMCMOpen);
		a_vm->RegisterFunction("OnConfigClose", MCM, OnConfigClose);
		a_vm->RegisterFunction("ColorStringToInt", MCM, ColorStringToInt);
		a_vm->RegisterFunction("LoadMCMCategorySettings", MCM, LoadMCMCategorySettings);
		a_vm->RegisterFunction("LoadMCMScaledSettings", MCM, LoadMCMScaledSettings);
		a_vm->RegisterFunction("LoadMCMGenericSettings", MCM, LoadMCMGenericSettings);

		logger::info("Registered {} class", MCM);

		return true;
	}
}
