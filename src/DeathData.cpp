#include "DeathData.h"

#include "CauseOfDeath.h"
#include "ImGui/FontStyles.h"
#include "ImGui/Graphics.h"
#include "ImGui/Util.h"
#include "Manager.h"

void Format::load_generic_settings(CSimpleIniA& a_ini)
{
	ini::get_value(a_ini, showDistance, "Entries", "bShowDistance");
	ini::get_value(a_ini, showDirection, "Entries", "bShowDirection");
	ini::get_value(a_ini, showBackground, "Entries", "bShowBg");

	auto bgFade = backgroundFade.underlying();
	ini::get_value(a_ini, bgFade, "Background", "iBgFade");
	backgroundFade = static_cast<BackgroundFade>(bgFade);
}

void Format::sync_settings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, showDistance, "Entries", "bShowDistance", a_mode);
	Settings::Visit(a_ini, showDirection, "Entries", "bShowDirection", a_mode);
	Settings::Visit(a_ini, showBackground, "Entries", "bShowBg", a_mode);

	auto bgFade = backgroundFade.underlying();
	Settings::Visit(a_ini, bgFade, "Background", "iBgFade", a_mode);
	backgroundFade = static_cast<BackgroundFade>(bgFade);
}

void Format::load_color_settings(CSimpleIniA& a_ini)
{
	ImGui::LoadColor(a_ini, backgroundColor, "Background", "sBgColor");
	ini::get_value(a_ini, backgroundColor.w, "Background", "fBgAlpha");
}

void Format::sync_color_settings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, backgroundColor, "Background", "sBgColor", a_mode);
	Settings::Visit(a_ini, backgroundColor.w, "Background", "fBgAlpha", a_mode);
}

DeathData::Combatant::Combatant(EventSource a_type) :
	type(a_type)
{
	update_color();
}

DeathData::Combatant::Combatant(EventSource a_type, std::string a_name) :
	type(a_type),
	name(std::move(a_name))
{
	update_color();
}

void DeathData::Combatant::set_name(const RE::TESObjectREFRPtr& a_ref)
{
	name = RE::GetNPCName(a_ref);
}

void DeathData::Combatant::update_color()
{
	textColor = Manager::GetSingleton()->GetTextColor(type);
}

bool DeathData::Combatant::Draw(ImDrawList* a_drawList, float a_posY, float a_entryH, float a_textH) const
{
	if (name.empty() || string::is_only_space(name)) {
		return false;
	}

	ImGui::PushStyleColor(ImGuiCol_Text, textColor);
	{
		ImGui::SetCursorPosY(a_posY + ((a_entryH - a_textH) * 0.5f));
		ImGui::TextWithBlurredShadow(a_drawList, ImGui::GetCursorScreenPos(), name.c_str());
	}
	ImGui::PopStyleColor();

	return true;
}

DeathData::Icon::Icon(CAUSE_OF_DEATH a_cause) :
	cause(a_cause),
	texture(CauseOfDeathManager::GetSingleton()->GetIcon(a_cause))
{}

void DeathData::Icon::update_tint()
{
	iconTint = Manager::GetSingleton()->GetIconTint(cause);
}

void DeathData::Icon::Draw(float a_posY, float a_entryH) const
{
	texture->ButtonIcon(a_posY, a_entryH, iconTint);
}

DeathData::DeathData(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer, float a_distance, CAUSE_OF_DEATH a_primaryCause, std::optional<CAUSE_OF_DEATH> a_secondaryCause) :
	primaryCause(a_primaryCause),
	killer(Manager::GetEventSourceType(a_killer)),
	victim(Manager::GetEventSourceType(a_victim)),
	distance(std::format("({})", std::truncf(a_distance)))
{
	if (primaryCause && a_secondaryCause) {
		secondaryCause.emplace(*a_secondaryCause);
	}

	if (a_killer) {
		killer.set_name(a_killer);
	}

	victim.set_name(a_victim);

	cache_compass_angle(a_victim);

	iconMode = get_icon_mode(Manager::GetSingleton()->UseSingleIcon());

	update_tints();
}

DeathData::DeathData(std::string a_victimName, std::string a_killerName, EventSource a_victimType, EventSource a_killerType, CAUSE_OF_DEATH a_primaryCause, std::optional<CAUSE_OF_DEATH> a_secondaryCause) :
	primaryCause(a_primaryCause),
	victim(a_victimType, std::move(a_victimName)),
	killer(a_killerType, std::move(a_killerName))
{
	if (primaryCause && a_secondaryCause) {
		secondaryCause.emplace(*a_secondaryCause);
	}

	flags.set(Flags::kDummyData);
}

DeathData::DeathData(EventSource a_victimSource, EventSource a_killerSource, CAUSE_OF_DEATH a_cause, std::optional<CAUSE_OF_DEATH> a_secondaryCause) :
	DeathData(
		get_event_source_name(a_victimSource),
		get_event_source_name(a_killerSource),
		a_victimSource,
		a_killerSource,
		a_cause,
		a_secondaryCause)
{}

DeathData::DeathData(const char* a_victimName, const char* a_killerName, CAUSE_OF_DEATH a_cause, std::optional<CAUSE_OF_DEATH> a_secondaryCause) :
	DeathData(
		a_victimName,
		a_killerName,
		EventSource::kHostile,
		EventSource::kPlayer,
		a_cause,
		a_secondaryCause)
{}

void DeathData::Draw(const Format& a_format, bool a_needRealTimeUpdate, bool a_simpleIcons)
{
	const float textH = ImGui::GetTextLineHeight();
	const float iconH = primaryCause.get_height();
	const float entryH = std::max(textH, iconH);

	const float posY = ImGui::GetCursorPosY();

	const auto drawList = ImGui::GetWindowDrawList();

	if (a_needRealTimeUpdate) {
		iconMode = get_icon_mode(a_simpleIcons);

		update_tints();

		victim.update_color();
		killer.update_color();
	}

	if (a_format.showBackground) {
		drawList->ChannelsSplit(2);
		drawList->ChannelsSetCurrent(1);
	}

	ImGui::BeginGroup();
	{
		if (killer.Draw(drawList, posY, entryH, textH)) {
			ImGui::SameLine();
		}

		if (drawIcons(posY, entryH)) {
			ImGui::SameLine();
		}

		victim.Draw(drawList, posY, entryH, textH);

		if (is_offscreen()) {
			if (a_format.showDistance) {
				ImGui::SameLine();
				drawDistance(drawList, posY, entryH, textH);
			}
			if (a_format.showDirection) {
				ImGui::SameLine();
				drawCompass(posY, textH, entryH);
			}
		}
	}
	ImGui::EndGroup();

	if (a_format.showBackground) {
		drawList->ChannelsSetCurrent(0);
		drawBackground(drawList, a_format);
		drawList->ChannelsMerge();
	}

	ImGui::SetCursorPosY(posY);
	const float totalH = entryH + ImGui::GetStyle().ItemSpacing.y;
	ImGui::Dummy(ImVec2(0.0f, totalH));
}

DeathData::IconMode DeathData::get_icon_mode(bool a_singleIcon) const
{
	if (!a_singleIcon || !secondaryCause || primaryCause == CAUSE_OF_DEATH::kShoutMouth) {
		return IconMode::kBoth;
	}

	return stl::any_of(primaryCause, CAUSE_OF_DEATH::kSpellCast, CAUSE_OF_DEATH::kScroll, CAUSE_OF_DEATH::kStaffMagic) ? IconMode::kSecondaryOnly : IconMode::kPrimaryOnly;
}

void DeathData::update_tints()
{
	switch (iconMode.get()) {
	case IconMode::kPrimaryOnly:
		primaryCause.iconTint = Manager::GetSingleton()->GetIconTint(secondaryCause->get_cause());
		break;
	case IconMode::kSecondaryOnly:
		secondaryCause->update_tint();
		break;
	case IconMode::kBoth:
		primaryCause.update_tint();
		if (secondaryCause) {
			secondaryCause->update_tint();
		}
		break;
	}
}

const char* DeathData::get_event_source_name(EventSource a_source)
{
	switch (a_source) {
	case EventSource::kPlayer:
		return "Player";
	case EventSource::kFollower:
		return "Follower";
	case EventSource::kAlly:
		return "Ally";
	case EventSource::kHostile:
		return "Hostile";
	case EventSource::kEnvironmental:
		return "Environmental";
	default:
		std::unreachable();
	}
}

void DeathData::cache_compass_angle(const RE::TESObjectREFRPtr& a_victim)
{
	auto angle = RE::GetHeadingAngleFromPlayer(a_victim);

	constexpr float WING_SWEEP = RE::NI_TWO_PI / 3;  // 120 degrees

	compassAngle[0] = std::sinf(angle);
	compassAngle[1] = std::cosf(angle);
	compassAngle[2] = std::sinf(angle - WING_SWEEP);
	compassAngle[3] = std::cosf(angle - WING_SWEEP);
	compassAngle[4] = std::sinf(angle + WING_SWEEP);
	compassAngle[5] = std::cosf(angle + WING_SWEEP);
}

bool DeathData::drawIcons(float a_posY, float a_entryH) const
{
	if (!primaryCause) {
		return false;
	}

	switch (iconMode.get()) {
	case IconMode::kPrimaryOnly:
		primaryCause.Draw(a_posY, a_entryH);
		break;
	case IconMode::kSecondaryOnly:
		secondaryCause->Draw(a_posY, a_entryH);
		break;
	case IconMode::kBoth:
		{
			primaryCause.Draw(a_posY, a_entryH);

			if (secondaryCause) {
				if (primaryCause == CAUSE_OF_DEATH::kShoutMouth) {
					const auto primaryW = primaryCause.get_width();
					const auto secondaryW = secondaryCause->get_width();

					ImGui::SameLine(0.0f, 0.0f);
					const auto primaryEndX = ImGui::GetCursorPosX();
					ImGui::SetCursorPosX(primaryEndX - primaryW + (primaryW - secondaryW) * 0.5f);

					secondaryCause->Draw(a_posY, a_entryH);

					ImGui::SameLine(0.0f, 0.0f);
					ImGui::SetCursorPosX(primaryEndX + std::max(0.0f, (secondaryW - primaryW) * 0.5f));
				} else {
					ImGui::SameLine(0.0f, 0.0f);
					secondaryCause->Draw(a_posY, a_entryH);
				}
			}
		}
		break;
	}
	return true;
}

void DeathData::drawDistance(ImDrawList* a_drawList, float a_posY, float a_entryH, float a_textH) const
{
	ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * scalingFactor);
	{
		ImGui::SetCursorPosY(a_posY + ((a_entryH - (a_textH * scalingFactor)) * 0.5f));
		ImGui::TextWithBlurredShadow(a_drawList, ImGui::GetCursorScreenPos(), distance.c_str());
	}
	ImGui::PopFont();
}

void DeathData::drawCompass(float a_posY, float a_textH, float a_entryH) const
{
	const auto compassSize = a_textH * scalingFactor;

	ImGui::SetCursorPosY(a_posY + a_entryH - compassSize);
	ImGui::Dummy({ compassSize, compassSize });

	const auto dummyPos = ImGui::GetItemRectMin();
	const auto dummyCenter = ImVec2(dummyPos.x + compassSize * 0.5f, dummyPos.y + compassSize * 0.5f);

	ImGui::DrawCompassArrow(dummyCenter, compassSize * 0.375f, compassAngle);
}

void DeathData::drawBackground(ImDrawList* a_drawList, const Format& a_format) const
{
	static const auto padding = ImGui::GetStyle().FramePadding;
	static const auto frameRounding = ImGui::GetStyle().FrameRounding;

	const auto rectMin = ImGui::GetItemRectMin() - padding;
	const auto rectMax = ImGui::GetItemRectMax() + padding;

	const auto bgColor = ImGui::GetColorU32(a_format.backgroundColor);

	const auto bgFade = ImGui::GetColorU32(ImVec4(
		a_format.backgroundColor.x,
		a_format.backgroundColor.y,
		a_format.backgroundColor.z,
		0.0f));

	switch (a_format.backgroundFade.get()) {
	case Format::BackgroundFade::kLeft:
		{
			const auto seamX = std::floor((rectMin.x + rectMax.x) * 0.5f);

			a_drawList->AddRectFilledMultiColor(rectMin, ImVec2(seamX, rectMax.y), bgFade, bgColor, bgColor, bgFade);
			a_drawList->AddRectFilled(ImVec2(seamX, rectMin.y), rectMax, bgColor, frameRounding, ImDrawFlags_RoundCornersRight);
		}
		break;
	case Format::BackgroundFade::kRight:
		{
			const auto seamX = std::floor((rectMin.x + rectMax.x) * 0.5f);

			a_drawList->AddRectFilled(rectMin, ImVec2(seamX, rectMax.y), bgColor, frameRounding, ImDrawFlags_RoundCornersLeft);
			a_drawList->AddRectFilledMultiColor(ImVec2(seamX, rectMin.y), rectMax, bgColor, bgFade, bgFade, bgColor);
		}
		break;
	case Format::BackgroundFade::kBoth:
		{
			const auto fadeW = (rectMax.x - rectMin.x) * 0.25f;
			const auto leftSeamX = std::floor(rectMin.x + fadeW);
			const auto rightSeamX = std::floor(rectMax.x - fadeW);

			a_drawList->AddRectFilledMultiColor(rectMin, ImVec2(leftSeamX, rectMax.y), bgFade, bgColor, bgColor, bgFade);
			a_drawList->AddRectFilled(ImVec2(leftSeamX, rectMin.y), ImVec2(rightSeamX, rectMax.y), bgColor);
			a_drawList->AddRectFilledMultiColor(ImVec2(rightSeamX, rectMin.y), rectMax, bgColor, bgFade, bgFade, bgColor);
		}
		break;
	default:
		a_drawList->AddRectFilled(rectMin, rectMax, bgColor, frameRounding);
		break;
	}
}
