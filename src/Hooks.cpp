#include "Hooks.h"

#include "Manager.h"

namespace Hooks
{
	namespace Actor
	{
		struct SendHitEvent
		{
#ifdef SKYRIM_AE
			static void thunk(RE::AIProcess* a_targetProcess, RE::HitData& a_hitData)
			{
				func(a_targetProcess, a_hitData);

				if (a_hitData.flags.any(RE::HitData::Flag::kFatal) && a_hitData.target.get()) {
					CauseOfDeathManager::GetSingleton()->InsertIntoHitMap(a_hitData);
				}
			}
#else
			static void thunk(RE::ScriptEventSourceHolder* a_holder,
				RE::NiPointer<RE::TESObjectREFR>&          a_target,
				RE::NiPointer<RE::TESObjectREFR>&          a_aggressor,
				RE::FormID                                 a_source,
				RE::FormID                                 a_projectile,
				RE::HitData&                               a_hitData)
			{
				func(a_holder, a_target, a_aggressor, a_source, a_projectile, a_hitData);

				if (a_hitData.flags.any(RE::HitData::Flag::kFatal) && a_hitData.target.get()) {
					CauseOfDeathManager::GetSingleton()->InsertIntoHitMap(a_hitData);
				}
			}
#endif
			static inline REL::Relocation<decltype(thunk)> func;
		};

		void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37633, 38586), OFFSET(0x16A, 0xFA) };
			stl::write_thunk_call<SendHitEvent>(target.address());
		}
	}

	/*void Install()
	{
		REL::Relocation<std::uintptr_t> hook1{ RELOCATION_ID(42638, 43806) };  // ConeProjectile::CollisionHitUpdate
		REL::Relocation<std::uintptr_t> hook2{ RELOCATION_ID(42943, 44123) };  // Projectile::ProcessImpacts
		REL::Relocation<std::uintptr_t> hook3{ RELOCATION_ID(43014, 44205) };  // Projectile::ProcessImpacts

		stl::write_thunk_call<SpawnCollisionEffects>(hook1.address() + OFFSET(0x479, 0x3A3));
		stl::write_thunk_call<SpawnCollisionEffects>(hook2.address() + OFFSET(0x410, 0x407));
		stl::write_thunk_call<SpawnCollisionEffects>(hook3.address() + OFFSET(0x1EA, 0x18A));

		REL::Relocation<std::uintptr_t> hook4{ RELOCATION_ID(43015, 44206) };  // Projectile::CastSpell
		stl::write_thunk_call<WouldSpellKill>(hook4.address() + 0x120);
	}*/

	struct ReanimateEffect__Start
	{
		static void thunk(RE::ReanimateEffect* a_this)
		{
			func(a_this);

			const auto& commandedActor = a_this->commandedActor.get();
			const auto& caster = a_this->GetCasterActor();

			if (commandedActor && caster) {
				const auto manager = Manager::GetSingleton();

				auto [withinDistance, distance] = Manager::GetSingleton()->IsWithinDistance(commandedActor);
				if (!withinDistance) {
					return;
				}

				auto [shouldDisplay, inView] = manager->ShouldDisplay(caster, commandedActor, EventType::kReanimation);

				if (!shouldDisplay) {
					return;
				}

				auto deathData = DeathData(commandedActor, caster, distance, CAUSE_OF_DEATH::kRaiseZombie);
				deathData.set_offscreen(!inView);

				manager->PushData(std::move(deathData));
			}
		}
		static inline REL::Relocation<decltype(thunk)> func;
		static inline constexpr std::size_t            idx{ 0x14 };

		static void Install()
		{
			stl::write_vfunc<RE::ReanimateEffect, ReanimateEffect__Start>();
		}
	};

	struct HUDMenu_ProcessMessage
	{
		static RE::UI_MESSAGE_RESULTS thunk(RE::HUDMenu* a_this, RE::UIMessage& a_message)
		{
			if (auto hudData = static_cast<RE::HUDData*>(a_message.data)) {
				if (hudData->type == RE::HUD_MESSAGE_TYPE::kSetMode) {
					static constexpr std::array badModes{
						"TweenMode"sv,
						"InventoryMode"sv,
						"WorldMapMode"sv,
						"BookMode"sv,
						"JournalMode"sv
					};
					if (std::ranges::any_of(badModes, [&](const auto& a_mode) { return string::iequals(hudData->text, a_mode); })) {
						Manager::GetSingleton()->SetVisible(!hudData->show);
					}
				}
			}

			return func(a_this, a_message);
		}

		static inline REL::Relocation<decltype(thunk)> func;
		static inline std::size_t                      idx = 0x4;
	};

	void Install()
	{
		Actor::Install();
		//Projectile::Install();

		ReanimateEffect__Start::Install();
		ActiveEffect::Finish<RE::ReanimateEffect>::Install();
		ActiveEffect::Finish<RE::SummonCreatureEffect>::Install();

		stl::write_vfunc<RE::HUDMenu, HUDMenu_ProcessMessage>();
	}
}
