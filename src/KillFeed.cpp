#include "KillFeed.h"

void KillFeed::load_fuck_settings()
{
	if (FUCK::CollapsingHeader("$KF_Animation_Header"_T, ImGuiTreeNodeFlags_DefaultOpen)) {
		FUCK::Indent();

		if (FUCK::SliderFloat("$KF_FadeInTime_Text"_T, &fadeInTime, 0.f, 10.f, "%.2f")) {
			fadeOutStartTime = fadeInTime + lifeTime;
		}
		//FUCK::SetTooltip("$KF_FadeInTime_Help"_T);

		if (FUCK::SliderFloat("$KF_LifeTime_Text"_T, &lifeTime, 0.1f, 10.f, "%.2f")) {
			fadeOutStartTime = fadeInTime + lifeTime;
		}
		//FUCK::SetTooltip("$KF_LifeTime_Help"_T);

		if (FUCK::SliderFloat("$KF_FadeOutTime_Text"_T, &fadeOutTime, 0.f, 10.f, "%.2f")) {
			maxDuration = fadeOutStartTime + fadeOutTime;
		}
		//FUCK::SetTooltip("$KF_FadeOutTime_Help"_T);

		FUCK::SliderFloat("$KF_SlideOffsetX_Text"_T, &slideOffsetX.value, -50.f, 50.f, "%.2f");
		//FUCK::SetTooltip("$KF_SlideOffsetX_Help"_T);

		FUCK::SliderFloat("$KF_SlideOffsetY_Text"_T, &slideOffsetY.value, -50.f, 50.f, "%.2f");
		//FUCK::SetTooltip("$KF_SlideOffsetY_Help"_T);

		FUCK::Unindent();
	}
}

void KillFeed::load_mcm_settings(CSimpleIniA& a_ini)
{
	ini::get_value(a_ini, fadeInTime, "Animation", "fFadeInTime");
	ini::get_value(a_ini, lifeTime, "Animation", "fLifeTime");
	ini::get_value(a_ini, fadeOutTime, "Animation", "fFadeOutTime");

	load_scaled_settings(a_ini);

	fadeOutStartTime = fadeInTime + lifeTime;
	maxDuration = fadeOutStartTime + fadeOutTime;
}

void KillFeed::load_scaled_settings(CSimpleIniA& a_ini)
{
	slideOffsetX.load(a_ini, "Animation", "fSlideOffsetX");
	slideOffsetY.load(a_ini, "Animation", "fSlideOffsetY");
}

void KillFeed::apply_scaled_settings(float a_resolutionScale)
{
	slideOffsetX.apply_scale("fSlideOffsetX:Animation", a_resolutionScale);
	slideOffsetY.apply_scale("fSlideOffsetY:Animation", a_resolutionScale);
}

void KillFeed::sync_settings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, fadeInTime, "Animation", "fFadeInTime", a_mode);
	Settings::Visit(a_ini, lifeTime, "Animation", "fLifeTime", a_mode);
	Settings::Visit(a_ini, fadeOutTime, "Animation", "fFadeOutTime", a_mode);
	Settings::Visit(a_ini, slideOffsetX.value, "Animation", "fSlideOffsetX", a_mode);
	Settings::Visit(a_ini, slideOffsetY.value, "Animation", "fSlideOffsetY", a_mode);
}

bool KillFeed::empty() const
{
	return size() == 0;
}

std::uint32_t KillFeed::size() const
{
	return totalCount.load(std::memory_order_relaxed);
}

void KillFeed::push(DeathData&& a_data)
{
	totalCount.fetch_add(1, std::memory_order_relaxed);

	{
		Locker locker(lock);
		incoming.push_back(std::move(a_data));
		hasIncoming.store(true, std::memory_order_relaxed);
	}
}

void KillFeed::set_capacity(std::uint32_t a_newSize)
{
	newCapacity.store(a_newSize, std::memory_order_release);
}

void KillFeed::clear()
{
	clearRequested.fetch_or(kClear, std::memory_order_relaxed);
}

void KillFeed::clear_dummy_data()
{
	clearRequested.fetch_or(kClearDummyData, std::memory_order_relaxed);
}

void KillFeed::clear_impl()
{
	const auto flags = clearRequested.exchange(ClearFlag::kNone, std::memory_order_relaxed);
	if (flags == 0) {
		return;
	}

	Locker locker(lock);

	if ((flags & kClear) != 0) {
		incoming.clear();
		pending.clear();
		active.clear();
		totalCount.store(0, std::memory_order_relaxed);
		hasIncoming.store(false, std::memory_order_relaxed);

		return;
	}

	if ((flags & kClearDummyData) != 0) {
		std::erase_if(incoming, [](const DeathData& d) { return d.is_dummy_data(); });
		std::erase_if(pending, [](const DeathData& d) { return d.is_dummy_data(); });

		auto it = active.begin();
		while (it != active.end()) {
			if (it->data.is_dummy_data()) {
				it = active.erase(it);
			} else {
				++it;
			}
		}

		totalCount.store(
			static_cast<std::uint32_t>(active.size() + pending.size() + incoming.size()),
			std::memory_order_relaxed);
		if (incoming.empty()) {
			hasIncoming.store(false, std::memory_order_relaxed);
		}
	}
}
