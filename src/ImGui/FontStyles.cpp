#include "FontStyles.h"

#include "BSFont.h"
#include "Compatibility.h"
#include "Manager.h"
#include "Settings.h"

namespace ImGui
{
	void Font::LoadFontSettings(const CSimpleIniA& a_ini, const char* a_section)
	{
		name = a_ini.GetValue(a_section, "sFont", "");
		spacing = static_cast<float>(a_ini.GetDoubleValue(a_section, "fSpacing", 1.0));
	}

	void Font::LoadFont(ImFontConfig& config)
	{
		if (name.empty() || font) {
			return;
		}

		name = R"(Data\Interface\ImGuiIcons\Fonts\)" + name;

		const auto& io = ImGui::GetIO();
		config.GlyphExtraAdvanceX = spacing;
		font = io.Fonts->AddFontFromFileTTF(name.c_str(), 0.0f, &config);
	}

	void FontStyles::LoadFonts()
	{
		ImFontConfig config;
		auto&        io = ImGui::GetIO();

		if (UsingDefaultFont()) {
			logger::info("Using default font...");
			primaryFont.font = io.Fonts->AddFontFromMemoryCompressedTTF(BSFont_Data, BSFont_Size, 0.0f, &config);
		} else {
			logger::info("Using config font...");
			primaryFont.LoadFont(config);
		}
		config.MergeMode = true;
		secondaryFont.LoadFont(config);

		io.FontDefault = primaryFont.font;
	}

	bool FontStyles::UsingDefaultFont()
	{
		RE::BSResourceNiBinaryStream fontConfig("sFontConfigFile:Fonts"_ini.value());
		if (!fontConfig.good()) {
			return false;
		}

		constexpr std::string_view target = "$EverywhereMediumFont"sv;

		std::string line;

		while (std::getline(fontConfig, line)) {
			if (line.contains(target)) {
				auto eq = line.find('=');
				auto q1 = line.find('"', eq);
				auto q2 = line.find('"', q1 + 1);
				auto font = line.substr(q1 + 1, q2 - q1 - 1);
				if (font == "Futura Condensed") {
					return true;
				}
			}
		}

		return false;
	}

	std::optional<ImVec4> FontStyles::LoadColor(CSimpleIniA& a_ini, const char* a_section, const char* a_key)
	{
		std::string colorStr;
		ini::get_value(a_ini, colorStr, a_section, a_key);
		if (!colorStr.empty()) {
			return ToVar<ImVec4>(colorStr).first;
		}
		return std::nullopt;
	}

	void FontStyles::LoadStyleSettings(CSimpleIniA& a_ini)
	{
		auto get_value = [&]<typename T>(T& a_userValue, const T& a_defValue, const char* a_section, const char* a_key) {
			bool hex = false;
			std::tie(a_userValue, hex) = ToVar<std::remove_cvref_t<decltype(a_userValue)>>(a_ini.GetValue(a_section, a_key, ToString(a_defValue, true).c_str()));
			a_ini.SetValue(a_section, a_key, ToString(a_userValue, hex).c_str());
		};

		get_value(user.frameRounding, def.frameRounding, "Background", "fBgRounding");

		get_value(user.framePadding.x, def.framePadding.x, "Background", "fBgPaddingX");
		get_value(user.framePadding.y, def.framePadding.y, "Background", "fBgPaddingY");
	}

	void FontStyles::LoadFontStyles()
	{
		// load style
		Settings::GetSingleton()->Load(FileType::kStyles, [&](auto& a_ini) { LoadStyleSettings(a_ini); });

		auto& style = GetStyle();
		style.Colors[ImGuiCol_TextShadow] = { 0.0f, 0.0f, 0.0f, 1.0f };
		style.TextShadowOffset = { 2.0, 2.0 };
		style.FramePadding = user.framePadding;
		style.FrameRounding = user.frameRounding;

		// load fonts
		Settings::GetSingleton()->Load(FileType::kFonts, [&](auto& a_ini) {
			primaryFont.LoadFontSettings(a_ini, "PrimaryFont");
			secondaryFont.LoadFontSettings(a_ini, "SecondaryFont");
		});

		LoadFonts();
		style.ScaleAllSizes(ModAPIHandler::GetSingleton()->GetResolutionScale());

		Manager::GetSingleton()->UpdateVerticalSpacing();
	}

	void LoadColor(CSimpleIniA& a_ini, ImVec4& a_color, const char* a_section, const char* a_key)
	{
		if (auto loadedColor = ImGui::FontStyles::LoadColor(a_ini, a_section, a_key)) {
			a_color = *loadedColor;
		}
	}
}
