#include "Settings.h"

void ScaledSetting::load(CSimpleIniA& a_ini, const char* a_section, const char* a_key)
{
	ini::get_value(a_ini, value, a_section, a_key);
}

void ScaledSetting::apply_scale(const char* a_key, float a_resolutionScale)
{
	if (numeric::essentially_equal(value, -1.234567f)) {
		logger::info("Applying resolution scale {} to setting {} with base value {}", a_resolutionScale, a_key, baseValue);
		set(a_key, baseValue * a_resolutionScale);
	}
}

void ScaledSetting::set(const char* a_key, float a_newValue)
{
	value = a_newValue;
	Settings::PushMCMVar(a_key, value);
}

void Settings::SerializeINI(const wchar_t* a_path, const INIFunc& a_func, bool a_generate)
{
	CSimpleIniA ini;
	ini.SetUnicode();

	if (ini.LoadFile(a_path) >= SI_OK || a_generate) {
		a_func(ini);

		if (a_generate) {
			(void)ini.SaveFile(a_path);
		}
	}
}

void Settings::SerializeINI(const wchar_t* a_defaultPath, const wchar_t* a_userPath, const INIFunc& a_func)
{
	SerializeINI(a_defaultPath, a_func);
	SerializeINI(a_userPath, a_func);
}

void Settings::Load(const FileType a_type, const INIFunc& a_func, bool a_generate) const
{
	switch (a_type) {
	case FileType::kFonts:
		SerializeINI(fontsPath, a_func, a_generate);
		break;
	case FileType::kStyles:
		SerializeINI(stylePath, a_func, a_generate);
		break;
	case FileType::kMCM:
		SerializeINI(defaultMCMPath, userMCMPath, a_func);
		break;
	case FileType::kDisplayTweaks:
		SerializeINI(defaultDisplayTweaksPath, userDisplayTweaksPath, a_func);
		break;
	default:
		std::unreachable();
	}
}

void Settings::Save(FileType a_type, const INIFunc& a_func) const
{
	switch (a_type) {
	case FileType::kFonts:
		SerializeINI(fontsPath, a_func, true);
		break;
	case FileType::kStyles:
		SerializeINI(stylePath, a_func, true);
		break;
	case FileType::kMCM:
		SerializeINI(userMCMPath, a_func, true);
		break;
	case FileType::kDisplayTweaks:
		break;
	default:
		std::unreachable();
	}
}

bool Settings::IsStylesIniNewerThanMCM() const
{
	std::error_code ec;

	const auto stylesT = std::filesystem::last_write_time(stylePath, ec);
	if (ec) {
		return false;
	}

	const auto mcmT = std::filesystem::last_write_time(userMCMPath, ec);
	if (ec) {
		return true;
	}

	return stylesT > mcmT;
}

void Settings::SyncMCMToStyles() const
{
	Save(FileType::kStyles, [this](auto& a_ini) { schema(a_ini, SyncMode::WriteToStyles); });
}

bool Settings::SyncStylesToMCMIfNewer()
{
	if (!IsStylesIniNewerThanMCM()) {
		logger::info("MCM settings is newer than styles.ini. Syncing MCM to styles");
		return false;
	}

	logger::info("styles.ini is newer than MCM settings. Syncing styles to MCM");

	Load(FileType::kStyles, [this](auto& a_ini) {
		schema(a_ini, SyncMode::ReadFromStyles);
	});

	Save(FileType::kMCM, [this](auto& a_ini) {
		schema(a_ini, SyncMode::PushToMCM);
	});

	return true;
}

void Settings::SyncStylesAndMCM()
{
	logger::info("Syncing styles.ini and MCM settings");

	Save(FileType::kStyles, [this](auto& a_ini) {
		schema(a_ini, SyncMode::WriteToStyles);
	});

	Save(FileType::kMCM, [this](auto& a_ini) {
		schema(a_ini, SyncMode::PushToMCM);
	});
}
