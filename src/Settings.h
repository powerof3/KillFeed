#pragma once

#include "ImGui/FontStyles.h"

struct ScaledSetting
{
	ScaledSetting(float a_baseValue) :
		baseValue(a_baseValue)
	{}

	[[nodiscard]] float get() const { return value; }

	void load(CSimpleIniA& a_ini, const char* a_section, const char* a_key);
	void apply_scale(const char* a_key, float a_resolutionScale);

	// members
	float baseValue{};
	float value{ -1.234567f };
};

enum class FileType : std::uint8_t
{
	kFonts,
	kStyles,
	kMCM,
	kDisplayTweaks,
};

enum class SyncMode : std::uint8_t
{
	ReadFromStyles,
	WriteToStyles,
	PushToMCM
};

class Settings
{
public:
	using INIFunc = std::function<void(CSimpleIniA&)>;
	using SchemaFunc = void (*)(CSimpleIniA&, SyncMode);  // std::function isn't constexpr

	static Settings* GetSingleton()
	{
		return &instance;
	}

	void Load(FileType a_type, const INIFunc& a_func, bool a_generate = false) const;
	void Save(FileType a_type, const INIFunc& a_func) const;

	void RegisterSchema(SchemaFunc fn) { schema = std::move(fn); }

	void SyncMCMToStyles() const;
	bool SyncStylesToMCMIfNewer();

	template <class T>
	static void Visit(CSimpleIniA& a_ini, T& value, const char* a_section, const char* a_key, SyncMode a_mode);

	template <class T>
	static void PushMCMVar(const char* a_key, T& a_value);

private:
	static void SerializeINI(const wchar_t* a_path, const INIFunc& a_func, bool a_generate = false);
	static void SerializeINI(const wchar_t* a_defaultPath, const wchar_t* a_userPath, const INIFunc& a_func);

	bool IsStylesIniNewerThanMCM() const;

	// members
	SchemaFunc schema{ nullptr };

	const wchar_t* fontsPath{ L"Data/Interface/KillFeed/fonts.ini" };
	const wchar_t* stylePath{ L"Data/Interface/KillFeed/styles.ini" };
	const wchar_t* defaultMCMPath{ L"Data/MCM/Config/KillFeed/settings.ini" };
	const wchar_t* userMCMPath{ L"Data/MCM/Settings/KillFeed.ini" };
	const wchar_t* defaultDisplayTweaksPath{ L"Data/SKSE/Plugins/SSEDisplayTweaks.ini" };
	const wchar_t* userDisplayTweaksPath{ L"Data/SKSE/Plugins/SSEDisplayTweaks_Custom.ini" };

	static Settings instance;
};

inline constinit Settings Settings::instance;

template <class T>
void Settings::Visit(CSimpleIniA& a_ini, T& value, const char* a_section, const char* a_key, SyncMode a_mode)
{
	switch (a_mode) {
	case SyncMode::ReadFromStyles:
		{
			if constexpr (std::is_same_v<T, ImVec4>) {
				ImGui::LoadColor(a_ini, value, a_section, a_key);
			} else if constexpr (std::is_same_v<T, bool>) {  // MCM Helper doesn't read true/false
				value = a_ini.GetLongValue(a_section, a_key, static_cast<long>(value));
				a_ini.SetLongValue(a_section, a_key, static_cast<long>(value), nullptr);
			} else {
				ini::get_value(a_ini, value, a_section, a_key);
			}
		}
		break;
	case SyncMode::WriteToStyles:
		{
			if constexpr (std::is_same_v<T, bool>) {
				a_ini.SetLongValue(a_section, a_key, static_cast<long>(value));
			} else if constexpr (std::is_integral_v<T>) {
				a_ini.SetLongValue(a_section, a_key, value);
			} else if constexpr (std::is_floating_point_v<T>) {
				a_ini.SetDoubleValue(a_section, a_key, value);
			} else if constexpr (std::is_same_v<T, std::string>) {
				a_ini.SetValue(a_section, a_key, value.c_str());
			} else if constexpr (std::is_same_v<T, ImVec4>) {
				a_ini.SetValue(a_section, a_key, ImGui::FontStyles::ToString(value, true).c_str());
			}
		}
		break;
	case SyncMode::PushToMCM:
		{
			// update our settings when LoadMCMSettings is called, PushMCMVar doesn't update immediately
			if constexpr (std::is_same_v<T, bool>) {
				a_ini.SetLongValue(a_section, a_key, static_cast<long>(value));
			} else if constexpr (std::is_integral_v<T>) {
				a_ini.SetLongValue(a_section, a_key, value);
			} else if constexpr (std::is_floating_point_v<T>) {
				a_ini.SetDoubleValue(a_section, a_key, value);
			} else if constexpr (std::is_same_v<T, std::string>) {
				a_ini.SetValue(a_section, a_key, value.c_str());
			} else if constexpr (std::is_same_v<T, ImVec4>) {
				a_ini.SetValue(a_section, a_key, ImGui::FontStyles::ToString(value, true).c_str());
			}

			const auto name = std::format("{}:{}", a_key, a_section);
			if constexpr (std::is_same_v<T, ImVec4>) {
				auto colorStr = ImGui::FontStyles::ToString(value, true);
				PushMCMVar(name.c_str(), colorStr);
			} else {
				PushMCMVar(name.c_str(), value);
			}
		}
		break;
	default:
		std::unreachable();
	}
}

template <class T>
void Settings::PushMCMVar(const char* a_key, T& a_value)
{
	RE::BSFixedString mod{ "KillFeed" };
	RE::BSFixedString setting{ a_key };
	T                 movedValue = a_value;

	RE::BSFixedString fnName;
	if constexpr (std::is_same_v<T, bool>) {
		fnName = "SetModSettingBool";
	} else if constexpr (std::is_integral_v<T>) {
		fnName = "SetModSettingInt";
	} else if constexpr (std::is_floating_point_v<T>) {
		fnName = "SetModSettingFloat";
	} else if constexpr (std::is_same_v<T, std::string>) {
		fnName = "SetModSettingString";
	}

	RE::DispatchStaticCall("MCM", fnName, std::move(mod), std::move(setting), std::move(movedValue));
}
