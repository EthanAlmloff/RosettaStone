// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_TRINKET_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TRINKET_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
enum class TrinketEffect { NONE, SHOP_STATS, EXTRA_SHOP_SLOT,
                           HIGHER_TIER_REFRESH, TIER_SIX_ONLY_REFRESH,
                           MAX_GOLD, GOLD_AND_MAX_GOLD,
                           // Safety Patch casts the Battlegrounds Ice Block
                           // secret and grants its immediate gold payload.
                           // The lethal-hit prevention is resolved by Hero,
                           // not by an unconditional health aura.
                           SAFETY_PATCH,
                           SHOP_STATS_AND_EXTRA_SLOT,
                           // A shop-stat portrait whose text sets an
                           // absolute minimum Tavern offer count.
                           SHOP_STATS_AND_TAVERN_SLOTS,
                           START_TURN_GOLD_PER_MINION_TYPE, IMMEDIATE_GOLD,
                           // Mysterious Orb pays immediately and restricts
                           // the next valid Trinket offer to the Lesser pool.
                           // The restriction is stateful and is consumed only
                           // when an offer modal is successfully opened.
                           IMMEDIATE_GOLD_AND_LESSER_NEXT,
                           // Lockbox Portrait repeats the Lockbox cadence at
                           // recruit start; the payload itself is resolved by
                           // Player's shared lockbox scheduler.
                           LOCKBOX_PORTRAIT,
                           ACQUIRE_RANDOM_MINIONS,
                           // Magician's Top Hat grants two independent
                           // random minions from each of tiers 1, 2, and 3
                           // on acquisition.  The executor owns hand-cap
                           // handling and samples each tier independently.
                           ACQUIRE_RANDOM_MINIONS_TIER_BATCH,
                           // Lavish Cape casts one random legal Tavern spell
                           // per distinct friendly minion type, immediately
                           // and again at each recruit start.
                           START_TURN_RANDOM_TAVERN_SPELLS_PER_TYPE,
                           // Glass of Perspective grants one random
                           // Battlegrounds Choose One minion on acquisition
                           // and repeats that reward at each recruit start.
                           ACQUIRE_RANDOM_CHOOSE_ONE,
                           // Sacrificial Altar is an acquisition-time
                           // conversion: remove every friendly warband
                           // minion and award three Gold per removed entity.
                           // Keep it typed so the destructive one-shot
                           // payload cannot be mistaken for a passive aura.
                           SACRIFICIAL_ALTAR,
    //! Errgl Sticker's exact two-card Mrrglton reward pool.
    ACQUIRE_RANDOM_MRGLTON,
                           // Ominous Stone opens a Tier-4, most-common-type
                           // minion Discover paired with an executable Dark
                           // Gift.  The modal and gift are resolved at the
                           // selection boundary, not during acquisition.
                           OMINOUS_STONE_DISCOVER,
                           // Wax Lance opens a Tier-7 minion Discover paired
                           // with a legal Dark Gift at acquisition.
                           WAX_LANCE_DISCOVER,
                           // Discovers a plain warband copy with a Dark Gift
                           // on acquisition and again at recruit start.
                           MALDRAXXUS_DAGGER_DISCOVER,
                           // Baller Portrait grants one random canonical
                           // Fire/Snow Baller on acquisition and at each
                           // subsequent recruit start.
                           ACQUIRE_RANDOM_BALLER,
                           ACQUIRE_RANDOM_FRIENDLY_COPY,
                           START_TURN_RANDOM_MINIONS,
                           AFTER_FRIENDLY_DEATH_RANDOM_MINION,
                           AFTER_SELL_RANDOM_MINION,
                           ACQUIRE_FIXED_CARD,
                           // Primalfin Portrait grants the executable
                           // Primalfin Lookout; each resulting minion
                           // Discover also queues one random Tavern spell.
                           ACQUIRE_PRIMALFIN_PORTRAIT,
                           // Fixed-card portrait whose first purchased
                           // Pirate each recruit turn is free.
                           ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE,
                           ACQUIRE_FIXED_CARD_AND_BOUNTIES,
                           START_TURN_RANDOM_BOUNTIES,
                           // Acquisition portraits that grant two distinct
                           // generated minions (for example Goldgrubber
                           // Portrait). Keeping both identities in the
                           // descriptor avoids a second ad-hoc card path.
                           ACQUIRE_TWO_FIXED_CARDS,
                           // Acquisition plus a fixed Tavern-capacity target
                           // (for example Felbat Portrait: grant Famished
                           // Felbat and keep seven Tavern offers).
                           ACQUIRE_FIXED_CARD_AND_TAVERN_SLOTS,
                           ACQUIRE_FIXED_GLOWSCALE,
                           ACQUIRE_FIXED_LIONFISH,
                           END_TURN_MAX_GOLD,
                           END_TURN_GOLDEN_STATS,
                           AFTER_PLAY_HAND_BUFF,
                           AFTER_PLAY_ELEMENTAL_SHOP_BUFF,
                           // Fountain Pen modifies the payload of an
                           // Elemental stat-giver; it is not itself a play
                           // trigger.
                           ELEMENTAL_STAT_GIVER_BONUS,
                           // Amplifying Essence adds a stat payload to every
                           // Elemental stat-giver and improves that payload
                           // after its owned lifetime play threshold. The
                           // threshold/progress are per Trinket instance.
                           ESCALATING_ELEMENTAL_STAT_GIVER_BONUS,
                           AFTER_TAVERN_SPELL_SHOP_BUFF,
                           // Felsteel Cleaver consumes the Tavern minion
                           // targeted by a successful spell after resolution.
                           AFTER_SPELL_ON_SHOP_CONSUME,
                           AFTER_TAVERN_SPELL_RACE_BUFF,
                           TAVERN_SPELL_STATS,
                           TAVERN_SPELL_GROWING_STATS,
                           TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST,
                           AFTER_BUY_RANDOM_FRIENDLY_BUFF,
                           // Reusable Batteries grants one Magnetic
                           // Satellite copied from the first minion bought
                           // each recruit turn.
                           AFTER_BUY_MINION_MAGNETIC_SATELLITE,
                           // Trusty Crowbar buffs the left-most friendly
                           // minion whenever a Pirate is acquired.
                           AFTER_GET_PIRATE_LEFTMOST_STATS,
                           // Secret Schematic grants one random Tavern spell
                           // after each successful purchase of a Mechanical
                           // minion.  The trigger is purchase-scoped (not a
                           // generic minion-acquisition aura) so failed/full
                           // hand purchases cannot award a spell.
                           AFTER_BUY_MECH_RANDOM_SPELL,
                           END_TURN_DIVINE_SHIELD_ATTACK,
                           AFTER_PLAY_CARD_RANDOM_RACE_BUFF,
                           STATIC_RACE_STATS,
                           AFTER_OUTSIDE_COMBAT_DESTROY_STATS,
                           AFTER_OUTSIDE_COMBAT_DESTROY_GOLD,
                           AFTER_REBORN_STATS,
                           // Funeral Wreath adds a plain copy of each
                           // friendly Reborn minion, capped per combat.
                           AFTER_REBORN_COPY,
                           // Deathtouch Apple restores Reborn to a friendly
                           // Undead after its combat Reborn resolves.  The
                           // value is the per-recruit-turn trigger cap.
                           AFTER_REBORN_UNDEAD_REBORN,
                           DUPLICATE_DRAGON_BATTLECRY,
                           FIRST_MINION_DIVINE_SHIELD,
                           BATTLECRY_BUY_DISCOUNT,
                           REFRESH_SHOP_STATS,
                           // Lightning in a Bottle copies the highest-Attack
                           // Tavern minion's final stats to the lowest-Attack
                           // offer after each successful refresh.
                           REFRESH_HIGHEST_ATTACK_TO_LOWEST_STATS,
                           // Demonic Tapestry arms one highest-tier Tavern
                           // minion purchase to be paid with Health after
                           // `value` successful refreshes. `amount` is the
                           // health cost and `statScale` on the owned effect
                           // is the one-purchase armed marker.
                           REFRESH_HIGHEST_TIER_HEALTH_PURCHASE,
                           // The Eye of Sargeras replaces every fourth
                           // successful minion purchase's Gold cost with
                           // Health.  Progress is kept on each owned
                           // Trinket instance; the purchase executor owns
                           // the payment boundary.
                           BUY_MINION_HEALTH_CADENCE,
                           REFRESH_UPGRADE_COST_DISCOUNT,
                           HERO_DAMAGE_SHOP_STATS,
                           STATIC_MINION_STATS,
                           BLOOD_GEM_BONUS,
                           START_COMBAT_MINION_STATS,
                           STATIC_TIER_MINION_STATS,
                           STATIC_FODDER_SHOP_STATS,
                           // Arm one Fodder offer on the next successful
                           // refresh after a Tavern spell is cast.
                           ARM_FODDER_REFRESH,
                           TAVERN_STATS_PER_SOLD,
                           UPGRADE_COST_DISCOUNT,
                           CONDUCTOR_DISCARD_BLOOD_GEM,
                           SHIP_IN_A_BOTTLE,
                           BRONZEBEARD_PORTRAIT,
                           NEXT_TAVERN_SPELL_DISCOUNT,
                           STAT_TAVERN_SPELL_DISCOUNT,
                           FREE_TAVERN_SPELL_USES,
                           START_TURN_GOLD_DAMAGE,
                           REFRESH_TEMP_SHOP_STATS,
                           AVENGE_MINION_STATS,
                           // Cloud Serpent Horn gives (copies) the right-most
                           // friendly minion's Attack to another friendly
                           // Dragon after three friendly deaths.  The source
                           // keeps its Attack; the combat result tracks one
                           // trigger per owned Horn instance.
                           AVENGE_RIGHTMOST_ATTACK_TO_DRAGON,
                           // Summon a 2/2 Beetle after the Avenge threshold.
                           // `amount` carries the number of Beetles (golden = 2).
                           AVENGE_SUMMON_BEETLES,
                           TAVERN_SPELL_TEMP_STATS_AFTER_DAMAGE,
                           PIRATE_ATTACK_GOLD,
                           ATTACKING_MINION_STATS,
                           START_COMBAT_HEALTH_FROM_ATTACK,
                           AVENGE_TAVERN_SPELL_ATTACK,
                           REFRESH_EXTRA_SHOP_SLOTS,
                           MAGNETIC_MECH_COST_AND_REFRESH_SLOT,
                           SPELL_COUNT_MINION_ATTACK,
                           END_TURN_UNDEAD_ATTACK,
                           REACH_TIER_GOLD,
                           DELAYED_GOLD,
                           SPELL_CAST_MINION_STATS,
                           // Bluegill Flippers affects only the left-most
                           // minion in hand and the left-most minion in the
                           // warband after each successful Tavern spell.
                           SPELL_CAST_LEFTMOST_MINION_STATS,
                           // Combat-only trigger: the next five summoned
                           // friendly minions each receive Divine Shield.
                           SUMMON_DIVINE_SHIELD,
                           // Mug of the Sire fires when a friendly summon is
                           // attempted against a full warband. `attack` is
                           // the permanent attack granted to all minions.
                           AFTER_SUMMON_OVERFLOW_STATS,
                           START_COMBAT_NAGA_SPELLCRAFT,
                           // Repeat every friendly Start-of-Combat effect
                           // once for the owning combat board.
                           START_COMBAT_EXTRA_TRIGGER,
                           AFTER_PLAY_NAGA_SPELLCRAFT,
                           SUMMON_MECH_RANDOM_DIVINE_SHIELD,
                           START_COMBAT_QUILBOAR_BLOOD_GEMS,
                           SUMMON_BEAST_DOUBLE_ATTACK,
                           SUMMON_BEAST_STATS,
                           SUMMON_BEAST_RANDOM_MINION,
                           START_COMBAT_ELEMENTAL_FROSTLING,
                           START_COMBAT_BEAST_SCALING,
                           START_COMBAT_QUILBOAR_BLOOD_GOLEM,
                           START_COMBAT_EDGE_SHIELDS,
                           START_COMBAT_LEFT_COPY,
                           START_COMBAT_FIRST_SUMMON_COPY,
                           // Boom Controller remembers the first friendly
                           // Mech death and summons an exact combat copy when
                           // a slot becomes available.
                           BOOM_CONTROLLER_FIRST_MECH_COPY,
                           MECH_DIVINE_SHIELD_REPAIR,
                           ACQUIRE_FLAGBEARER_PORTRAIT,
                           START_COMBAT_AUTOMATON_SUMMON,
                           // Assembler Portrait magnetizes one canonical Auto
                           // Assembler onto every friendly Mech at combat
                           // start.  Keep this distinct from summoning an
                           // Automaton: the attachment carries Magnetic and
                           // Deathrattle semantics through Minion.
                           START_COMBAT_AUTO_ASSEMBLER,
                           START_COMBAT_UNDEAD_EDGE_REBORN,
                           START_COMBAT_TRIGGER_DEATHRATTLES,
                           START_COMBAT_HIGHEST_HAND_MINION,
                           AFTER_DEATHRATTLE_RIGHTMOST_STATS,
                           // Blood Amulet plays one permanent Blood Gem on
                           // three distinct random friendly minions after a
                           // friendly Deathrattle resolves.
                           AFTER_DEATHRATTLE_BLOOD_GEMS,
                           START_COMBAT_NEUTRAL_TRIPLE,
                           START_COMBAT_DRAGON_MAX_ATTACK,
                           START_COMBAT_LEFTMOST_HAND_STATS,
                           START_COMBAT_LOWEST_ATTACK_DOUBLE,
                           START_COMBAT_LEFT_BEAST_SHIELDS,
                           START_COMBAT_HIGHEST_TIER_DRAGON_GOLDEN,
                           START_COMBAT_THREE_BLOOD_GEMS,
                           START_COMBAT_TYPE_STATS,
                           START_COMBAT_NAGA_HEALTH,
                           START_COMBAT_RANDOM_PIRATE_SHIELDS,
                           START_COMBAT_MURLOC_MAX_ATTACK,
                           START_COMBAT_RALLY_SHIELDS,
                           START_COMBAT_DRAGON_SHIELDS,
                           START_COMBAT_NAGA_DOUBLE_STATS,
                           AVENGE_BLOOD_GEM_BONUS,
                           AVENGE_RANDOM_UNDEAD_REBORN,
                           FIRST_DEATH_MAX_STATS_RANDOM,
                           AVENGE_RANDOM_MAGNETIC,
                           AFTER_TWO_ATTACKS_QUILBOAR_GEM,
                           SPELL_COUNT_RANDOM_NAGA,
                           SPELL_COUNT_GOLD_ON_MINION,
                           // Bubble Crown improves future Tavern-spell stat
                           // payloads once after twelve successful spells.
                           SPELL_COUNT_TAVERN_SPELL_STATS,
                           // Rune of Transmutation replaces its owned
                           // instance after fifteen successful Tavern spells
                           // with a random executable Greater Naga Trinket.
                           SPELL_COUNT_REPLACE_GREATER_NAGA,
                           // Bloodbound Earrings: `value` is the recurring
                           // successful-spell threshold and `amount` is the
                           // number of Blood Gems played on each minion per
                           // completed threshold. The printed "(N left!)"
                           // counter is progress toward the next trigger,
                           // not a lifetime activation count.
                           SPELL_COUNT_BLOOD_GEMS,
                           TAVERN_SPELL_NO_TYPE_STATS,
                           BLOOD_GEM_DIVINE_SHIELD,
                           ATTACKING_DRAGON_DIVINE_SHIELD,
                           ATTACKING_BEAST_SCALING,
                           END_TURN_BLOOD_GEMS_PER_TYPE,
                           END_TURN_MURLOC_STATS,
                           AFTER_MAGNETIC_MECH_REPAIR,
                           AFTER_MAGNETIZE_STATS,
                           AFTER_FRIENDLY_DAMAGE_STATS,
                           END_TURN_MINION_STATS,
                           // Charming Panpipes buffs only the left-most
                           // warband minion and improves once per spell.
                           END_TURN_LEFTMOST_MINION_STATS_PER_SPELL,
                           END_TURN_LEFT_STATS_PER_BATTLECRY,
                           RALLY_ATTACK_FREE_REFRESH,
                           BATTLECRY_EDGE_STATS,
                           END_TURN_LEFT_DEATHRATTLES,
                           END_TURN_RALLY_TRIGGERS,
                           AFTER_SPELLCRAFT_DIVINE_SHIELD_STATS,
                           AFTER_BEAST_ATTACK_STATS,
                           HERO_POWER_TWICE,
                           // A second Hero Power activation this recruit
                           // turn; distinct from HERO_POWER_TWICE, which
                           // duplicates one activation's effect.
                           HERO_POWER_EXTRA_USE,
                           SPELLCRAFT_TRANSFORM_HIGHER_TIER,
                           // Spellcraft markers whose generated tokens use
                           // the reviewed Tavern-spell executors.  Keeping
                           // these distinct from generic transform markers
                           // makes coverage and future lifecycle checks
                           // preserve the printed effect.
                           SPELLCRAFT_CONSUME_SHOP_STATS,
                           // Vibrant Bubble is a lesser Trinket whose only
                           // executable payload is its generated Murloc
                           // keyword Spellcraft token.  Keep the source
                           // marker distinct from token effects so coverage
                           // and acquisition cannot credit the wrong target.
                           SPELLCRAFT_MURLOC_KEYWORD,
                           // Ophidian Staff supplies a temporary Spellcraft
                           // token whose TavernSpellBehavior owns the exact
                           // Beast +2/+2 and Reborn target contract.
                           SPELLCRAFT_OPHIDIAN_STAFF,
                           SPELLCRAFT_TRIGGER_DEATHRATTLE,
                           // Chillmere Mosaic supplies a Spellcraft token
                           // that refreshes the Tavern with Battlecry
                           // minions costing one Gold.
                           SPELLCRAFT_CHILLMERE_MOSAIC,
                           PRECIOUS_PEARL_SPELLCRAFT,
                           BOOK_OF_MEDIVH_DISCOVER,
                           AZEROTH_MODEL_GLOBE,
                           END_TURN_RANDOM_TYPE_MINIONS,
                           ACQUIRE_LAST_OPPONENT_COPY,
                           FIRST_SPELL_REPEAT,
                           // Lovely Locket repeats every successful spell
                           // cast on a friendly minion onto another friendly
                           // minion.  This is distinct from Cathedral's
                           // once-per-turn first-spell replay.
                           AFTER_FRIENDLY_SPELL_REPEAT,
                           SPELLCRAFT_REPEAT,
                           // Trailblazer Sticker makes every supported
                           // Choose One source resolve both branches.
                           TRAILBLAZER_CHOOSE_ONE,
                           AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL,
                           AFTER_DIVINE_SHIELD_LOST_RANDOM_SPELL,
                           AFTER_BLOOD_GEM_DIVINE_SHIELD,
                           END_TURN_EXTRA_TRIGGER,
                           // Re-fire the left- and right-most friendly
                           // minions' Battlecries at recruit end.
                           END_TURN_BATTLECRY_TRIGGER,
                           // War Drum repeats one successful friendly
                           // Battlecry twice per recruit turn.  The
                           // per-instance triggerProgress marker is reset at
                           // recruit start, so the effect is not a lifetime
                           // one-shot and cannot be consumed by failed plays.
                           BATTLECRY_EXTRA_TRIGGERS,
                           // Add a random Tavern spell after playing an
                           // Elemental, with `value` as the per-turn cap.
                           AFTER_PLAY_ELEMENTAL_RANDOM_SPELL,
                           // Count played Murlocs toward a recurring random
                           // Tavern-spell reward. `value` is the threshold
                           // and `amount` is the reward count.
                           AFTER_PLAY_MURLOC_RANDOM_SPELL,
                           // Magicfin Sticker rewards a taught 1/1 Murloc
                           // after a successful Tavern-spell purchase;
                           // `value` is its per-recruit-turn cap.
                           AFTER_BUY_TAVERN_SPELL_MURLOC,
                           AFTER_PLAY_ELEMENTAL_FREE_REFRESH,
                           AFTER_PLAY_DEMON_DAMAGE,
                           // Ur'zul Sticker makes another friendly Demon
                           // consume one Tavern minion after a Demon is
                           // played.
                           AFTER_PLAY_DEMON_CONSUME,
                           // Consuming Claw copies every Bonus Keyword from
                           // a Tavern minion consumed by a friendly Demon and
                           // adds its fixed stat payload at that boundary.
                           DEMON_CONSUME_BONUS_KEYWORDS,
                           END_TURN_LEFTMOST_STATS_PER_BATTLECRY,
                           END_TURN_FIXED_CARD,
                           // Inductive Gyroblade creates a Magnetic
                           // Satellite at recruit end whose stats scale by
                           // Tavern spells successfully cast this turn.
                           END_TURN_SPELL_SCALED_SATELLITE,
                           // At recruit end, grant copies of the last Tavern
                           // spell cast this turn.  The spell identity is
                           // resolved from Season14's public last-spell
                           // state, not from a hard-coded card pool.
                           END_TURN_LAST_TAVERN_SPELL,
                           // Emergency Gearblade casts the canonical Repair
                           // Job on the left-most friendly Mech at recruit
                           // end through the normal spell lifecycle.
                           END_TURN_LEFTMOST_MECH_REPAIR,
                           AFTER_BUY_BATTLECRY_MINION,
                           START_COMBAT_GOLDEN_FISH,
                           SPEND_GOLD_PIRATE_STATS,
                           SPEND_GOLD_PIRATE_THRESHOLD_STATS,
                           // `value` is the spend threshold and `amount` is
                           // the lifetime trigger cap. Progress is stored on
                           // the owned Season14PersistentEffect instance.
                           SPEND_GOLD_DOUBLE_ATTACK,
                           // Spend threshold reward: one random Golden
                           // minion from the requested Tavern tier.
                           SPEND_GOLD_RANDOM_GOLDEN,
                           // Fancy Spellbook casts Shiny Ring once after
                           // seven successfully spent Gold. `value` is the
                           // threshold and the owned effect's progress
                           // tracks partial spend across actions.
                           SPEND_GOLD_SHINY_RING,
                           // Designer Eyepatch lowers the triple threshold
                           // for Pirates only: two matching non-golden copies
                           // become one Golden Pirate.
                           TWO_COPIES_MAKE_GOLDEN,
                           // Patch 36.4 typed trinket lifecycles.  These
                           // descriptors are intentionally separate from
                           // generic acquisition markers so the existing
                           // Player/Battle event paths can dispatch them
                           // without string-based exceptions.
                           DEATHLY_PHYLACTERY,
                           MYSTERY_CUBE_REPLACE_LESSER,
                           TICKATUS_DARKMOON_PRIZE,
                           AFTER_PLAY_MINION_RANDOM_TIER_SPELL,
                           // "After you Discover a minion, add a plain copy"
                           SINSTONE_DISCOVER_COPY,
                           SOUVENIR_STAND_GREATER_COPY,
                           TRIP_VOUCHERS_REPLACE_GREATER,
                           INNKEEPERS_HEARTH_DISCOVER,
                           TRANSFORM_WARBAND_TIER,
                           AFTER_FIRST_SELL_BLOOD_GEMS_TAVERN,
                           END_TURN_HIGHEST_TIER_TAVERN,
                           BUY_DEMON_HEALTH_ONCE_PER_TURN,
                           DEMON_CONSUME_HIGHEST_HEALTH,
                           AFTER_TAVERN_MINION_CONSUMED_RANDOM_SPELL,
                           GOLD_PENDANT_GOLDENIZE,
                           END_TURN_GOLDEN_LEFTMOST_STATS,
                           AFTER_DEATHRATTLE_TEMP_BLOOD_GEM_BONUS,
                           JEWELRY_BOX_BLOOD_GEM,
                           START_COMBAT_POWDER_KEG,
                           START_COMBAT_SOUL_FERMENTER,
                           AFTER_SELL_HERO_POWER_BUDDY,
                           ACQUIRE_RANDOM_TRINKET,
                           KALEIDOSCOPE_DISCOVER,
                           SPELLCRAFT_DOUBLE_STITCH,
                           ACQUIRE_RANDOM_CHROMADRAKES,
                           POCKET_CYCLONE,
                           AFTER_PLAY_ELEMENTAL_FIXED_CARD,
                           REFRESH_DOUBLE_HIGHEST_HEALTH,
                           ACQUIRE_FIXED_CARD_AFTER_SELL,
                           SPELLCRAFT_MIGHT_OF_STORMWIND,
                           AFTER_BUY_MINION_COPY };

//! Extra text carried by a fixed-portrait Trinket.  This is deliberately
//! separate from TrinketEffect: the fixed card can be executable while the
//! portrait's account-wide modifier is not yet represented by the simulator.
//! Keeping the distinction in the descriptor prevents an acquisition-only
//! registration from receiving full executable-coverage credit.
enum class PortraitEffect {
    NONE,
    ETERNAL_KNIGHT_TAUNT_REBORN,
    LIVING_AZERITE_ELEMENTAL_STATS,
    BELCHER_VENOMOUS_LOSS_STATS,
    SURPRISE_MORE_ELEMENTALS,
    KABOOM_BOT_DEATHRATTLE_DAMAGE,
    WHELP_SMUGGLER_STATS_AND_DRAGON,
    ZESTY_SHAKER_EXTRA_COPY,
    FELBLOOD_BOTH_STATS,
    LIGHTFANG_ALL_TYPES,
    BRISTLEBACH_ALL_MINIONS,
    BASSGILL_SUMMON_DIVINE_SHIELD,
    SELFLESS_BATTLECRY,
    GROUNDBREAKER_ADJACENT_STATS,
    DRAKKARI_ENCHANTER_ALL_TYPES,
    CZARINA_DIVINE_SHIELD_HEALTH,
    SOUL_REWINDER_EXTRA_ATTACK,
    FLAMING_ENFORCER_ADJACENT_STATS,
    ETERNAL_KNIGHT_UNDEAD_STATS,
    GLOWSCALE_DIVINE_SHIELD_STATS,
    LIONFISH_BEAST_STATS,
    // Flighty Portrait extends each successful Tavern-spell resolution to
    // every Flighty Scout held in hand or in the warband.
    FLIGHTY_SCOUT_TAVERN_SPELL_STATS,
    // Promo Portrait repeats the first friendly Start-of-Combat effect and
    // grants a Prized Promo-Drake on acquisition.
    PROMO_START_COMBAT_EXTRA_TRIGGER,
    // Hackerfin Portrait makes every owned Hackerfin repeat its Battlecry at
    // recruit end.  The marker is stored on each Minion instance so copies
    // retain the lifecycle through hand/board movement.
    HACKERFIN_END_TURN_BATTLECRY,
    RYLAK_START_COMBAT_DEATHRATTLES,
    MACAW_LEFTMOST_BATTLECRY,
    SKY_GOLEM_DEATHRATTLE_STATS,
    SCRAPSMITH_TAUNT_DEATH_GEMS
    ,GOOSE_FLEDGLING_REWARD
    ,VINESPEAKER_BLOOD_GEM_HEALTH
    ,PERMANENT_SPELLCRAFT
};
// End-of-recruit persistent race aura.
// Effects whose trigger is a successful recruit-phase refresh or self-damage
// are deliberately separate from static auras: their counters must survive
// Tavern replacement and be replayable from Season14State.
                           
                           
                           
struct TrinketBehavior
{
    TrinketEffect effect = TrinketEffect::NONE;
    int attack = 0;
    int health = 0;
    int value = 0;
    Race race = Race::INVALID;
    int tier = 0;
    int amount = 0;
    bool repeatAtStartTurn = false;
    bool battlecryOnly = false;
    bool magneticOnly = false;
    // Canonical generated card for fixed-card acquisition effects.
    std::string_view cardID{};
    //! Portrait-only modifier, if the Trinket is one of the fixed portraits.
    PortraitEffect portraitEffect = PortraitEffect::NONE;
    //! True only when the modifier above is fully applied by the simulator.
    bool portraitExecutable = false;
    // Optional count for a recurring fixed-card grant when it differs from
    // the acquisition count (for example Essence of Dreams: 2 now, 1 each
    // subsequent recruit start).
    int startTurnAmount = 0;
    // Optional recruit-start cadence for recurring fixed-card grants.  A
    // value of one means every recruit start; larger values count starts
    // between grants while preserving the per-Trinket trigger counter.
    int startTurnCadence = 1;
    // Optional second generated card for ACQUIRE_TWO_FIXED_CARDS.
    std::string_view secondaryCardID{};
    // If true, random acquisition draws without replacement from the
    // filtered pool.  Season 14 uses this for "different" minion rewards.
    bool distinct = false;
};

//! Exact, executable subset of Patch 36.4 passive Trinkets.
TrinketBehavior FindTrinketBehavior(std::string_view id) noexcept;
class TrinketBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds
#endif
