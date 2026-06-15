#pragma once

#include "DeathData.h"
#include "Settings.h"

class KillFeed
{
public:
	void load_fuck_settings();
	void load_mcm_settings(CSimpleIniA& a_ini);
	void load_scaled_settings(CSimpleIniA& a_ini);
	void apply_scaled_settings(float a_resolutionScale);

	void sync_settings(CSimpleIniA& a_ini, SyncMode a_mode);

	bool          empty() const;
	std::uint32_t size() const;
	void          push(DeathData&& a_data);

	template <class... Args>
		requires std::constructible_from<DeathData, Args...>
	void push(Args&&... a_args)
	{
		push(DeathData(std::forward<Args>(a_args)...));
	}

	void set_capacity(std::uint32_t a_newSize);
	void clear();
	void clear_dummy_data();

	template <class F>
	void draw(F&& a_func)
	{
		clear_impl();

		if (const auto requestedCap = newCapacity.exchange(sentinelCapacity, std::memory_order_acquire); requestedCap != sentinelCapacity) {
			if (requestedCap >= active.capacity()) {
				active.set_capacity(requestedCap);
			} else {
				const auto oldSize = active.size();
				active.set_capacity(requestedCap);
				if (const auto droppedCount = static_cast<std::uint32_t>(oldSize - active.size()); droppedCount > 0) {
					totalCount.fetch_sub(droppedCount, std::memory_order_relaxed);
				}
			}
		}

		if (!RE::UI::GetSingleton()->GameIsPaused() && !RE::Main::GetSingleton()->freezeTime) {
			accumulatedTime += RE::BSTimer::GetSingleton()->realTimeDelta;
		}

		if (hasIncoming.load(std::memory_order_relaxed)) {
			std::vector<DeathData> local;
			{
				Locker locker(lock);
				local.swap(incoming);
				hasIncoming.store(false, std::memory_order_relaxed);
			}
			for (auto& item : local) {
				pending.push_back(std::move(item));
			}
		}

		while (active.size() < active.capacity() && !pending.empty()) {
			active.push_front({ std::move(pending.front()), accumulatedTime });
			pending.pop_front();
		}

		const float slideX = slideOffsetX.get();
		const float slideY = slideOffsetY.get();

		const bool shouldFadeIn = fadeInTime > 0.0f;
		const bool shouldFadeOut = fadeOutTime > 0.0f;

		for (auto& [data, startTime] : active) {
			float  opacity = 1.0f;
			ImVec2 offset{};

			const auto elapsedTime = accumulatedTime - startTime;

			if (shouldFadeIn && elapsedTime < fadeInTime && !data.is_dummy_data()) {
				const auto percent = std::clamp(static_cast<float>(elapsedTime / fadeInTime), 0.0f, 1.0f);
				opacity = percent;
				offset.x = -slideX * (1.0f - percent);
				offset.y = -slideY * (1.0f - percent);
			} else if (shouldFadeOut && elapsedTime > fadeOutStartTime) {
				const auto percent = std::clamp(static_cast<float>(elapsedTime - fadeOutStartTime) / fadeOutTime, 0.0f, 1.0f);
				opacity = 1.0f - percent;
				offset.x = slideX * percent;
				offset.y = slideY * percent;
			}

			a_func(data, opacity, offset);
		}

		while (!active.empty() && (accumulatedTime - active.back().startTime) > maxDuration) {
			active.pop_back();
			totalCount.fetch_sub(1, std::memory_order_relaxed);
		}
	}

private:
	static constexpr std::uint32_t sentinelCapacity = std::numeric_limits<std::uint32_t>::max();

	enum ClearFlag : std::uint8_t
	{
		kNone = 0,
		kClear = 1 << 0,
		kClearDummyData = 1 << 1,
	};

	using Lock = std::mutex;
	using Locker = std::scoped_lock<Lock>;

	struct Item
	{
		DeathData data;
		double    startTime{ 0.0 };
	};

	void clear_impl();

	// members
	float                        fadeInTime{ 0.3f };
	float                        lifeTime{ 3.0f };
	float                        fadeOutTime{ 0.3f };
	float                        fadeOutStartTime{};
	float                        maxDuration{};
	ScaledSetting                slideOffsetX{ 0.0f };
	ScaledSetting                slideOffsetY{ 20.0f };
	std::atomic<std::uint32_t>   totalCount{ 0 };
	double                       accumulatedTime{ 0.0 };
	mutable Lock                 lock;
	std::vector<DeathData>       incoming;
	std::atomic<bool>            hasIncoming{ false };
	std::atomic<std::uint8_t>    clearRequested{ ClearFlag::kNone };
	std::atomic<std::uint32_t>   newCapacity{ sentinelCapacity };
	std::deque<DeathData>        pending;
	boost::circular_buffer<Item> active;
};
