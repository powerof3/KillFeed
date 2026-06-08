#pragma once

enum class CAUSE_OF_DEATH : std::uint8_t
{
	// weapons
	kHandToHandMelee,
	kOneHandSword,
	kOneHandDagger,
	kOneHandAxe,
	kOneHandMace,
	kTwoHandSword,
	kTwoHandAxe,
	kBow,
	kStaff,
	kStaffMagic,
	kCrossbow,
	kClub,
	kHalberd,
	kKatana,
	kWhip,
	kScythe,
	kSpear,
	kGun,
	kDart,
	kScimitar,
	kWarhammer,
	kWeapClaws,
	kTrident,
	kGlaive,
	kSickle,
	kFishingRod,
	kSai,
	kSaber,
	kShuriken,
	kWarpick,
	kFlail,
	kKunai,
	kChakram,
	kLightsaber,
	kBoomerang,
	kSlingshot,
	kCrescent,
	kBomb,
	kQuarterstaff,
	kScalpel,

	kScroll,
	kSpellCast,
	kShoutMouth,

	kFire,
	kFireDragon,
	kFireShout,

	kFrost,
	kFrostDragon,

	kShock,
	kShockDragon,
	kShockShout,

	kSun,

	kDrain,
	kDrainDragon,
	kDrainShout,

	kBanished,

	kPoison,
	kSprigganSpray,
	kAfflictedSpray,

	kShout,
	kBleeding,
	kSteam,

	// creatures
	kPaw,
	kPincer,
	kCrabPincer,
	kTalon,
	kHoof,
	kBite,
	kSpiderBite,
	kSkeletalHand,
	kTusks,
	kTusksBoar,
	kClaws,
	kTentacle,
	kIceSpear,

	// traps
	kTrapAxe,
	kTrapSpinningBlades,
	kTrapBatteringRam,
	kTrapMace,
	kTrapSpikes,
	kTrapRocks,
	kTrapGeneric,
	kLava,

	kFalling,

	kSummonExpired,

	kGeneric,

	kRaiseZombie,

	kTotal
};

using OptCause = std::optional<CAUSE_OF_DEATH>;
using OptCausePair = std::pair<OptCause, OptCause>;

// note : glaze enum requires enum values to be in fixed order
template <>
struct glz::meta<CAUSE_OF_DEATH>
{
	using enum CAUSE_OF_DEATH;

	static constexpr auto value = enumerate(
		// weapons
		"unarmed", kHandToHandMelee,
		"one-handed-sword", kOneHandSword,
		"dagger", kOneHandDagger,
		"one-handed-axe", kOneHandAxe,
		"one-handed-mace", kOneHandMace,
		"two-handed-sword", kTwoHandSword,
		"two-handed-axe", kTwoHandAxe,
		"bow", kBow,
		"staff", kStaff,
		"staff-magic", kStaffMagic,
		"crossbow", kCrossbow,
		"club", kClub,
		"halberd", kHalberd,
		"katana", kKatana,
		"whip", kWhip,
		"scythe", kScythe,
		"spear", kSpear,
		"gun", kGun,
		"dart", kDart,
		"scimitar", kScimitar,
		"warhammer", kWarhammer,
		"weap-claws", kWeapClaws,
		"trident", kTrident,
		"glaive", kGlaive,
		"sickle", kSickle,
		"fishing-rod", kFishingRod,
		"sai", kSai,
		"saber", kSaber,
		"shuriken", kShuriken,
		"war-pick", kWarpick,
		"flail", kFlail,
		"kunai", kKunai,
		"chakram", kChakram,
		"lightsaber", kLightsaber,
		"boomerang", kBoomerang,
		"slingshot", kSlingshot,
		"crescent", kCrescent,
		"bomb", kBomb,
		"quarterstaff", kQuarterstaff,
		"scalpel", kScalpel,

		"scroll", kScroll,
		"spell-cast", kSpellCast,
		"shout-mouth", kShoutMouth,

		// magic
		"fire", kFire,
		"dragon-fire", kFireDragon,
		"shout-fire", kFireShout,

		"frost", kFrost,
		"dragon-frost", kFrostDragon,

		"shock", kShock,
		"dragon-shock", kShockDragon,
		"shout-shock", kShockShout,

		"sun", kSun,

		"drain", kDrain,
		"dragon-drain", kDrainDragon,
		"shout-drain", kDrainShout,

		"banished", kBanished,

		"poison", kPoison,
		"spriggan-spray", kSprigganSpray,
		"afflicted-spray", kAfflictedSpray,

		"shout-generic", kShout,
		"bleeding", kBleeding,
		"steam", kSteam,

		// creatures
		"paw", kPaw,
		"pincer", kPincer,
		"crab-pincer", kCrabPincer,
		"talon", kTalon,
		"hoof", kHoof,
		"bite", kBite,
		"spider-bite", kSpiderBite,
		"skeletal-hand", kSkeletalHand,
		"tusks", kTusks,
		"boar-tusks", kTusksBoar,
		"claws", kClaws,
		"tentacle", kTentacle,
		"ice-spear", kIceSpear,

		// traps
		"trap-axe", kTrapAxe,
		"trap-spinning-blades", kTrapSpinningBlades,
		"trap-battering-ram", kTrapBatteringRam,
		"trap-mace", kTrapMace,
		"trap-spikes", kTrapSpikes,
		"trap-rocks", kTrapRocks,
		"trap-generic", kTrapGeneric,
		"lava", kLava,

		// environment
		"falling", kFalling,

		// misc
		"expired", kSummonExpired,
		"generic", kGeneric,
		"raise-zombie", kRaiseZombie,

		"total", kTotal);
};
