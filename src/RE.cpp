#include "RE.h"

namespace RE
{
	float GetHeadingAngleFromPlayer(const TESObjectREFRPtr& a_victim)
	{
		auto player = PlayerCharacter::GetSingleton();
		return normalize_angle(std::atan2(a_victim->GetPositionX() - player->GetPositionX(), a_victim->GetPositionY() - player->GetPositionY()) - player->GetAngleZ());
	}

	std::string GetFocusedField(GFxMovieView* view)
	{
		GFxValue result;
		if (view->Invoke("Selection.getFocus", &result, nullptr, 0) && result.IsString()) {
			return result.GetString();
		}
		return {};
	}

	void SetActiveTextInputMaxChars()
	{
		static std::string path;

		auto ui = UI::GetSingleton();
		auto view = ui ? ui->GetMovieView(JournalMenu::MENU_NAME) : nullptr;
		if (!view) {
			return;
		}

		auto focusedPath = GetFocusedField(view.get());

		if (focusedPath == path) {
			return;
		}

		path = focusedPath;

		view->SetVariable((path + ".maxChars").c_str(), 260);

		// move cursor at end
		GFxValue textVal;
		if (view->GetVariable(&textVal, (path + ".text").c_str())) {
			const auto         len = std::strlen(textVal.GetString());
			const RE::GFxValue args[2]{ len, len };
			view->Invoke("Selection.setSelection", nullptr, args, 2);
		}
	}

	std::string GetNPCName(const TESObjectREFRPtr& a_actor)
	{
		if (const auto base = a_actor->GetBaseObject()) {
			BSString tmp;
			if (base->GetActivateText(a_actor.get(), tmp) && !tmp.empty()) {
				std::string text{ tmp };
				const auto  pos = text.find('\n');
				return pos != std::string_view::npos ?
				           text.substr(pos + 1) :
				           text;
			}
		}

		if (auto display = a_actor->GetDisplayFullName(); !string::is_empty(display)) {
			return display;
		}

		return a_actor->GetName();
	}
}
