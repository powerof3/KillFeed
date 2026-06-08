#pragma once

namespace ImGui
{
	struct Font
	{
		void LoadFontSettings(const CSimpleIniA& a_ini, const char* a_section);
		void LoadFont(ImFontConfig& config);

		std::string name{ "Jost-Medium.ttf" };
		float       spacing{ 1.0f };
		ImFont*     font;
	};

	struct Style
	{
		ImVec2 framePadding{ 4, 3 };
		float  frameRounding{ 0.0f };
	};

	class FontStyles : public REX::Singleton<FontStyles>
	{
	public:
		void LoadStyleSettings(CSimpleIniA& a_ini);
		void LoadFontStyles();
		void LoadFonts();

		static bool UsingDefaultFont();

		static std::optional<ImVec4> LoadColor(CSimpleIniA& a_ini, const char* a_section, const char* a_key);

		template <class T>
		static std::pair<T, bool> ToVar(const std::string& a_str);

		template <class T>
		static std::string ToString(const T& a_style, bool a_hex);

	private:
		// members
		Style def;
		Style user;

		Font primaryFont{};
		Font secondaryFont{};

		static FontStyles instance;
	};

	template <class T>
	std::pair<T, bool> FontStyles::ToVar(const std::string& a_str)
	{
		if constexpr (std::is_same_v<ImVec4, T> || std::is_same_v<std::int32_t, T>) {
			static srell::regex rgb_pattern_rgba("([0-9]+),([0-9]+),([0-9]+),([0-9]+)");
			static srell::regex rgb_pattern_rgb("([0-9]+),([0-9]+),([0-9]+)");
			static srell::regex hex_pattern_rgba("#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");
			static srell::regex hex_pattern_rgb("#([0-9a-fA-F]{2})([0-9a-fA-F]{2})([0-9a-fA-F]{2})");

			srell::smatch m;

			std::uint32_t r = 0;
			std::uint32_t g = 0;
			std::uint32_t b = 0;
			std::uint32_t a = 255;

			bool isHex = false;
			bool matched = false;

			if (srell::regex_match(a_str, m, rgb_pattern_rgba)) {
				r = string::to_num<std::uint32_t>(m[1]);
				g = string::to_num<std::uint32_t>(m[2]);
				b = string::to_num<std::uint32_t>(m[3]);
				a = string::to_num<std::uint32_t>(m[4]);
				matched = true;
			} else if (srell::regex_match(a_str, m, rgb_pattern_rgb)) {
				r = string::to_num<std::uint32_t>(m[1]);
				g = string::to_num<std::uint32_t>(m[2]);
				b = string::to_num<std::uint32_t>(m[3]);
				matched = true;
			} else if (srell::regex_match(a_str, m, hex_pattern_rgba)) {
				r = string::to_num<std::uint32_t>(m[1], true);
				g = string::to_num<std::uint32_t>(m[2], true);
				b = string::to_num<std::uint32_t>(m[3], true);
				a = string::to_num<std::uint32_t>(m[4], true);
				isHex = true;
				matched = true;
			} else if (srell::regex_match(a_str, m, hex_pattern_rgb)) {
				r = string::to_num<std::uint32_t>(m[1], true);
				g = string::to_num<std::uint32_t>(m[2], true);
				b = string::to_num<std::uint32_t>(m[3], true);
				isHex = true;
				matched = true;
			}

			if (!matched) {
				return { T(), false };
			}

			if constexpr (std::is_same_v<ImVec4, T>) {
				return { ImVec4{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f }, isHex };
			} else {
				const auto color = static_cast<std::int32_t>((r << 16) | (g << 8) | b);
				return { color, isHex };
			}
		} else {
			return { string::to_num<T>(a_str), false };
		}
	}

	template <class T>
	std::string FontStyles::ToString(const T& a_style, bool a_hex)
	{
		if constexpr (std::is_same_v<ImVec4, T>) {
			if (a_hex) {
				return std::format("#{:02X}{:02X}{:02X}{:02X}", static_cast<std::uint8_t>(255.0f * a_style.x), static_cast<std::uint8_t>(255.0f * a_style.y), static_cast<std::uint8_t>(255.0f * a_style.z), static_cast<std::uint8_t>(255.0f * a_style.w));
			}
			return std::format("{},{},{},{}", static_cast<std::uint8_t>(255.0f * a_style.x), static_cast<std::uint8_t>(255.0f * a_style.y), static_cast<std::uint8_t>(255.0f * a_style.z), static_cast<std::uint8_t>(255.0f * a_style.w));
		} else if constexpr (std::is_same_v<float, T>) {
			return std::format("{:.3f}", a_style);
		} else {
			return std::format("{}", a_style);
		}
	}

	void LoadColor(CSimpleIniA& a_ini, ImVec4& a_color, const char* a_section, const char* a_key);
}

template <>
struct glz::meta<ImVec4>
{
	static constexpr auto read = [](ImVec4& s, const std::string& input) { s = ImGui::FontStyles::ToVar<ImVec4>(input).first; };
	static constexpr auto write = [](auto& s) -> auto& { return ImGui::FontStyles::ToString(s, false); };
	static constexpr auto value = glz::custom<read, write>;
};
