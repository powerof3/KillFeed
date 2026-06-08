#pragma once

namespace Papyrus
{
	inline constexpr auto MCM = "KillFeed_MCM"sv;

	void         OnConfigClose(RE::TESQuest*);
	std::int32_t ColorStringToInt(RE::TESQuest*, std::string a_str);

	bool Register(RE::BSScript::IVirtualMachine* a_vm);
}
