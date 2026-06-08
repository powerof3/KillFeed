#include "Util.h"

#include "FontStyles.h"

namespace ImGui
{
	ImVec2 GetNativeViewportPos()
	{
		return GetMainViewport()->Pos;
	}

	ImVec2 GetNativeViewportSize()
	{
		return GetMainViewport()->Size;
	}

	ImVec2 GetNativeViewportCenter()
	{
		const auto Size = GetNativeViewportSize();
		return { Size.x * 0.5f, Size.y * 0.5f };
	}

	void DrawCompassArrow(const ImVec2& a_center, float a_size, const std::array<float, 6>& a_angles)
	{
		const auto drawList = ImGui::GetWindowDrawList();

		AddBlurredShadow(a_center, [&](const ImVec2& origin, ImU32 color) {
			const ImVec2 tip{ origin.x + a_angles[0] * a_size, origin.y - a_angles[1] * a_size };
			const ImVec2 left{ origin.x + a_angles[2] * a_size, origin.y - a_angles[3] * a_size };
			const ImVec2 right{ origin.x + a_angles[4] * a_size, origin.y - a_angles[5] * a_size };
			const ImVec2 notch{ origin.x - a_angles[0], origin.y + a_angles[1] };

			const std::array points{ tip, right, notch, left };
			drawList->AddConcavePolyFilled(points.data(), static_cast<int>(points.size()), color);
		});
	}

	void TextWithBlurredShadow(ImDrawList* a_drawlist, const ImVec2& a_pos, const char* a_text, const char* a_textEnd)
	{
		const auto font = ImGui::GetFont();
		const auto fontSize = ImGui::GetFontSize();

		AddBlurredShadow(a_pos, [&](const ImVec2& origin, ImU32 color) {
			a_drawlist->AddText(font, fontSize, origin, color, a_text, a_textEnd);
		});

		ImGui::Dummy(ImGui::CalcTextSize(a_text, a_textEnd));
	}
}
