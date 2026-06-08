#include "Debug.h"

#include "Manager.h"

namespace Debug
{
	struct SpawnNPCs
	{
		constexpr static auto OG_COMMAND = "ToggleSPUTransformUpdate"sv;

		constexpr static auto LONG_NAME = "SpawnNPC"sv;
		constexpr static auto SHORT_NAME = "SpawnNPC"sv;
		constexpr static auto HELP = ""sv;

		constexpr static RE::SCRIPT_PARAMETER SCRIPT_PARAMS = { "Integer", RE::SCRIPT_PARAM_TYPE::kInt, true };

		static bool Execute(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData* a_scriptData, RE::TESObjectREFR*, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
		{
			static std::vector<RE::TESNPC*> npcs;
			static std::vector<RE::TESNPC*> npcsInterior;

			if (npcs.empty() || npcsInterior.empty()) {
				npcs.clear();
				npcsInterior.clear();

				for (const auto& npc : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESNPC>()) {
					if (npc && !npc->IsPlayer() && npc->GetAggressionLevel() > RE::ACTOR_AGGRESSION::kUnaggressive && npc->GetConfidenceLevel() > RE::ACTOR_CONFIDENCE::kCowardly) {
						if (!npc->IsEssential() && !npc->IsProtected()) {
							npcs.emplace_back(npc);
							if (!npc->HasApplicableKeywordString("ActorTypeDragon")) {
								npcsInterior.emplace_back(npc);
							}
						}
					}
				}
			}

			const auto count = std::max(a_scriptData->GetIntegerChunk()->GetInteger(), 2);

			const auto player = RE::PlayerCharacter::GetSingleton();
			const auto inInterior = player->GetParentCell()->IsInteriorCell();

			const auto initialSeed = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());

			auto pick_npc = [&](std::size_t index) -> RE::TESNPC* {
				auto& vec = inInterior ? npcsInterior : npcs;

				std::uint64_t seed = initialSeed;
				boost::hash_combine(seed, static_cast<std::uint64_t>(index));

				auto npcIdx = clib_util::RNG(seed).generate<std::size_t>(0, npcs.size() - 1);
				auto npc = vec[npcIdx];

				vec[npcIdx] = vec.back();
				vec.pop_back();

				return npc;
			};

			const auto& playerPos = player->GetPosition();
			const float playerAngleZ = player->GetAngleZ();
			const float forwardX = std::sin(playerAngleZ);
			const float forwardY = std::cos(playerAngleZ);
			const float rightX = std::cos(playerAngleZ);
			const float rightY = -std::sin(playerAngleZ);

			constexpr float spawnDistance = 1024.0f;
			constexpr float spawnDistanceInterior = 256.0f;

			const float offset = (count - 1) * 0.5f;

			RE::NiPoint3 basePos = playerPos;
			basePos.x += forwardX * (inInterior ? spawnDistanceInterior : spawnDistance);
			basePos.y += forwardY * (inInterior ? spawnDistanceInterior : spawnDistance);

			for (std::int32_t i = 0; i < count; ++i) {
				constexpr float spacing = 128.0f;

				if (npcs.empty()) {
					break;
				}

				auto npc = pick_npc(i);
				if (!npc) {
					continue;
				}

				const auto& refPtr = player->PlaceObjectAtMe(npc, false).get();
				const auto  actor = refPtr ? refPtr->As<RE::Actor>() : nullptr;

				if (!actor) {
					continue;
				}

				float        t = static_cast<float>(i) - offset;
				RE::NiPoint3 pos = basePos;
				pos.x += spacing * t * rightX;
				pos.y += spacing * t * rightY;

				actor->SetPosition(pos, true);

				Manager::GetSingleton()->AddSpawnedDebugActor(refPtr);
			}
			return true;
		}
	};

	void Install()
	{
		logger::info("{:*^50}", "DEBUG");
		ConsoleCommandHandler<SpawnNPCs>::Install();
	}
}
