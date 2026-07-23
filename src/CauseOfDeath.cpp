#include "CauseOfDeath.h"

#include "Settings.h"

void CauseOfDeathManager::LoadIcons()
{
	const auto renderer = RE::BSGraphics::Renderer::GetSingleton();
	if (!renderer) {
		return;
	}

	auto device = reinterpret_cast<ID3D11Device*>(renderer->data.forwarder);
	if (!device) {
		return;
	}

	const auto scale = iconScale / 1080;  // standard window height
	const auto height = RE::BSGraphics::Renderer::GetScreenSize().height;
	auto       textureScale = height * scale;

	std::for_each(std::execution::par, std::begin(iconTable), std::end(iconTable),
		[this, device, textureScale](const auto& entry) {
			const auto& [cause, name] = entry;
			if (!icons[static_cast<std::size_t>(cause)].Load(device, name, textureScale)) {
				logger::warn("Failed to load icon for cause: {}", static_cast<std::size_t>(cause));
			}
		});
}

void CauseOfDeathManager::ReloadIconsOnDemand()
{
	if (reloadIcons.exchange(false, std::memory_order_acquire)) {
		LoadIcons();
	}
}

void CauseOfDeathManager::LoadFuckSettings()
{
	auto oldIconScale = iconScale;
	if (FUCK::SliderFloat("$KF_IconScale_Text"_T, &iconScale, 0.01f, 1.f, "%.3f")) {
		if (!numeric::essentially_equal(iconScale, oldIconScale)) {
			reloadIcons.store(true, std::memory_order_release);
		}
	}
	//FUCK::SetTooltip("$KF_IconScale_Help"_T);
}

void CauseOfDeathManager::LoadMCMSettings(CSimpleIniA& a_ini)
{
	auto oldIconScale = iconScale;
	ini::get_value(a_ini, iconScale, "Icons", "fIconScale");
	if (!numeric::essentially_equal(iconScale, oldIconScale)) {
		reloadIcons.store(true, std::memory_order_release);
	}
}

void CauseOfDeathManager::SyncSettings(CSimpleIniA& a_ini, SyncMode a_mode)
{
	Settings::Visit(a_ini, iconScale, "Icons", "fIconScale", a_mode);
}

void CauseOfDeathManager::InsertIntoHitMap(const RE::HitData& a_hitData)
{
	hitSourceMap.insert_or_assign(
		a_hitData.target.get()->GetFormID(),
		HitSource(a_hitData.sourceRef.get() ? a_hitData.sourceRef.get() : a_hitData.aggressor.get(), a_hitData.weapon));
}

void CauseOfDeathManager::InsertIntoCommandedActorMap(RE::FormID a_summon, const RE::ActorHandle& a_commander)
{
	commandedActorMap.insert_or_assign(a_summon, a_commander);
}

void CauseOfDeathManager::Erase(RE::FormID a_formID)
{
	hitSourceMap.erase(a_formID);
	commandedActorMap.erase(a_formID);
}

void CauseOfDeathManager::ClearMaps()
{
	hitSourceMap.clear();
	commandedActorMap.clear();
}

const ImGui::IconTexture* CauseOfDeathManager::GetIcon(CAUSE_OF_DEATH a_cause) noexcept
{
	return std::addressof(icons[static_cast<std::size_t>(a_cause)]);
}

CauseOfDeathManager::HitSource CauseOfDeathManager::GetWeaponSource(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer)
{
	logger::debug("\tGetWeaponSource");

	HitSource hitSrc{};

	if (hitSourceMap.cvisit(a_victim->GetFormID(), [&](const auto& a_hitSrc) {
			hitSrc = a_hitSrc.second;
		})) {
		const auto owner_actor_is_killer = [&]() {
			if (!hitSrc.src) {
				return false;
			}
			if (auto srcActor = hitSrc.src->As<RE::Actor>()) {
				if (auto commandedActor = srcActor->GetCommandingActor()) {
					return commandedActor == a_killer;
				}
				if (srcActor->IsPlayerTeammate()) {
					return a_killer && a_killer->IsPlayerRef();	
				}
			} else {
				if (auto baseObj = hitSrc.src->GetBaseObject(); baseObj && baseObj->IsAmmo()) {
					auto actorCause = hitSrc.src->GetActorCause();
					return actorCause && actorCause->actor.get() == a_killer;
				}
			}
			return false;
		};

		if (hitSrc.weapon) {
			logger::debug("\t\tWeapon: {}", RE::FormLogger(hitSrc.weapon));
			logger::debug("\t\tSource: {}", RE::FormLogger(hitSrc.src.get()));

			if (!a_killer) {
				logger::debug("\t\t-> No killer specified, returning cached weapon");
			} else if (hitSrc.src == a_killer || owner_actor_is_killer()) {
				logger::debug("\t\t\t-> Source matches killer, returning cached weapon");
			} else {
				logger::debug("\t\t\t-> Source does not match killer ({}), ignoring cache", RE::FormLogger(a_killer.get()));
				hitSrc.weapon = nullptr;
			}
		} else {
			logger::debug("\t\tWeapon is NULL in hit source map");
			if (hitSrc.src) {
				logger::debug("\t\tSource found: (Base: {})", RE::FormLogger(hitSrc.src->GetBaseObject()));
			} else {
				logger::debug("\t\tSource is NULL in hit source map");
			}
		}
	} else {
		logger::debug("\t\tNo cached hit source found in hitSourceMap");
	}

	return hitSrc;
}

OptCause CauseOfDeathManager::GetCauseFromWeapon(const RE::TESObjectREFRPtr& a_killer, RE::TESObjectWEAP* a_weapon)
{
	auto weaponType = a_weapon->GetWeaponType();
	auto killerActor = a_killer ? a_killer->As<RE::Actor>() : nullptr;

	logger::debug("GetCauseFromWeapon: weapon={}, weaponType={}",
		RE::FormLogger(a_weapon), static_cast<std::uint32_t>(weaponType));

	OptCause cause{};

	a_weapon->ForEachKeyword([&](RE::BGSKeyword* a_keyword) {
		if (a_keyword) {
			if (auto edid = a_keyword->GetFormEditorID()) {
				if (auto it = weaponKeywordCause.find(edid); it != weaponKeywordCause.end()) {
					logger::debug("\t\t-> {} keyword found", edid);
					cause = it->second;
					return RE::BSContainer::ForEachResult::kStop;
				}
			}
		}
		return RE::BSContainer::ForEachResult::kContinue;
	});

	if (cause) {
		return cause;
	}

	const auto has_formid = [a_weapon](RE::FormID formID) { return a_weapon->GetFormID() == formID || a_weapon->templateWeapon && a_weapon->templateWeapon->GetFormID() == formID; };

	switch (weaponType) {
	case RE::WEAPON_TYPE::kHandToHandMelee:
		{
			if (a_weapon->impactDataSet && a_weapon->impactDataSet->GetFormID() == 0x189E8) {
				cause = CAUSE_OF_DEATH::kDart;
				logger::debug("\t\t-> Dart");
			} else {
				if (killerActor) {
					if (killerActor->HasKeywordString("ActorTypeNPC")) {
						logger::debug("\t\t\t> HandToHandMelee (ActorTypeNPC)");
						cause = CAUSE_OF_DEATH::kHandToHandMelee;
					} else {
						cause = GetCauseFromCreature(killerActor);
					}
				}
			}
		}
		break;
	case RE::WEAPON_TYPE::kOneHandSword:
		{
			if (has_formid(0x0007A91A) || string::iequals(a_weapon->GetModel(), R"(creationclub\SBJSSE001\weapons\Zulfiqar\Zulfiqar.nif)")) {
				logger::debug("\t\t-> Scimitar");
				cause = CAUSE_OF_DEATH::kScimitar;
			} else {
				logger::debug("\t\t-> OneHandSword");
				cause = CAUSE_OF_DEATH::kOneHandSword;
			}
		}
		break;
	case RE::WEAPON_TYPE::kOneHandDagger:
		cause = CAUSE_OF_DEATH::kOneHandDagger;
		logger::debug("\t\t-> OneHandDagger");
		break;
	case RE::WEAPON_TYPE::kOneHandAxe:
		cause = CAUSE_OF_DEATH::kOneHandAxe;
		logger::debug("\t\t-> OneHandAxe");
		break;
	case RE::WEAPON_TYPE::kOneHandMace:
		cause = CAUSE_OF_DEATH::kOneHandMace;
		logger::debug("\t\t-> OneHandMace");
		break;
	case RE::WEAPON_TYPE::kTwoHandSword:
		cause = GetCauseFromWeapon2HSword(killerActor);
		break;
	case RE::WEAPON_TYPE::kTwoHandAxe:
		logger::debug("\t\t-> TwoHandAxe");
		cause = CAUSE_OF_DEATH::kTwoHandAxe;
		break;
	case RE::WEAPON_TYPE::kBow:
		{
			if (killerActor && killerActor->HasKeywordString("ActorTypeDwarven")) {
				logger::debug("\t\t-> Crossbow");
				cause = CAUSE_OF_DEATH::kCrossbow;
			} else {
				logger::debug("\t\t-> Bow");
				cause = CAUSE_OF_DEATH::kBow;
			}
		}
		break;
	case RE::WEAPON_TYPE::kStaff:
		cause = CAUSE_OF_DEATH::kStaff;
		logger::debug("\t\t-> Staff");
		break;
	case RE::WEAPON_TYPE::kCrossbow:
		cause = CAUSE_OF_DEATH::kCrossbow;
		logger::debug("\t\t-> Crossbow");
		break;
	default:
		std::unreachable();
	}

	return cause;
}

CAUSE_OF_DEATH CauseOfDeathManager::GetCauseFromAmmo(RE::TESAmmo* a_ammo)
{
	OptCause cause{};

	a_ammo->ForEachKeyword([&](RE::BGSKeyword* a_keyword) {
		if (a_keyword) {
			if (auto edid = a_keyword->GetFormEditorID()) {
				if (auto it = ammoKeywordCause.find(edid); it != ammoKeywordCause.end()) {
					cause = it->second;
					return RE::BSContainer::ForEachResult::kStop;
				}
			}
		}
		return RE::BSContainer::ForEachResult::kContinue;
	});

	if (cause) {
		return *cause;
	}

	return a_ammo->IsBolt() ? CAUSE_OF_DEATH::kCrossbow : CAUSE_OF_DEATH::kBow;
}

CAUSE_OF_DEATH CauseOfDeathManager::GetCauseFromWeapon2HSword(RE::Actor* a_killer)
{
	if (a_killer) {
		if (a_killer->HasKeywordString("ActorTypeGiant")) {
			logger::debug("\t\t-> Club");
			return CAUSE_OF_DEATH::kClub;
		}
		if (a_killer->HasKeywordString("DLC2RieklingKeyword")) {
			logger::debug("\t\t-> Spear");
			return CAUSE_OF_DEATH::kSpear;
		}
	}

	logger::debug("\t\t-> TwoHandSword");
	return CAUSE_OF_DEATH::kTwoHandSword;
}

OptCause CauseOfDeathManager::GetCauseFromCreature(const RE::Actor* killer)
{
	OptCause cause{};

	if (auto killerRace = killer ? killer->GetRace() : nullptr) {
		logger::debug("\t\t\t\t\tKiller race: {}", RE::FormLogger(killerRace));

		if (auto voiceType = killerRace->defaultVoiceTypes[0]) {
			if (auto it = creatureCause.find(voiceType->GetFormEditorID()); it != creatureCause.end()) {
				cause = it->second;
			}
		}
	}

	return cause;
}

OptCausePair CauseOfDeathManager::GetCauseFromSource(RE::TESForm* a_src, const RE::TESObjectREFRPtr& a_killer, OptCause a_secondaryCause)
{
	if (auto weapon = a_src->As<RE::TESObjectWEAP>()) {
		return { GetCauseFromWeapon(a_killer, weapon), a_secondaryCause };
	} else if (auto ammo = a_src->As<RE::TESAmmo>()) {
		return { GetCauseFromAmmo(ammo), a_secondaryCause };
	} else {
		return { a_secondaryCause, std::nullopt };
	}
}

CauseOfDeathManager::MGEFSource CauseOfDeathManager::GetMagicSource(const RE::TESObjectREFRPtr& a_victim)
{
	logger::debug("\tGetMagicSource");

	std::optional<MGEFSource> bestSource;
	float                     bestMagnitude = -1.0f;

	using MGEF_FLAG = RE::EffectSetting::EffectSettingData::Flag;

	auto victimActor = a_victim->As<RE::Actor>();

	if (auto activeEffects = victimActor->GetActiveEffectList()) {
		logger::debug("\t\t{} active effects", activeEffects->size());

		std::uint32_t effectIndex = 0;

		for (const auto& activeEffect : *activeEffects) {
			if (activeEffect) {
				const auto spell = activeEffect->spell;
				const auto mgef = activeEffect->GetBaseObject();
				if (!spell || !mgef) {
					effectIndex++;
					continue;
				}

				if (mgef->data.flags.none(MGEF_FLAG::kDetrimental)) {
					logger::debug("\t\t\tEffect[{}] {} - Skipped (not detrimental)", effectIndex, RE::FormLogger(mgef));
					effectIndex++;
					continue;
				}

				if (spell->GetCastingType() == RE::MagicSystem::CastingType::kConstantEffect) {
					logger::debug("\t\t\tEffect[{}] {} - Skipped (casting type is constant)",
						effectIndex,
						RE::FormLogger(mgef));
					effectIndex++;
					continue;
				}

				auto caster = activeEffect->caster.get();

				logger::debug("\t\t\tEffect[{}]: mgef={}, spell={}, magnitude={:.2f}, caster={}, hostile={}, detrimental={}",
					effectIndex,
					RE::FormLogger(mgef),
					RE::FormLogger(spell),
					activeEffect->magnitude,
					RE::FormLogger(caster.get()),
					mgef->data.flags.any(MGEF_FLAG::kHostile),
					mgef->data.flags.any(MGEF_FLAG::kDetrimental));

				if (const auto mag = std::abs(activeEffect->magnitude); mag > bestMagnitude) {
					bestMagnitude = mag;
					bestSource = MGEFSource{ mgef, spell, caster, activeEffect->source };
				}
			}
			effectIndex++;
		}

		logger::debug("\t\tSummary: {} total effects", effectIndex);
	} else {
		logger::debug("\t\tVictim has no active effects");
	}

	if (bestSource) {
		logger::debug("\t\t-> Best source found: mgef={}, spell={}, magnitude={:.2f}",
			RE::FormLogger(bestSource->mgef),
			RE::FormLogger(bestSource->spell),
			bestMagnitude);
		return *bestSource;
	}

	logger::debug("\tGetMagicSource: No magic source found, returning empty");
	return {};
}

OptCausePair CauseOfDeathManager::GetCauseFromMGEF(const MGEFSource& a_magicItem, const RE::TESObjectREFRPtr& a_killer)
{
	OptCausePair cause{};

	const auto& [mgef, magicItem, caster, src] = a_magicItem;

	logger::debug("\tGetCauseFromMGEF: mgef={}, spell={}", RE::FormLogger(mgef), RE::FormLogger(magicItem));

	auto commandingOwner = caster ? caster->GetCommandingActor() : RE::ActorPtr{};

	if (commandingOwner && a_killer && commandingOwner != a_killer) {
		logger::debug("\t\t-> Commanding actor is not killer; {} != {}", RE::FormLogger(commandingOwner.get()), RE::FormLogger(a_killer.get()));
		return cause;
	}

	const auto avEffect = magicItem->GetAVEffect();
	const auto mgef_has = [&](auto&& fn) {
		return fn(mgef) || (avEffect && avEffect != mgef && fn(avEffect));
	};

	auto isDragon = caster && caster->HasKeywordString("ActorTypeDragon");
	auto isShout = !isDragon && mgef_has([](auto e) { return e->HasKeywordString("MagicDamageShout"); });

	auto spellType = magicItem->GetSpellType();
	auto isStaff = (spellType == RE::MagicSystem::SpellType::kStaffEnchantment);
	auto isWeapEnch = (spellType == RE::MagicSystem::SpellType::kEnchantment);
	auto isScroll = (spellType == RE::MagicSystem::SpellType::kScroll);

	auto isHandCast = false;
	if (auto spell = magicItem->As<RE::SpellItem>()) {
		if (auto slot = spell->GetEquipSlot()) {
			isHandCast = stl::any_of(slot->GetFormID(), 0x00013F42, 0x00013F43, 0x00013F44, 0x00013F45);
		}
	}

	const auto resolve_cast_source = [&] [[nodiscard]] (std::string_view a_type, CAUSE_OF_DEATH a_default, OptCause a_dragon, OptCause a_shout) {
		OptCausePair castSource{};
		logger::debug("\t\t-> {} ({})", a_type, isStaff ? "staff" : isScroll ? "scroll" :
																isWeapEnch   ? "weapon enchantment" :
																isDragon     ? "dragon" :
																isShout      ? "shout" :
																isHandCast   ? "spell cast" :
																			   "default");
		if (isStaff) {
			castSource.first = CAUSE_OF_DEATH::kStaffMagic;
			castSource.second = a_default;
		} else if (isScroll) {
			castSource.first = CAUSE_OF_DEATH::kScroll;
			castSource.second = a_default;
		} else if (isWeapEnch) {
			if (src) {
				castSource = GetCauseFromSource(src, a_killer, a_default);
			} else {
				castSource.first = a_default;
			}
		} else if (isDragon) {
			castSource.first = a_dragon ? *a_dragon : a_default;
		} else if (isShout) {
			castSource.first = CAUSE_OF_DEATH::kShoutMouth;
			castSource.second = a_shout ? *a_shout : a_default;
		} else if (isHandCast) {
			castSource.first = CAUSE_OF_DEATH::kSpellCast;
			castSource.second = a_default;
		} else {
			castSource.first = a_default;
		}
		return castSource;
	};

	if (auto element = MatchMagicType(mgef, avEffect); element == CAUSE_OF_DEATH::kFire) {
		cause = resolve_cast_source("MagicDamageFire",
			CAUSE_OF_DEATH::kFire,
			CAUSE_OF_DEATH::kFireDragon,
			CAUSE_OF_DEATH::kFireShout);
	} else if (element == CAUSE_OF_DEATH::kFrost) {
		cause = resolve_cast_source("MagicDamageFrost",
			CAUSE_OF_DEATH::kFrost,
			CAUSE_OF_DEATH::kFrostDragon,
			std::nullopt);
	} else if (element == CAUSE_OF_DEATH::kShock) {
		cause = resolve_cast_source("MagicDamageShock",
			CAUSE_OF_DEATH::kShock,
			CAUSE_OF_DEATH::kShockDragon,
			CAUSE_OF_DEATH::kShockShout);
	} else if (element == CAUSE_OF_DEATH::kSun) {
		cause = resolve_cast_source("MagicDamageSun",
			CAUSE_OF_DEATH::kSun,
			std::nullopt,
			std::nullopt);
	} else if (mgef_has([](auto e) { return e->GetFormID() == 0xC367A; })) {  // PerkBleedingDamage
		logger::debug("\t\t-> Bleeding");
		if (src) {
			cause = GetCauseFromSource(src, a_killer, CAUSE_OF_DEATH::kBleeding);
		} else {
			cause.first = CAUSE_OF_DEATH::kBleeding;
		}
	} else if (mgef_has([](auto e) { return e->GetFormID() == 0x201533C; })) {  // DLC1LD_LavaEffect
		logger::debug("\t\t-> Lava");
		cause.first = CAUSE_OF_DEATH::kLava;
	} else if (mgef_has([](auto e) { return e->HasArchetype(RE::EffectArchetype::kAbsorb) || stl::any_of(e->GetFormID(), 0x02008449, 0x02008448, 0x0200844A); })) {  //DLC1DragonDrainVitalityX
		cause = resolve_cast_source("Absorb",
			CAUSE_OF_DEATH::kDrain,
			CAUSE_OF_DEATH::kDrainDragon,
			CAUSE_OF_DEATH::kDrainShout);
	} else if (magicItem->IsPoison() || mgef_has([](auto e) { return e->data.resistVariable == RE::ActorValue::kPoisonResist; })) {
		logger::debug("\t\t-> Poison");
		if (src) {
			cause = GetCauseFromSource(src, a_killer, CAUSE_OF_DEATH::kPoison);
		} else if (mgef_has([&](auto x) { return x->data.projectileBase && stl::any_of(x->data.projectileBase->GetFormID(), 0x000A852A, 0x001090F8, 0x02013B80); })) {
			cause.first = CAUSE_OF_DEATH::kSprigganSpray;
		} else if (mgef_has([&](auto x) { return x->data.projectileBase && x->data.projectileBase->GetFormID() == 0x00059325; })) {  // DA13PoisonSprayProjectile
			cause.first = CAUSE_OF_DEATH::kShoutMouth;
			cause.second = CAUSE_OF_DEATH::kAfflictedSpray;
		} else {
			cause.first = CAUSE_OF_DEATH::kPoison;
		}
	} else if (mgef_has([](auto e) { return e->HasArchetype(RE::EffectArchetype::kBanish); })) {
		cause = resolve_cast_source("Banish",
			CAUSE_OF_DEATH::kBanished,
			std::nullopt,
			std::nullopt);
	} else if (mgef_has([](auto e) { return e->GetFormID() == 0x201533B || (e->data.effectShader && e->data.effectShader->GetFormID() == 0x0010FDF9); })) {  // DLC1LD_SteamEffect
		logger::debug("\t\t-> Steam");
		cause.first = CAUSE_OF_DEATH::kSteam;
	} else if (isShout) {
		logger::debug("\t\t-> Shout (generic)");
		cause.first = CAUSE_OF_DEATH::kShout;
	}

	if (!cause.first) {
		logger::debug("\t\t-> No matching magic damage type");
	}

	return cause;
}

OptCause CauseOfDeathManager::MatchMagicType(RE::EffectSetting* a_mgef, RE::EffectSetting* a_avEffect) const
{
	OptCause result;

	const auto match = [&](RE::EffectSetting* b_mgef) {
		if (!b_mgef || result) {
			return;
		}
		b_mgef->ForEachKeyword([&](RE::BGSKeyword* kw) {
			if (auto it = magicKeywordCause.find(kw->GetFormEditorID()); it != magicKeywordCause.end()) {
				result = it->second;
				return RE::BSContainer::ForEachResult::kStop;
			}
			return RE::BSContainer::ForEachResult::kContinue;
		});
	};

	match(a_mgef);
	if (a_mgef != a_avEffect) {
		match(a_avEffect);
	}

	return result;
}

OptCausePair CauseOfDeathManager::GetCauseFromEquipped(const RE::TESObjectREFRPtr& a_killer)
{
	OptCausePair cause{};

	auto killerActor = a_killer ? a_killer->As<RE::Actor>() : nullptr;
	if (!killerActor) {
		return cause;
	}

	for (std::uint32_t i = 0; i < 2; i++) {
		auto equipped = killerActor->GetEquippedObject(static_cast<bool>(i));
		if (!equipped) {
			continue;
		}

		logger::debug("\t\t{} hand: {}", i ? "Left" : "Right", RE::FormLogger(equipped));

		if (auto weapon = equipped->As<RE::TESObjectWEAP>()) {
			if (auto weapCause = GetCauseFromWeapon(a_killer, weapon)) {
				logger::debug("\t\t\t-> Weapon cause from {} hand: {}",
					i ? "left" : "right",
					static_cast<std::uint32_t>(*weapCause));
				cause.first = weapCause;
				return cause;
			}
		} else if (auto spell = equipped->As<RE::SpellItem>()) {
			if (auto avEffect = spell->GetAVEffect()) {
				using MGEF_FLAG = RE::EffectSetting::EffectSettingData::Flag;

				if (avEffect->data.flags.any(MGEF_FLAG::kHostile, MGEF_FLAG::kDetrimental)) {
					MGEFSource mgefSrc{ avEffect, spell, RE::ActorPtr{ killerActor } };
					cause = GetCauseFromMGEF(mgefSrc, a_killer);
					if (cause.first) {
						logger::debug("\t\t\t-> Magic cause from {} hand: {}", i ? "left" : "right", static_cast<std::uint32_t>(*cause.first));
						return cause;
					}
				}
			}
		}
	}

	logger::debug("\tGetCauseFromEquipped: No cause found");
	return cause;
}

CAUSE_OF_DEATH CauseOfDeathManager::GetCauseFromTrap(const RE::TESBoundObject* a_trapBase)
{
	switch (a_trapBase->GetFormID()) {
	case 0x6D912:     // TrapDweBallista
	case 0x020037E2:  // DLC1TrapCrossbow
		return CAUSE_OF_DEATH::kCrossbow;
	case 0x6C0F1:  // TrapDweThresher
		return CAUSE_OF_DEATH::kTrapSpinningBlades;
	case 0x5B8F4:  // TrapSpear01
	case 0x60960:  // TrapSpearDwemer01
	case 0x6095F:  // TrapSpearFalmer01
		return CAUSE_OF_DEATH::kSpear;
	case 0x1CDD0:  // TrapBladeSwinging01
	case 0xF8C9B:  // TrapBladeSwinging01DustFX
		return CAUSE_OF_DEATH::kTrapAxe;
	case 0x0402BBC2:  // DLC2TrapApoTentacle
	case 0x04035A63:  // DLC2TrapApoTentacleNoTrigger
		return CAUSE_OF_DEATH::kTentacle;
	case 0x2C15A:  // TrapBatteringRam01
	case 0x7A886:  // TrapSkullRam01
		return CAUSE_OF_DEATH::kTrapBatteringRam;
	case 0x1B3C0:  // TrapMace01
		return CAUSE_OF_DEATH::kTrapMace;
	case 0xC6440:     // TrapWallWood
	case 0x73265:     // TrapWoodSpikeBed01
	case 0x7325C:     // TrapNorPlatform01
	case 0x0403A9C0:  // DLC2TrapNorPlatformStairs
		return CAUSE_OF_DEATH::kTrapSpikes;
	case 0x70B9B:  // TrapNorFirePlate01
	case 0x750BD:  // TrapDweFlamePillar01
	case 0x7C26B:  // TrapNorFlamethrower
	case 0x864FC:  // TrapDweFlamethrower
	case 0x82E18:  // TrapExplosiveGas
	case 0x862CC:  // TrapOilPool01
	case 0x90EFC:  // TrapFallingOilLamp01
	case 0x7D901:  // TrapFallingOilLampExtender
		return CAUSE_OF_DEATH::kFire;
	case 0x79923:  // TrapFallingRockLg01
	case 0x74C8B:  // TrapFallingRockLg01_Rock
	case 0x887F3:  // TrapFallingRocksDust
	case 0x79922:  // TrapFallingRockSm01
	case 0xF39BF:  // TrapFallingRockSm01_CaveGBoulder
	case 0x74C8C:  // TrapFallingRockSm01_Rock
		return CAUSE_OF_DEATH::kTrapRocks;
	//case 0x90573:  // TrapWallFalmer
	//return CAUSE_OF_DEATH::kPincer;
	default:
		return CAUSE_OF_DEATH::kTrapGeneric;
	}
}

std::pair<DeathData, RE::TESObjectREFRPtr> CauseOfDeathManager::CreateDeathData(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer, float a_distance)
{
	RE::TESObjectREFRPtr actualKiller = a_killer;
	RE::ActorPtr         victimCommanderActor = nullptr;

	OptCause primaryCause;
	OptCause secondaryCause;

	logger::debug("=== CreateDeathData ===");
	logger::debug("Victim: {}", RE::FormLogger(a_victim.get()));
	logger::debug("Assumed killer: {}", RE::FormLogger(actualKiller.get()));

	if (!actualKiller && commandedActorMap.cvisit(a_victim->GetFormID(), [&](const auto& a_commandedActor) {
			victimCommanderActor = a_commandedActor.second.get();
		})) {
		primaryCause = CAUSE_OF_DEATH::kSummonExpired;
	} else if (auto [src, weapon] = GetWeaponSource(a_victim, actualKiller); weapon) {
		logger::debug("\tWeapon Source: {}", RE::FormLogger(weapon));

		primaryCause = GetCauseFromWeapon(src, weapon);
		if (actualKiller != src) {
			actualKiller = src;
		}
	} else if (!actualKiller && src) {
		actualKiller = src;
		if (const auto baseObj = src->GetBaseObject(); baseObj && baseObj->Is(RE::FormType::Activator)) {
			logger::debug("\t\tKiller is Activator -> Trap");
			primaryCause = GetCauseFromTrap(baseObj);
		}
	} else if (const auto mgef = GetMagicSource(a_victim); mgef.mgef && mgef.spell) {
		std::tie(primaryCause, secondaryCause) = GetCauseFromMGEF(mgef, actualKiller);
		if (primaryCause && mgef.caster != actualKiller) {
			actualKiller = mgef.caster;
		}
	}

	// check if src died by falling
	if (!primaryCause && !actualKiller) {
		logger::debug("\tNo killer or cause found, checking for fall damage");
		auto victimActor = a_victim->As<RE::Actor>();
		if (auto charController = victimActor ? victimActor->GetCharController() : nullptr) {
			if (charController->fallTime > 0.0f) {
				logger::info("\t\tFall time={}", charController->fallTime);
				primaryCause = CAUSE_OF_DEATH::kFalling;
			} else if (charController->fallStartHeight != 0.0f) {
				RE::hkVector4 pos;
				charController->GetPosition(pos, true);
				auto fallDistance = charController->fallStartHeight - (pos.quad.m128_f32[2] * 69.99125f);
				logger::info("\t\tFall distance: {}", fallDistance);
				if (fallDistance > 0.0f) {
					primaryCause = CAUSE_OF_DEATH::kFalling;
				}
			}
		}
	}

	// check equipped
	if (!primaryCause) {
		logger::debug("\tNo cause found. Checking killer's equipped items");
		std::tie(primaryCause, secondaryCause) = GetCauseFromEquipped(actualKiller);
	}

	// check creature if everything failed so far (no valid hit source)
	if (!primaryCause && actualKiller) {
		logger::debug("\tNo cause found. Checking killer creature type");
		primaryCause = GetCauseFromCreature(actualKiller->As<RE::Actor>());
	}

	if (!primaryCause) {  // generic skull icon
		primaryCause = CAUSE_OF_DEATH::kGeneric;
	}

	auto deathData = DeathData(a_victim, actualKiller, a_distance, *primaryCause, secondaryCause, victimCommanderActor);

	logger::debug("Primary Cause: {} ({})", static_cast<std::uint32_t>(*primaryCause), glz::get_enum_name(*primaryCause));
	logger::debug("Secondary Cause: {} ({})", secondaryCause ? static_cast<std::int32_t>(*secondaryCause) : -1, secondaryCause ? glz::get_enum_name(*secondaryCause) : "None");
	logger::debug("Victim Name: {}", deathData.victim.get_name());
	logger::debug("Killer Name: {}", deathData.killer.get_name());
	logger::debug("=== End CreateDeathData ===");

	return { deathData, actualKiller };
}
