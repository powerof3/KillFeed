#pragma once

#include "CauseOfDeathEnum.h"
#include "DeathData.h"
#include "ImGui/Graphics.h"

enum class SyncMode : std::uint8_t;

class CauseOfDeathManager : public REX::Singleton<CauseOfDeathManager>
{
public:
	void LoadFuckSettings();
	void LoadMCMSettings(CSimpleIniA& a_ini);
	void SyncSettings(CSimpleIniA& a_ini, SyncMode a_mode);

	void LoadIcons();
	void ReloadIconsOnDemand();

	void InsertIntoHitMap(const RE::HitData& a_hitData);
	void InsertIntoCommandedActorMap(RE::FormID a_summon, const RE::ActorHandle& a_commander);
	void Erase(RE::FormID a_formID);
	void ClearMaps();

	const ImGui::IconTexture* GetIcon(CAUSE_OF_DEATH a_cause) noexcept;

	std::pair<DeathData, RE::TESObjectREFRPtr> CreateDeathData(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer, float a_distance);

private:
	struct HitSource
	{
		RE::TESObjectREFRPtr src{ nullptr };
		RE::TESObjectWEAP*   weapon{ nullptr };
	};

	struct MGEFSource
	{
		RE::EffectSetting*  mgef{ nullptr };
		RE::MagicItem*      spell{ nullptr };
		RE::ActorPtr        caster{ nullptr };
		RE::TESBoundObject* src{ nullptr };
	};

	HitSource GetWeaponSource(const RE::TESObjectREFRPtr& a_victim, const RE::TESObjectREFRPtr& a_killer);

	OptCause              GetCauseFromWeapon(const RE::TESObjectREFRPtr& a_killer, RE::TESObjectWEAP* a_weapon);
	static CAUSE_OF_DEATH GetCauseFromWeapon2HSword(RE::Actor* a_killer);
	CAUSE_OF_DEATH        GetCauseFromAmmo(RE::TESAmmo* a_ammo);
	OptCause              GetCauseFromCreature(const RE::Actor* killer);
	OptCausePair          GetCauseFromSource(RE::TESForm* a_source, const RE::TESObjectREFRPtr& a_killer, OptCause a_secondaryCause);

	MGEFSource   GetMagicSource(const RE::TESObjectREFRPtr& a_victim);
	OptCausePair GetCauseFromMGEF(const MGEFSource& a_magicItem, const RE::TESObjectREFRPtr& a_killer);
	OptCause     MatchMagicType(RE::EffectSetting* a_mgef, RE::EffectSetting* a_avEffect) const;

	OptCausePair GetCauseFromEquipped(const RE::TESObjectREFRPtr& a_killer);

	static CAUSE_OF_DEATH GetCauseFromTrap(const RE::TESBoundObject* a_trapBase);

	// members
	NodeMap<RE::FormID, HitSource>       hitSourceMap;
	NodeMap<RE::FormID, RE::ActorHandle> commandedActorMap;

	static constexpr std::array<std::pair<CAUSE_OF_DEATH, std::wstring_view>, static_cast<std::size_t>(CAUSE_OF_DEATH::kTotal)> iconTable{
		{ { CAUSE_OF_DEATH::kGeneric, ICON_PATH("generic") },
			{ CAUSE_OF_DEATH::kFire, ICON_PATH("fire") },
			{ CAUSE_OF_DEATH::kFrost, ICON_PATH("frost") },
			{ CAUSE_OF_DEATH::kShock, ICON_PATH("shock") },
			{ CAUSE_OF_DEATH::kSun, ICON_PATH("sun") },
			{ CAUSE_OF_DEATH::kBanished, ICON_PATH("banished") },
			{ CAUSE_OF_DEATH::kPoison, ICON_PATH("poison") },
			{ CAUSE_OF_DEATH::kSprigganSpray, ICON_PATH("spriggan-spray") },
			{ CAUSE_OF_DEATH::kAfflictedSpray, ICON_PATH("afflicted-spray") },
			{ CAUSE_OF_DEATH::kDrain, ICON_PATH("drain") },
			{ CAUSE_OF_DEATH::kHandToHandMelee, ICON_PATH("unarmed") },
			{ CAUSE_OF_DEATH::kOneHandSword, ICON_PATH("one-handed-sword") },
			{ CAUSE_OF_DEATH::kOneHandDagger, ICON_PATH("dagger") },
			{ CAUSE_OF_DEATH::kOneHandAxe, ICON_PATH("one-handed-axe") },
			{ CAUSE_OF_DEATH::kOneHandMace, ICON_PATH("one-handed-mace") },
			{ CAUSE_OF_DEATH::kTwoHandSword, ICON_PATH("two-handed-sword") },
			{ CAUSE_OF_DEATH::kTwoHandAxe, ICON_PATH("two-handed-axe") },
			{ CAUSE_OF_DEATH::kBow, ICON_PATH("bow") },
			{ CAUSE_OF_DEATH::kStaff, ICON_PATH("staff") },
			{ CAUSE_OF_DEATH::kStaffMagic, ICON_PATH("staff-magic") },
			{ CAUSE_OF_DEATH::kCrossbow, ICON_PATH("crossbow") },
			{ CAUSE_OF_DEATH::kClub, ICON_PATH("club") },
			{ CAUSE_OF_DEATH::kHalberd, ICON_PATH("halberd") },
			{ CAUSE_OF_DEATH::kKatana, ICON_PATH("katana") },
			{ CAUSE_OF_DEATH::kWhip, ICON_PATH("whip") },
			{ CAUSE_OF_DEATH::kScythe, ICON_PATH("scythe") },
			{ CAUSE_OF_DEATH::kSpear, ICON_PATH("spear") },
			{ CAUSE_OF_DEATH::kGun, ICON_PATH("gun") },
			{ CAUSE_OF_DEATH::kDart, ICON_PATH("dart") },
			{ CAUSE_OF_DEATH::kScimitar, ICON_PATH("scimitar") },
			{ CAUSE_OF_DEATH::kWarhammer, ICON_PATH("warhammer") },
			{ CAUSE_OF_DEATH::kWeapClaws, ICON_PATH("weap-claws") },
			{ CAUSE_OF_DEATH::kTrident, ICON_PATH("trident") },
			{ CAUSE_OF_DEATH::kGlaive, ICON_PATH("glaive") },
			{ CAUSE_OF_DEATH::kSickle, ICON_PATH("sickle") },
			{ CAUSE_OF_DEATH::kFishingRod, ICON_PATH("fishing-rod") },
			{ CAUSE_OF_DEATH::kSai, ICON_PATH("sai") },
			{ CAUSE_OF_DEATH::kSaber, ICON_PATH("saber") },
			{ CAUSE_OF_DEATH::kShuriken, ICON_PATH("shuriken") },
			{ CAUSE_OF_DEATH::kWarpick, ICON_PATH("war-pick") },
			{ CAUSE_OF_DEATH::kFlail, ICON_PATH("flail") },
			{ CAUSE_OF_DEATH::kKunai, ICON_PATH("kunai") },
			{ CAUSE_OF_DEATH::kChakram, ICON_PATH("chakram") },
			{ CAUSE_OF_DEATH::kLightsaber, ICON_PATH("lightsaber") },
			{ CAUSE_OF_DEATH::kBoomerang, ICON_PATH("boomerang") },
			{ CAUSE_OF_DEATH::kSlingshot, ICON_PATH("slingshot") },
			{ CAUSE_OF_DEATH::kCrescent, ICON_PATH("crescent") },
			{ CAUSE_OF_DEATH::kBomb, ICON_PATH("bomb") },
			{ CAUSE_OF_DEATH::kQuarterstaff, ICON_PATH("quarterstaff") },
			{ CAUSE_OF_DEATH::kScalpel, ICON_PATH("scalpel") },
			{ CAUSE_OF_DEATH::kScroll, ICON_PATH("scroll") },
			{ CAUSE_OF_DEATH::kSpellCast, ICON_PATH("spell-cast") },
			{ CAUSE_OF_DEATH::kPaw, ICON_PATH("paw") },
			{ CAUSE_OF_DEATH::kPincer, ICON_PATH("pincer") },
			{ CAUSE_OF_DEATH::kCrabPincer, ICON_PATH("crab-pincer") },
			{ CAUSE_OF_DEATH::kTalon, ICON_PATH("talon") },
			{ CAUSE_OF_DEATH::kHoof, ICON_PATH("hoof") },
			{ CAUSE_OF_DEATH::kBite, ICON_PATH("bite") },
			{ CAUSE_OF_DEATH::kSpiderBite, ICON_PATH("spider-bite") },
			{ CAUSE_OF_DEATH::kTusks, ICON_PATH("tusks") },
			{ CAUSE_OF_DEATH::kTusksBoar, ICON_PATH("boar-tusks") },
			{ CAUSE_OF_DEATH::kSkeletalHand, ICON_PATH("skeletal-hand") },
			{ CAUSE_OF_DEATH::kClaws, ICON_PATH("claws") },
			{ CAUSE_OF_DEATH::kTentacle, ICON_PATH("tentacle") },
			{ CAUSE_OF_DEATH::kIceSpear, ICON_PATH("ice-spear") },
			{ CAUSE_OF_DEATH::kFireDragon, ICON_PATH("dragon-fire") },
			{ CAUSE_OF_DEATH::kFrostDragon, ICON_PATH("dragon-frost") },
			{ CAUSE_OF_DEATH::kShockDragon, ICON_PATH("dragon-shock") },
			{ CAUSE_OF_DEATH::kDrainDragon, ICON_PATH("dragon-drain") },
			{ CAUSE_OF_DEATH::kFireShout, ICON_PATH("shout-fire") },
			{ CAUSE_OF_DEATH::kShockShout, ICON_PATH("shout-shock") },
			{ CAUSE_OF_DEATH::kDrainShout, ICON_PATH("shout-drain") },
			{ CAUSE_OF_DEATH::kShout, ICON_PATH("shout-generic") },
			{ CAUSE_OF_DEATH::kShoutMouth, ICON_PATH("shout-mouth") },
			{ CAUSE_OF_DEATH::kBleeding, ICON_PATH("bleeding") },
			{ CAUSE_OF_DEATH::kTrapAxe, ICON_PATH("trap-axe") },
			{ CAUSE_OF_DEATH::kTrapSpinningBlades, ICON_PATH("trap-spinning-blades") },
			{ CAUSE_OF_DEATH::kTrapBatteringRam, ICON_PATH("trap-battering-ram") },
			{ CAUSE_OF_DEATH::kTrapMace, ICON_PATH("trap-mace") },
			{ CAUSE_OF_DEATH::kTrapSpikes, ICON_PATH("trap-spikes") },
			{ CAUSE_OF_DEATH::kTrapRocks, ICON_PATH("trap-rocks") },
			{ CAUSE_OF_DEATH::kTrapGeneric, ICON_PATH("trap-generic") },
			{ CAUSE_OF_DEATH::kFalling, ICON_PATH("falling") },
			{ CAUSE_OF_DEATH::kLava, ICON_PATH("lava") },
			{ CAUSE_OF_DEATH::kSteam, ICON_PATH("steam") },
			{ CAUSE_OF_DEATH::kSummonExpired, ICON_PATH("expired") },
			{ CAUSE_OF_DEATH::kRaiseZombie, ICON_PATH("raise-zombie") } },
	};
	static_assert(iconTable.size() == static_cast<std::size_t>(CAUSE_OF_DEATH::kTotal));

	std::array<ImGui::IconTexture, static_cast<std::size_t>(CAUSE_OF_DEATH::kTotal)> icons{};

	FlatMap<std::string_view, CAUSE_OF_DEATH> creatureCause{
		{ "CrAtronachFlameVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrAtronachFrostVoice", CAUSE_OF_DEATH::kIceSpear },
		{ "CrAtronachStormVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrBearVoice", CAUSE_OF_DEATH::kPaw },
		{ "CrChaurusVoice", CAUSE_OF_DEATH::kPincer },
		{ "CrChickenVoice", CAUSE_OF_DEATH::kTalon },
		{ "CrCowVoice", CAUSE_OF_DEATH::kHoof },
		{ "CrDeerVoice", CAUSE_OF_DEATH::kHoof },
		{ "CrDogVoice", CAUSE_OF_DEATH::kBite },
		{ "CrDragonPriestVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrDragonVoice", CAUSE_OF_DEATH::kBite },
		{ "CrDraugrVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrDremoraVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrDwarvenCenturionVoice", CAUSE_OF_DEATH::kHalberd },
		{ "CrDwarvenSphereVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrDwarvenSpiderVoice", CAUSE_OF_DEATH::kPincer },
		{ "CrFalmerVoice", CAUSE_OF_DEATH::kClaws },
		{ "CrFoxVoice", CAUSE_OF_DEATH::kBite },
		{ "CrFrostbiteSpiderGiantVoice", CAUSE_OF_DEATH::kSpiderBite },
		{ "CrFrostbiteSpiderVoice", CAUSE_OF_DEATH::kSpiderBite },
		{ "CrGiantVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrGoatVoice", CAUSE_OF_DEATH::kHoof },
		{ "CrHagravenVoice", CAUSE_OF_DEATH::kTalon },
		{ "CrHareVoice", CAUSE_OF_DEATH::kPaw },
		{ "CrHorkerVoice", CAUSE_OF_DEATH::kTusks },
		{ "CrHorseVoice", CAUSE_OF_DEATH::kHoof },
		{ "CrIceWraithVoice", CAUSE_OF_DEATH::kBite },
		{ "CrMammothVoice", CAUSE_OF_DEATH::kTusks },
		{ "CrMudcrabVoice", CAUSE_OF_DEATH::kCrabPincer },
		{ "CrSabreCatVoice", CAUSE_OF_DEATH::kPaw },
		{ "CrSkeeverVoice", CAUSE_OF_DEATH::kBite },
		{ "CrSkeletonVoice", CAUSE_OF_DEATH::kSkeletalHand },
		{ "CrSlaughterfishVoice", CAUSE_OF_DEATH::kBite },
		{ "CrSprigganVoice", CAUSE_OF_DEATH::kTalon },
		{ "CrTrollVoice", CAUSE_OF_DEATH::kPaw },
		{ "CrUniqueAlduin", CAUSE_OF_DEATH::kBite },
		{ "CrUniqueOdahviing", CAUSE_OF_DEATH::kBite },
		{ "CrUniquePaarthurnax", CAUSE_OF_DEATH::kBite },
		{ "CrWerewolfVoice", CAUSE_OF_DEATH::kClaws },
		{ "CrWispVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrWitchlightVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrWolfVoice", CAUSE_OF_DEATH::kBite },
		{ "CrChaurusInsectVoice", CAUSE_OF_DEATH::kPincer },
		{ "CrDogDeathHound", CAUSE_OF_DEATH::kBite },
		{ "CrDogHusky", CAUSE_OF_DEATH::kBite },
		{ "CrGargoyleVoice", CAUSE_OF_DEATH::kClaws },
		{ "CrMistmanVoice", CAUSE_OF_DEATH::kSkeletalHand },
		{ "crAshSpawnVoice", CAUSE_OF_DEATH::kHandToHandMelee },
		{ "CrFishmanVoice", CAUSE_OF_DEATH::kClaws },
		{ "CrNetchVoice", CAUSE_OF_DEATH::kTentacle },
		{ "CrScribVoice", CAUSE_OF_DEATH::kPincer },
		{ "CrSeekerVoice", CAUSE_OF_DEATH::kTentacle },
		{ "DLC2CrBristlebackVoice", CAUSE_OF_DEATH::kTusks },
		{ "DLC2CrGiantVoiceKarstaag", CAUSE_OF_DEATH::kClub },
		{ "ccBGSSSE035_CrNixHound", CAUSE_OF_DEATH::kPincer },
		{ "ccVSVSSE003_VOC_BoneColossus", CAUSE_OF_DEATH::kSkeletalHand }
	};
	FlatMap<std::string_view, CAUSE_OF_DEATH> weaponKeywordCause{
		// Quarterstaff
		{ "WeapTypeQuarterstaff"sv, CAUSE_OF_DEATH::kQuarterstaff },
		{ "WeapTypeQtrStaff"sv, CAUSE_OF_DEATH::kQuarterstaff },
		{ "OCF_WeapTypeQuarterstaff1H"sv, CAUSE_OF_DEATH::kQuarterstaff },
		{ "OCF_WeapTypeQuarterstaff2H"sv, CAUSE_OF_DEATH::kQuarterstaff },
		// Warhammer
		{ "WeapTypeWarhammer"sv, CAUSE_OF_DEATH::kWarhammer },
		{ "OCF_WeapTypeWarhammer2H"sv, CAUSE_OF_DEATH::kWarhammer },
		{ "OCF_WeapTypeMassiveHammer2H"sv, CAUSE_OF_DEATH::kWarhammer },
		// Scythe
		{ "WeapTypeScythe"sv, CAUSE_OF_DEATH::kScythe },
		{ "OCF_WeapTypeScythe2H"sv, CAUSE_OF_DEATH::kScythe },
		{ "OCF_WeapTypeWarscythe1H"sv, CAUSE_OF_DEATH::kScythe },
		{ "OCF_WeapTypeWarscythe2H"sv, CAUSE_OF_DEATH::kScythe },
		// Halberd
		{ "WeapTypeHalberd"sv, CAUSE_OF_DEATH::kHalberd },
		{ "OCF_WeapTypeHalberd1H"sv, CAUSE_OF_DEATH::kHalberd },
		{ "OCF_WeapTypeHalberd2H"sv, CAUSE_OF_DEATH::kHalberd },
		// Spear / Javelin
		{ "WeapTypeSpear"sv, CAUSE_OF_DEATH::kSpear },
		{ "WeapTypeJavelin"sv, CAUSE_OF_DEATH::kSpear },
		{ "WeapTypePike"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypeSpear1H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypeSpear2H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypeJavelin1H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypeLance1H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypeLance2H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypePike1H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_WeapTypePike2H"sv, CAUSE_OF_DEATH::kSpear },
		// trident
		{ "OCF_WeapTypeTrident1H"sv, CAUSE_OF_DEATH::kTrident },
		{ "OCF_WeapTypeTrident2H"sv, CAUSE_OF_DEATH::kTrident },
		// glaive
		{ "OCF_WeapTypeGlaive1H"sv, CAUSE_OF_DEATH::kGlaive },
		{ "OCF_WeapTypeGlaive2H"sv, CAUSE_OF_DEATH::kGlaive },
		// Gun
		{ "WeapTypeGun"sv, CAUSE_OF_DEATH::kGun },
		{ "OCF_WeapTypeGun"sv, CAUSE_OF_DEATH::kGun },
		// Whip
		{ "WeapTypeWhip"sv, CAUSE_OF_DEATH::kWhip },
		{ "OCF_WeapTypeWhip1H"sv, CAUSE_OF_DEATH::kWhip },
		// Katana
		{ "WeapTypeKatana"sv, CAUSE_OF_DEATH::kKatana },
		{ "OCF_WeapTypeKatana1H"sv, CAUSE_OF_DEATH::kKatana },
		{ "OCF_WeapTypeKatana2H"sv, CAUSE_OF_DEATH::kKatana },
		// Claws
		{ "WeapTypeClaw"sv, CAUSE_OF_DEATH::kWeapClaws },
		{ "OCF_WeapTypeClaw1H"sv, CAUSE_OF_DEATH::kWeapClaws },
		{ "OCF_WeapUnarmed_Claws"sv, CAUSE_OF_DEATH::kWeapClaws },
		// Scimitar
		{ "WeapTypeScimitar"sv, CAUSE_OF_DEATH::kScimitar },
		{ "OCF_WeapTypeScimitar1H"sv, CAUSE_OF_DEATH::kScimitar },
		{ "OCF_WeapTypeScimitar2H"sv, CAUSE_OF_DEATH::kScimitar },
		// Club
		{ "OCF_WeapTypeClub1H"sv, CAUSE_OF_DEATH::kClub },
		{ "OCF_WeapTypeClub2H"sv, CAUSE_OF_DEATH::kClub },
		// Crossbow
		{ "OCF_WeapTypeCrossbow"sv, CAUSE_OF_DEATH::kCrossbow },
		// Dart
		{ "OCF_WeapTypeBlowgun2H"sv, CAUSE_OF_DEATH::kDart },
		// Sickle
		{ "OCF_WeapTypeSickle1H"sv, CAUSE_OF_DEATH::kSickle },
		// Fishing
		{ "ccBGSSSE001_FishingPoleKW"sv, CAUSE_OF_DEATH::kFishingRod },
		{ "OCF_WeapTypeFishingRod1H"sv, CAUSE_OF_DEATH::kFishingRod },
		// Sai
		{ "OCF_WeapTypeSai1H"sv, CAUSE_OF_DEATH::kSai },
		// Saber
		{ "OCF_WeapTypeSaber1H"sv, CAUSE_OF_DEATH::kSaber },
		{ "OCF_WeapTypeSaber2H"sv, CAUSE_OF_DEATH::kSaber },
		// Shuriken
		{ "OCF_WeapTypeShuriken1H"sv, CAUSE_OF_DEATH::kShuriken },
		// Warpick
		{ "OCF_WeapTypeWarPick1H"sv, CAUSE_OF_DEATH::kWarpick },
		{ "OCF_WeapTypeWarPick2H"sv, CAUSE_OF_DEATH::kWarpick },
		{ "OCF_WeapTypePickaxe1H"sv, CAUSE_OF_DEATH::kWarpick },
		{ "OCF_WeapTypePickaxe2H"sv, CAUSE_OF_DEATH::kWarpick },
		// Flail
		{ "OCF_WeapTypeFlail1H"sv, CAUSE_OF_DEATH::kFlail },
		{ "OCF_WeapTypeFlail2H"sv, CAUSE_OF_DEATH::kFlail },
		// Kunai
		{ "OCF_WeapTypeKunai1H"sv, CAUSE_OF_DEATH::kKunai },
		// Chakram
		{ "OCF_WeapTypeChakram1H"sv, CAUSE_OF_DEATH::kChakram },
		// Lightsaber
		{ "OCF_WeapTypeLightsaber1H"sv, CAUSE_OF_DEATH::kLightsaber },
		{ "OCF_WeapTypeLightsaber2H"sv, CAUSE_OF_DEATH::kLightsaber },
		// Boomerang
		{ "OCF_WeapTypeBoomerang1H"sv, CAUSE_OF_DEATH::kBoomerang },
		// Slingshot
		{ "OCF_WeapTypeSlingshot2H"sv, CAUSE_OF_DEATH::kSlingshot },
		// Crescent
		{ "OCF_WeapTypeCrescent1H"sv, CAUSE_OF_DEATH::kCrescent },
		{ "OCF_WeapTypeCrescent2H"sv, CAUSE_OF_DEATH::kCrescent },
		// Bomb
		{ "OCF_WeapTypeBomb1H"sv, CAUSE_OF_DEATH::kBomb },
		// Scalpel
		{ "WAF_WeapTypeScalpel"sv, CAUSE_OF_DEATH::kScalpel },
	};

	FlatMap<std::string_view, CAUSE_OF_DEATH> ammoKeywordCause{
		{ "OCF_AmmoTypeBullet"sv, CAUSE_OF_DEATH::kGun },
		{ "OCF_AmmoTypeDart"sv, CAUSE_OF_DEATH::kDart },
		{ "OCF_AmmoTypeSlingshot"sv, CAUSE_OF_DEATH::kSlingshot },
		{ "OCF_WeapEdged1H"sv, CAUSE_OF_DEATH::kOneHandAxe },
		{ "OCF_WeapPole1H"sv, CAUSE_OF_DEATH::kSpear },
		{ "OCF_AmmoTypeBomb"sv, CAUSE_OF_DEATH::kBomb },
	};

	FlatMap<std::string_view, CAUSE_OF_DEATH> magicKeywordCause{
		{ "MagicDamageFire"sv, CAUSE_OF_DEATH::kFire },
		{ "MagicDamageFrost"sv, CAUSE_OF_DEATH::kFrost },
		{ "MagicDamageShock"sv, CAUSE_OF_DEATH::kShock },
		{ "MagicDamageSun"sv, CAUSE_OF_DEATH::kSun },
		{ "PO3_MagicDamageSun"sv, CAUSE_OF_DEATH::kSun },
	};

	float             iconScale{ 0.30f };
	std::atomic<bool> reloadIcons{ false };
};
