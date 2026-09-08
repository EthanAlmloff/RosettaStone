#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
TrinketBehavior FindTrinketBehavior(std::string_view id) noexcept
{
    if (id == "BG30_MagicItem_416") return {TrinketEffect::SPELLCRAFT_TRANSFORM_HIGHER_TIER};
    // Ancient Wishbone duplicates the resolved effect of one active Hero
    // Power activation. The bridge executor owns the multiplier; this marker
    // must never expand CanUseHeroPower or UseHeroPower.
    if (id == "BG30_MagicItem_804") return {TrinketEffect::HERO_POWER_TWICE};
    // Only effects whose complete acquisition and lifecycle semantics are
    // implemented are registered here.  Nether Pendant and both Cheese
    // Wheel forms improve on a counter; Innkeeper's Stein and Minion Bait
    // trigger on refresh; Goblin Wallet triggers at end of turn.  Treating
    // any of those as an unconditional aura/bonus would silently change the
    // game, so they remain fail-closed until their trigger state is modeled.
    if (id == "BG30_MagicItem_996") return {TrinketEffect::GOLD_AND_MAX_GOLD, 0, 0, 4};
    if (id == "BG30_MagicItem_998") return {TrinketEffect::IMMEDIATE_GOLD, 0, 0, 2};
    if (id == "BG30_MagicItem_841") return {TrinketEffect::SHOP_STATS_AND_EXTRA_SLOT, 3, 3, 1};
    if (id == "BG36_MagicItem_220") return {TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE};
    if (id == "BG32_MagicItem_858") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 4, 3};
    if (id == "BG30_MagicItem_993") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 7, 1, true};
    if (id == "BG30_MagicItem_430") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 1, true, true, false};
    if (id == "BG32_MagicItem_172") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG31_176"};
    if (id == "BG30_MagicItem_543") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "EBG_Spell_032"};
    if (id == "BG35_MagicItem_303") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_072"};
    // These effects are resolved at recruit-end by Game, after all player
    // actions have completed.  Keeping them out of the generic passive path
    // prevents Goblin Wallet from increasing the cap before its first turn.
    if (id == "BG30_MagicItem_847") return {TrinketEffect::END_TURN_MAX_GOLD, 0, 0, 1};
    if (id == "BG32_MagicItem_231") return {TrinketEffect::END_TURN_GOLDEN_STATS, 3, 3};
    if (id == "BG32_MagicItem_231t") return {TrinketEffect::END_TURN_GOLDEN_STATS, 10, 10};
    if (id == "BG30_MagicItem_914") return {TrinketEffect::AFTER_PLAY_HAND_BUFF, 3, 3};
    if (id == "BG30_MagicItem_914t") return {TrinketEffect::AFTER_PLAY_HAND_BUFF, 6, 6};
    if (id == "BG30_MagicItem_544") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF, 2, 2};
    if (id == "BG30_MagicItem_544t") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_SHOP_BUFF, 5, 5};
    if (id == "BG36_MagicItem_800") return {TrinketEffect::AFTER_TAVERN_SPELL_SHOP_BUFF, 1, 1};
    if (id == "BG35_MagicItem_710") return {TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF, 2, 2, 0, Race::PIRATE};
    if (id == "BG30_MagicItem_414") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 2, 1, 2};
    if (id == "BG30_MagicItem_414t") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 4, 4, 2};
    if (id == "BG30_MagicItem_984") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 3, 0};
    if (id == "BG30_MagicItem_984t") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 7, 0};
    if (id == "BG30_MagicItem_900") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 4, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_900t") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 6, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_989") return {TrinketEffect::STATIC_RACE_STATS, 3, 0, 0, Race::UNDEAD};
    if (id == "BG30_MagicItem_989t") return {TrinketEffect::STATIC_RACE_STATS, 15, 0, 0, Race::UNDEAD};
    if (id == "BG36_MagicItem_205") return {TrinketEffect::AFTER_REBORN_STATS, 2, 2};
    if (id == "BG36_MagicItem_215") return {TrinketEffect::DUPLICATE_DRAGON_BATTLECRY};
    if (id == "BG36_MagicItem_811") return {TrinketEffect::FIRST_MINION_DIVINE_SHIELD};
    if (id == "BG36_MagicItem_202") return {TrinketEffect::BATTLECRY_BUY_DISCOUNT, 0, 0, 2};
    // Season 14's refresh/self-damage Trinkets retain their progress in the
    // player-owned state.  `value` is the trigger threshold and `amount` is
    // the permanent +A/+H improvement; the initial aura is attack/health.
    if (id == "BG30_MagicItem_879") return {TrinketEffect::REFRESH_SHOP_STATS, 1, 1, 4, Race::INVALID, 0, 1};
    if (id == "BG30_MagicItem_879t") return {TrinketEffect::REFRESH_SHOP_STATS, 2, 2, 4, Race::INVALID, 0, 1};
    if (id == "BG30_MagicItem_541") return {TrinketEffect::HERO_DAMAGE_SHOP_STATS, 2, 2, 3, Race::INVALID, 0, 1};
    if (id == "BG30_MagicItem_423") return {TrinketEffect::HIGHER_TIER_REFRESH};
    if (id == "BG30_MagicItem_880") return {TrinketEffect::STATIC_MINION_STATS, 2, 1};
    if (id == "BG30_MagicItem_880t") return {TrinketEffect::STATIC_MINION_STATS, 8, 5};
    if (id == "BG30_MagicItem_988") return {TrinketEffect::BLOOD_GEM_BONUS, 2, 1, 0, Race::INVALID, 0, 3};
    if (id == "BG30_MagicItem_988t") return {TrinketEffect::BLOOD_GEM_BONUS, 3, 3, 0, Race::INVALID, 0, 5};
    if (id == "BG30_MagicItem_970") return {TrinketEffect::START_COMBAT_MINION_STATS, 2, 2};
    if (id == "BG30_MagicItem_970t") return {TrinketEffect::START_COMBAT_MINION_STATS, 6, 6};
    if (id == "BG30_MagicItem_843t") return {TrinketEffect::STATIC_TIER_MINION_STATS, 7, 5, 3};
    if (id == "BG30_MagicItem_547") return {TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF, 1, 0, 0, Race::UNDEAD};
    if (id == "BG30_MagicItem_547t") return {TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF, 2, 0, 0, Race::UNDEAD};
    if (id == "BG35_MagicItem_151") return {TrinketEffect::STATIC_FODDER_SHOP_STATS, 4, 4};
    if (id == "BG35_MagicItem_151t") return {TrinketEffect::STATIC_FODDER_SHOP_STATS, 15, 15};
    if (id == "BG30_MagicItem_992") return {TrinketEffect::TAVERN_STATS_PER_SOLD, 1, 1};
    if (id == "BG30_MagicItem_992t") return {TrinketEffect::TAVERN_STATS_PER_SOLD, 2, 2};
    // Bartend-o-Tron's Oilcan is a persistent -3 Tavern upgrade aura.
    if (id == "BG30_MagicItem_705") return {TrinketEffect::UPGRADE_COST_DISCOUNT, 0, 0, 3};
    if (id == "BG30_MagicItem_402") return {TrinketEffect::CONDUCTOR_DISCARD_BLOOD_GEM, 0, 0, 1, Race::INVALID, 0, 1, false, false, false, "BG28_585"};
    if (id == "BG30_MagicItem_407") return {TrinketEffect::SHIP_IN_A_BOTTLE};
    if (id == "BG30_MagicItem_418") return {TrinketEffect::BRONZEBEARD_PORTRAIT};
    if (id == "BG30_MagicItem_714") return {TrinketEffect::PRECIOUS_PEARL_SPELLCRAFT};
    if (id == "BG30_MagicItem_420") return {TrinketEffect::BOOK_OF_MEDIVH_DISCOVER, 0, 0, 1};
    if (id == "BG30_MagicItem_420t") return {TrinketEffect::BOOK_OF_MEDIVH_DISCOVER, 0, 0, 2};
    // Azeroth Model Globe: every two recruit starts, grant 2 Gold and open
    // one Tier 6 minion Discover.  The cadence is stored per Trinket in
    // triggerProgress; a full hand leaves it armed for a later retry.
    if (id == "BG30_MagicItem_425") return {TrinketEffect::AZEROTH_MODEL_GLOBE, 0, 0, 2, Race::INVALID, 6, 2};
    // Exquisite Dishware grants one random minion for every distinct race in
    // the warband at recruit end.  The race is sampled from each represented
    // type independently and hand capacity remains authoritative.
    if (id == "BG30_MagicItem_419") return {TrinketEffect::END_TURN_RANDOM_TYPE_MINIONS};
    // Colorful Compass gives a random supported minion immediately and once
    // at each recruit start; the generic random-minion path preserves retry
    // semantics when the hand is full.
    if (id == "BG30_MagicItem_426") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 1, true};
    if (id == "BG30_MagicItem_426t") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 2, true};
    // Reflective Pendant gives a plain copy of a random living friendly
    // minion immediately and repeats at each recruit start.
    if (id == "BG30_MagicItem_706") return {TrinketEffect::ACQUIRE_RANDOM_FRIENDLY_COPY, 0, 0, 0, Race::INVALID, 0, 1, true};
    // Essence of Dreams grants two copies immediately, then one at each
    // recruit start.  The generated card is the pinned Dreamer's Embrace
    // Tavern spell; amount is the acquisition count, not a tier.
    if (id == "BG30_MagicItem_916") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 2, true, false, false, "BG28_883", PortraitEffect::NONE, false, 1};
    // Mecha-Jaraxxus Sticker grants magnetic Mecha-Demons twice immediately
    // and twice again at each recruit start.
    if (id == "BG30_MagicItem_942") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::DEMON, 0, 2, true, false, true};
    // Burgling Claw resolves from the last publicly observed opponent field;
    // it is intentionally a start-turn-only plain copy.
    if (id == "BG30_MagicItem_930") return {TrinketEffect::ACQUIRE_LAST_OPPONENT_COPY, 0, 0, 0, Race::INVALID, 0, 1, true};
    // Replica Cathedral repeats only the first successful spell each recruit
    // turn; triggerProgress is reset at recruit start and is per Trinket.
    if (id == "BG30_MagicItem_434") return {TrinketEffect::FIRST_SPELL_REPEAT};
    // Spitescale Sushi Roll repeats the first two Spellcraft casts each
    // recruit turn.  The per-turn counter lives on this Trinket instance.
    if (id == "BG30_MagicItem_920") return {TrinketEffect::SPELLCRAFT_REPEAT, 0, 0, 2, Race::INVALID, 0, 1};
    // The Eye of Dalaran is event-driven: only a friendly no-type death
    // grants a random Tavern spell; typed deaths do not advance it.
    if (id == "BG30_MagicItem_981") return {TrinketEffect::AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL};
    // Goldenizer Supply arms a recurring end-of-turn grant every three turns.
    if (id == "BG30_MagicItem_435") return {TrinketEffect::END_TURN_FIXED_CARD, 0, 0, 3, Race::INVALID, 0, 1, false, false, false, "BG26_813t"};
    // Shaman Prayer Beads is a two-purchase lifetime cadence, not an aura.
    if (id == "BG30_MagicItem_982") return {TrinketEffect::AFTER_BUY_BATTLECRY_MINION, 0, 0, 2, Race::INVALID, 0, 1, false, true};
    // Fishy Sticker summons one golden Fish of N'Zoth at combat start.  The
    // golden token's deathrattle-copy lifecycle is resolved by Battle.cpp.
    if (id == "BG30_MagicItem_821t2") return {TrinketEffect::START_COMBAT_GOLDEN_FISH};
    // Surveyor Portrait grants the executable Hot-Air Surveyor token; its
    // hand Blood Gem +6/+6 payload is resolved at spell-cast time.
    if (id == "BG30_MagicItem_943") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG30_121"};
    // Booty Bay Brew buffs two friendly Pirates after each successful gold
    // spend.  The golden generated form doubles the stat payload.
    if (id == "BG30_MagicItem_924") return {TrinketEffect::SPEND_GOLD_PIRATE_STATS, 3, 3};
    if (id == "BG30_MagicItem_924t") return {TrinketEffect::SPEND_GOLD_PIRATE_STATS, 6, 6};
    // These portraits have a fixed immediate minion grant plus a distinct
    // trigger.  Keep dedicated effects so acquisition and trigger semantics
    // cannot be accidentally separated or counted as a plain portrait.
    if (id == "BG30_MagicItem_548") return {TrinketEffect::ACQUIRE_FIXED_GLOWSCALE, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_035"};
    if (id == "BG36_MagicItem_201") return {TrinketEffect::ACQUIRE_FIXED_LIONFISH, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG36_201"};
    if (id == "BG30_MagicItem_979") return {TrinketEffect::NEXT_TAVERN_SPELL_DISCOUNT, 0, 0, 1};
    if (id == "BG35_MagicItem_921") return {TrinketEffect::STAT_TAVERN_SPELL_DISCOUNT, 0, 0, 2};
    if (id == "BG30_MagicItem_986") return {TrinketEffect::FREE_TAVERN_SPELL_USES, 0, 0, 3};
    if (id == "BG32_MagicItem_823") return {TrinketEffect::START_TURN_GOLD_DAMAGE, 2, 0, 2};
    if (id == "BG35_MagicItem_150") return {TrinketEffect::REFRESH_TEMP_SHOP_STATS, 3, 3};
    if (id == "BG30_MagicItem_864") return {TrinketEffect::AVENGE_MINION_STATS, 4, 5, 3, Race::INVALID, 0, 1};
    if (id == "BG35_MagicItem_155") return {TrinketEffect::TAVERN_SPELL_TEMP_STATS_AFTER_DAMAGE, 1, 1};
    if (id == "BG30_MagicItem_923") return {TrinketEffect::PIRATE_ATTACK_GOLD, 0, 0, 1, Race::PIRATE, 0, 2};
    if (id == "BG30_MagicItem_925") return {TrinketEffect::ATTACKING_MINION_STATS, 4, 0};
    if (id == "BG30_MagicItem_995") return {TrinketEffect::START_COMBAT_HEALTH_FROM_ATTACK};
    if (id == "BG32_MagicItem_270") return {TrinketEffect::AVENGE_TAVERN_SPELL_ATTACK, 1, 0, 3};
    if (id == "BG30_MagicItem_973") return {TrinketEffect::REFRESH_EXTRA_SHOP_SLOTS, 0, 0, 2};
    if (id == "BG32_MagicItem_934") return {TrinketEffect::SPELL_COUNT_MINION_ATTACK, 1, 0, 4};
    if (id == "BG32_MagicItem_276") return {TrinketEffect::END_TURN_UNDEAD_ATTACK, 2, 0, 0, Race::UNDEAD};
    if (id == "BG35_MagicItem_814") return {TrinketEffect::REACH_TIER_GOLD, 0, 0, 12, Race::INVALID, 6};
    if (id == "BG32_MagicItem_428") return {TrinketEffect::DELAYED_GOLD, 0, 0, 10, Race::INVALID, 0, 2};
    if (id == "BG35_MagicItem_923") return {TrinketEffect::SPELL_CAST_MINION_STATS, 1, 1};
    if (id == "BG30_MagicItem_422") return {TrinketEffect::SPELL_CAST_MINION_STATS, 4, 4};
    if (id == "BG30_MagicItem_422t") return {TrinketEffect::SPELL_CAST_MINION_STATS, 10, 10};
    if (id == "BG30_MagicItem_886") return {TrinketEffect::SUMMON_DIVINE_SHIELD, 0, 0, 5};
    if (id == "BG30_MagicItem_917") return {TrinketEffect::START_COMBAT_NAGA_SPELLCRAFT};
    if (id == "BG30_MagicItem_919") return {TrinketEffect::AFTER_PLAY_NAGA_SPELLCRAFT};
    if (id == "BG30_MagicItem_978") return {TrinketEffect::SUMMON_MECH_RANDOM_DIVINE_SHIELD};
    if (id == "BG30_MagicItem_411") return {TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GEMS, 0, 0, 2};
    if (id == "BG30_MagicItem_540") return {TrinketEffect::SUMMON_BEAST_DOUBLE_ATTACK};
    if (id == "BG35_MagicItem_871") return {TrinketEffect::SUMMON_BEAST_STATS, 6, 6};
    // BG30_MagicItem_700 (Deathly Phylactery) is a Discover plus
    // first-deathrattle-doubling state machine.  It must remain fail-closed
    // until that modal/combat lifecycle is represented; it is not a Beast
    // summon trigger.
    if (id == "BG30_MagicItem_952") return {TrinketEffect::START_COMBAT_ELEMENTAL_FROSTLING};
    if (id == "BG35_MagicItem_701") return {TrinketEffect::START_COMBAT_BEAST_SCALING, 1, 1};
    if (id == "BG30_MagicItem_442") return {TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GOLEM};
    if (id == "BG30_MagicItem_902") return {TrinketEffect::START_COMBAT_EDGE_SHIELDS};
    if (id == "BG30_MagicItem_972") return {TrinketEffect::START_COMBAT_LEFT_COPY};
    if (id == "BG32_MagicItem_360") return {TrinketEffect::START_COMBAT_UNDEAD_EDGE_REBORN};
    if (id == "BG32_MagicItem_306") return {TrinketEffect::START_COMBAT_TRIGGER_DEATHRATTLES};
    if (id == "BG32_MagicItem_960") return {TrinketEffect::START_COMBAT_HIGHEST_HAND_MINION, 4, 4};
    if (id == "BG32_MagicItem_862") return {TrinketEffect::AFTER_DEATHRATTLE_RIGHTMOST_STATS, 2, 2};
    if (id == "BG32_MagicItem_862t") return {TrinketEffect::AFTER_DEATHRATTLE_RIGHTMOST_STATS, 6, 4};
    if (id == "BG30_MagicItem_403") return {TrinketEffect::START_COMBAT_NEUTRAL_TRIPLE};
    if (id == "BG30_MagicItem_542") return {TrinketEffect::START_COMBAT_DRAGON_MAX_ATTACK};
    if (id == "BG30_MagicItem_441") return {TrinketEffect::START_COMBAT_LEFTMOST_HAND_STATS};
    if (id == "BG30_MagicItem_962") return {TrinketEffect::START_COMBAT_LOWEST_ATTACK_DOUBLE};
    if (id == "BG35_MagicItem_702") return {TrinketEffect::START_COMBAT_LEFT_BEAST_SHIELDS};
    if (id == "BG32_MagicItem_419") return {TrinketEffect::START_COMBAT_HIGHEST_TIER_DRAGON_GOLDEN};
    if (id == "BG32_MagicItem_904") return {TrinketEffect::START_COMBAT_THREE_BLOOD_GEMS};
    if (id == "BG32_MagicItem_280") return {TrinketEffect::START_COMBAT_TYPE_STATS, 3, 2};
    // Vash'jir Anemone's improvement is derived from the authoritative
    // successful-spell counter at start of combat (+1 Health per four spells).
    if (id == "BG32_MagicItem_932") return {TrinketEffect::START_COMBAT_NAGA_HEALTH, 0, 1, 4, Race::NAGA};
    if (id == "BG35_MagicItem_711") return {TrinketEffect::START_COMBAT_RANDOM_PIRATE_SHIELDS, 0, 0, 4};
    if (id == "BG35_MagicItem_754") return {TrinketEffect::START_COMBAT_MURLOC_MAX_ATTACK};
    if (id == "BG36_MagicItem_213") return {TrinketEffect::START_COMBAT_RALLY_SHIELDS};
    if (id == "BG32_MagicItem_363") return {TrinketEffect::ATTACKING_DRAGON_DIVINE_SHIELD, 0, 0, 3};
    if (id == "BG36_MagicItem_361") return {TrinketEffect::START_COMBAT_NAGA_DOUBLE_STATS};
    if (id == "BG30_MagicItem_410") return {TrinketEffect::AVENGE_BLOOD_GEM_BONUS, 0, 1, 3};
    if (id == "BG30_MagicItem_410t2") return {TrinketEffect::AVENGE_BLOOD_GEM_BONUS, 1, 1, 4};
    if (id == "BG30_MagicItem_433t") return {TrinketEffect::FIRST_DEATH_MAX_STATS_RANDOM, 0, 0, 2};
    if (id == "BG32_MagicItem_270t") return {TrinketEffect::AVENGE_TAVERN_SPELL_ATTACK, 1, 1, 4};
    if (id == "BG35_MagicItem_864") return {TrinketEffect::AVENGE_MINION_STATS, 1, 1, 2};
    if (id == "BG35_MagicItem_864t") return {TrinketEffect::AVENGE_MINION_STATS, 4, 4, 2};
    if (id == "BG30_MagicItem_437") return {TrinketEffect::AVENGE_RANDOM_UNDEAD_REBORN, 0, 0, 5};
    if (id == "BG30_MagicItem_433") return {TrinketEffect::FIRST_DEATH_MAX_STATS_RANDOM};
    if (id == "BG30_MagicItem_545") return {TrinketEffect::AVENGE_RANDOM_MAGNETIC, 0, 0, 3};
    if (id == "BG30_MagicItem_546") return {TrinketEffect::AFTER_TWO_ATTACKS_QUILBOAR_GEM, 0, 0, 2};
    if (id == "BG32_MagicItem_930") return {TrinketEffect::SPELL_COUNT_RANDOM_NAGA, 0, 0, 7, Race::NAGA, 0, 1};
    if (id == "BG36_MagicItem_307") return {TrinketEffect::SPELL_COUNT_GOLD_ON_MINION, 0, 0, 3, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_281") return {TrinketEffect::TAVERN_SPELL_NO_TYPE_STATS, 4, 4};
    if (id == "BG32_MagicItem_279") return {TrinketEffect::BLOOD_GEM_DIVINE_SHIELD};
    if (id == "BG32_MagicItem_200") return {TrinketEffect::ATTACKING_BEAST_SCALING, 2};
    if (id == "BG32_MagicItem_284") return {TrinketEffect::END_TURN_BLOOD_GEMS_PER_TYPE, 7};
    if (id == "BG32_MagicItem_111") return {TrinketEffect::END_TURN_MURLOC_STATS, 8, 8};
    if (id == "BG32_MagicItem_170") return {TrinketEffect::AFTER_MAGNETIC_MECH_REPAIR, 4, 8};
    if (id == "BG32_MagicItem_364") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_314"};
    if (id == "BG36_MagicItem_302") return {TrinketEffect::END_TURN_MINION_STATS, 2, 1};
    if (id == "BG36_MagicItem_302t") return {TrinketEffect::END_TURN_MINION_STATS, 4, 2};
    if (id == "BG36_MagicItem_371") return {TrinketEffect::TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST, 1, 1};
    if (id == "BG36_MagicItem_200") return {TrinketEffect::RALLY_ATTACK_FREE_REFRESH};
    if (id == "BG36_MagicItem_203") return {TrinketEffect::BATTLECRY_EDGE_STATS, 5, 5};
    if (id == "BG36_MagicItem_212") return {TrinketEffect::END_TURN_LEFT_DEATHRATTLES};
    if (id == "BG36_MagicItem_214") return {TrinketEffect::END_TURN_RALLY_TRIGGERS};
    // Scraper Sticker: one random Magnetic Mech immediately and one more at
    // each subsequent recruit turn.  Keep this on the existing random-card
    // path so pool filtering and hand-cap behavior remain authoritative.
    if (id == "BG35_MagicItem_301") return {
        TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::MECHANICAL,
        0, 1, true, false, true};
    if (id == "BG30_MagicItem_710") return {
        TrinketEffect::AFTER_SELL_RANDOM_MINION, 0, 0, 5, Race::MURLOC};
    if (id == "BG30_MagicItem_951") return {
        TrinketEffect::AFTER_SELL_RANDOM_MINION, 0, 0, 6, Race::ELEMENTAL};
    if (id == "BG30_MagicItem_713") return {
        TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION, 0, 0, 8, Race::UNDEAD};
    if (id == "BG30_MagicItem_931") return {
        TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION, 0, 0, 7, Race::BEAST};
    if (id == "BG35_MagicItem_302") return {
        TrinketEffect::AFTER_FRIENDLY_DEATH_RANDOM_MINION, 0, 0, 8, Race::MECHANICAL};
    if (id == "BG30_MagicItem_301") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG25_008"};
    if (id == "BG30_MagicItem_303") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_TTN_401"};
    if (id == "BG30_MagicItem_310") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG25_354"};
    if (id == "BG30_MagicItem_406") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG28_604"};
    if (id == "BG30_MagicItem_821") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "LT23_809H"};
    if (id == "BG30_MagicItem_876") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_EX1_564"};
    if (id == "BG30_MagicItem_831") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BGS_115"};
    if (id == "BG30_MagicItem_944") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG28_169"};
    if (id == "BG30_MagicItem_987") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG26_814"};
    // Portraits whose acquisition is a single, canonical minion already
    // present in the supported Battlegrounds card registry.  Keep these on
    // the fixed-card path: the portrait's additional aura text is handled by
    // its generated minion definition, not by a guessed Trinket aura.
    if (id == "BG30_MagicItem_431") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG28_707", PortraitEffect::LIVING_AZERITE_ELEMENTAL_STATS, false};
    if (id == "BG30_MagicItem_432") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_888", PortraitEffect::BELCHER_VENOMOUS_LOSS_STATS, false};
    if (id == "BG30_MagicItem_555") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_175", PortraitEffect::SURPRISE_MORE_ELEMENTALS, false};
    if (id == "BG30_MagicItem_803") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_BOT_606", PortraitEffect::KABOOM_BOT_DEATHRATTLE_DAMAGE, false};
    if (id == "BG30_MagicItem_825") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG21_013", PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON, false};
    if (id == "BG30_MagicItem_828") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_505", PortraitEffect::ZESTY_SHAKER_EXTRA_COPY, false};
    if (id == "BG30_MagicItem_869") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG29_873", PortraitEffect::FELBLOOD_BOTH_STATS, false};
    if (id == "BG30_MagicItem_971") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BGS_009", PortraitEffect::LIGHTFANG_ALL_TYPES, false};
    if (id == "BG32_MagicItem_274") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_157", PortraitEffect::BRISTLEBACH_ALL_MINIONS, false};
    if (id == "BG32_MagicItem_301") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_350", PortraitEffect::BASSGILL_SUMMON_DIVINE_SHIELD, false};
    if (id == "BG32_MagicItem_804") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_OG_221", PortraitEffect::SELFLESS_BATTLECRY, false};
    if (id == "BG32_MagicItem_831") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BGS_115", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_741") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_149", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_742") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_147", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_924") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_035", PortraitEffect::GROUNDBREAKER_ADJACENT_STATS, false};
    if (id == "BG32_MagicItem_700") return {TrinketEffect::TAVERN_SPELL_GROWING_STATS, 1, 1};
    if (id == "BG32_MagicItem_801") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 1};
    if (id == "BG32_MagicItem_801t") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 1};
    if (id == "BG36_MagicItem_373") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 2};
    return {};
}

void TrinketBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    cards.emplace("BG30_MagicItem_416", CardDef{});
    cards.emplace("BG30_MagicItem_996", CardDef{});
    cards.emplace("BG30_MagicItem_998", CardDef{});
    cards.emplace("BG30_MagicItem_841", CardDef{});
    cards.emplace("BG36_MagicItem_220", CardDef{});
    cards.emplace("BG32_MagicItem_858", CardDef{});
    cards.emplace("BG30_MagicItem_993", CardDef{});
    cards.emplace("BG30_MagicItem_430", CardDef{});
    cards.emplace("BG32_MagicItem_172", CardDef{});
    cards.emplace("BG30_MagicItem_543", CardDef{});
    cards.emplace("BG35_MagicItem_303", CardDef{});
    cards.emplace("BG30_MagicItem_847", CardDef{});
    cards.emplace("BG32_MagicItem_231", CardDef{});
    cards.emplace("BG32_MagicItem_231t", CardDef{});
    cards.emplace("BG30_MagicItem_914", CardDef{});
    cards.emplace("BG30_MagicItem_914t", CardDef{});
    cards.emplace("BG30_MagicItem_544", CardDef{});
    cards.emplace("BG30_MagicItem_544t", CardDef{});
    cards.emplace("BG36_MagicItem_800", CardDef{});
    cards.emplace("BG35_MagicItem_710", CardDef{});
    cards.emplace("BG30_MagicItem_414", CardDef{});
    cards.emplace("BG30_MagicItem_414t", CardDef{});
    cards.emplace("BG30_MagicItem_984", CardDef{});
    cards.emplace("BG30_MagicItem_984t", CardDef{});
    cards.emplace("BG30_MagicItem_900", CardDef{});
    cards.emplace("BG30_MagicItem_900t", CardDef{});
    cards.emplace("BG30_MagicItem_989", CardDef{});
    cards.emplace("BG30_MagicItem_989t", CardDef{});
    cards.emplace("BG36_MagicItem_205", CardDef{});
    cards.emplace("BG36_MagicItem_215", CardDef{});
    cards.emplace("BG36_MagicItem_811", CardDef{});
    cards.emplace("BG36_MagicItem_202", CardDef{});
    cards.emplace("BG30_MagicItem_301", CardDef{});
    cards.emplace("BG30_MagicItem_303", CardDef{});
    cards.emplace("BG30_MagicItem_310", CardDef{});
    cards.emplace("BG30_MagicItem_406", CardDef{});
    cards.emplace("BG30_MagicItem_821", CardDef{});
    cards.emplace("BG30_MagicItem_876", CardDef{});
    cards.emplace("BG30_MagicItem_831", CardDef{});
    cards.emplace("BG30_MagicItem_944", CardDef{});
    cards.emplace("BG30_MagicItem_987", CardDef{});
    cards.emplace("BG30_MagicItem_431", CardDef{});
    cards.emplace("BG30_MagicItem_432", CardDef{});
    cards.emplace("BG30_MagicItem_555", CardDef{});
    cards.emplace("BG30_MagicItem_803", CardDef{});
    cards.emplace("BG30_MagicItem_825", CardDef{});
    cards.emplace("BG30_MagicItem_828", CardDef{});
    cards.emplace("BG30_MagicItem_869", CardDef{});
    cards.emplace("BG30_MagicItem_971", CardDef{});
    cards.emplace("BG32_MagicItem_274", CardDef{});
    cards.emplace("BG32_MagicItem_301", CardDef{});
    cards.emplace("BG32_MagicItem_804", CardDef{});
    cards.emplace("BG32_MagicItem_831", CardDef{});
    cards.emplace("BG35_MagicItem_741", CardDef{});
    cards.emplace("BG35_MagicItem_742", CardDef{});
    cards.emplace("BG35_MagicItem_924", CardDef{});
    cards.emplace("BG32_MagicItem_700", CardDef{});
    cards.emplace("BG32_MagicItem_801", CardDef{});
    cards.emplace("BG32_MagicItem_801t", CardDef{});
    cards.emplace("BG36_MagicItem_373", CardDef{});
    cards.emplace("BG30_MagicItem_879", CardDef{});
    cards.emplace("BG30_MagicItem_879t", CardDef{});
    cards.emplace("BG30_MagicItem_541", CardDef{});
    cards.emplace("BG30_MagicItem_423", CardDef{});
    cards.emplace("BG30_MagicItem_880", CardDef{});
    cards.emplace("BG30_MagicItem_880t", CardDef{});
    cards.emplace("BG30_MagicItem_988", CardDef{});
    cards.emplace("BG30_MagicItem_988t", CardDef{});
    cards.emplace("BG30_MagicItem_970", CardDef{});
    cards.emplace("BG30_MagicItem_970t", CardDef{});
    cards.emplace("BG30_MagicItem_843t", CardDef{});
    cards.emplace("BG30_MagicItem_547", CardDef{});
    cards.emplace("BG30_MagicItem_547t", CardDef{});
    cards.emplace("BG35_MagicItem_151", CardDef{});
    cards.emplace("BG35_MagicItem_151t", CardDef{});
    cards.emplace("BG30_MagicItem_992", CardDef{});
    cards.emplace("BG30_MagicItem_705", CardDef{});
    cards.emplace("BG30_MagicItem_402", CardDef{});
    cards.emplace("BG30_MagicItem_407", CardDef{});
    cards.emplace("BG30_MagicItem_418", CardDef{});
    cards.emplace("BG30_MagicItem_992t", CardDef{});
    cards.emplace("BG30_MagicItem_425", CardDef{});
    cards.emplace("BG30_MagicItem_419", CardDef{});
    cards.emplace("BG30_MagicItem_426", CardDef{});
    cards.emplace("BG30_MagicItem_426t", CardDef{});
    cards.emplace("BG30_MagicItem_706", CardDef{});
    cards.emplace("BG30_MagicItem_916", CardDef{});
    cards.emplace("BG30_MagicItem_942", CardDef{});
    cards.emplace("BG30_MagicItem_930", CardDef{});
    cards.emplace("BG30_MagicItem_434", CardDef{});
    cards.emplace("BG30_MagicItem_920", CardDef{});
    cards.emplace("BG30_MagicItem_981", CardDef{});
    cards.emplace("BG30_MagicItem_435", CardDef{});
    cards.emplace("BG30_MagicItem_982", CardDef{});
    cards.emplace("BG30_MagicItem_821t2", CardDef{});
    cards.emplace("BG30_MagicItem_943", CardDef{});
    cards.emplace("BG30_MagicItem_924", CardDef{});
    cards.emplace("BG30_MagicItem_924t", CardDef{});
    cards.emplace("BG30_MagicItem_548", CardDef{});
    cards.emplace("BG36_MagicItem_201", CardDef{});
    cards.emplace("BG30_MagicItem_979", CardDef{});
    cards.emplace("BG35_MagicItem_921", CardDef{});
    cards.emplace("BG30_MagicItem_986", CardDef{});
    cards.emplace("BG32_MagicItem_823", CardDef{});
    cards.emplace("BG35_MagicItem_150", CardDef{});
    cards.emplace("BG30_MagicItem_864", CardDef{});
    cards.emplace("BG35_MagicItem_155", CardDef{});
    cards.emplace("BG30_MagicItem_923", CardDef{});
    cards.emplace("BG30_MagicItem_925", CardDef{});
    cards.emplace("BG30_MagicItem_995", CardDef{});
    cards.emplace("BG32_MagicItem_270", CardDef{});
    cards.emplace("BG30_MagicItem_973", CardDef{});
    cards.emplace("BG32_MagicItem_934", CardDef{});
    cards.emplace("BG32_MagicItem_276", CardDef{});
    cards.emplace("BG35_MagicItem_814", CardDef{});
    cards.emplace("BG32_MagicItem_428", CardDef{});
    cards.emplace("BG35_MagicItem_923", CardDef{});
    cards.emplace("BG30_MagicItem_422", CardDef{});
    cards.emplace("BG30_MagicItem_422t", CardDef{});
    cards.emplace("BG30_MagicItem_886", CardDef{});
    cards.emplace("BG30_MagicItem_886e", CardDef{});
    cards.emplace("BG30_MagicItem_917", CardDef{});
    cards.emplace("BG30_MagicItem_917e", CardDef{});
    cards.emplace("BG30_MagicItem_919", CardDef{});
    cards.emplace("BG30_MagicItem_978", CardDef{});
    cards.emplace("BG30_MagicItem_978e", CardDef{});
    cards.emplace("BG30_MagicItem_411", CardDef{});
    cards.emplace("BG30_MagicItem_411e", CardDef{});
    cards.emplace("BG30_MagicItem_540", CardDef{});
    cards.emplace("BG30_MagicItem_540e", CardDef{});
    cards.emplace("BG35_MagicItem_871", CardDef{});
    cards.emplace("BG30_MagicItem_952", CardDef{});
    cards.emplace("BG30_MagicItem_952e", CardDef{});
    cards.emplace("BG30_MagicItem_442", CardDef{});
    cards.emplace("BG30_MagicItem_902", CardDef{});
    cards.emplace("BG30_MagicItem_972", CardDef{});
    cards.emplace("BG32_MagicItem_360", CardDef{});
    cards.emplace("BG32_MagicItem_306", CardDef{});
    cards.emplace("BG32_MagicItem_960", CardDef{});
    cards.emplace("BG32_MagicItem_862", CardDef{});
    cards.emplace("BG32_MagicItem_862t", CardDef{});
    cards.emplace("BG30_MagicItem_403", CardDef{});
    cards.emplace("BG30_MagicItem_542", CardDef{});
    cards.emplace("BG30_MagicItem_441", CardDef{});
    cards.emplace("BG30_MagicItem_962", CardDef{});
    cards.emplace("BG35_MagicItem_702", CardDef{});
    cards.emplace("BG32_MagicItem_419", CardDef{});
    cards.emplace("BG32_MagicItem_904", CardDef{});
    cards.emplace("BG32_MagicItem_280", CardDef{});
    cards.emplace("BG35_MagicItem_711", CardDef{});
    cards.emplace("BG35_MagicItem_754", CardDef{});
    cards.emplace("BG36_MagicItem_213", CardDef{});
    cards.emplace("BG30_MagicItem_410", CardDef{});
    cards.emplace("BG30_MagicItem_410t2", CardDef{});
    cards.emplace("BG30_MagicItem_433t", CardDef{});
    cards.emplace("BG32_MagicItem_270t", CardDef{});
    cards.emplace("BG35_MagicItem_864", CardDef{});
    cards.emplace("BG35_MagicItem_864t", CardDef{});
    cards.emplace("BG30_MagicItem_437", CardDef{});
    cards.emplace("BG30_MagicItem_433", CardDef{});
    cards.emplace("BG30_MagicItem_545", CardDef{});
    cards.emplace("BG30_MagicItem_546", CardDef{});
    cards.emplace("BG36_MagicItem_361", CardDef{});
    cards.emplace("BG36_MagicItem_302", CardDef{});
    cards.emplace("BG36_MagicItem_302t", CardDef{});
    cards.emplace("BG36_MagicItem_371", CardDef{});
    cards.emplace("BG36_MagicItem_200", CardDef{});
    cards.emplace("BG36_MagicItem_203", CardDef{});
    cards.emplace("BG36_MagicItem_212", CardDef{});
    cards.emplace("BG36_MagicItem_214", CardDef{});
    cards.emplace("BG35_MagicItem_301", CardDef{});
    cards.emplace("BG30_MagicItem_710", CardDef{});
    cards.emplace("BG30_MagicItem_951", CardDef{});
    cards.emplace("BG30_MagicItem_713", CardDef{});
    cards.emplace("BG30_MagicItem_931", CardDef{});
    cards.emplace("BG35_MagicItem_302", CardDef{});
    cards.emplace("BG32_MagicItem_930", CardDef{});
    cards.emplace("BG32_MagicItem_932", CardDef{});
    cards.emplace("BG36_MagicItem_307", CardDef{});
    cards.emplace("BG32_MagicItem_281", CardDef{});
    cards.emplace("BG32_MagicItem_279", CardDef{});
    cards.emplace("BG32_MagicItem_363", CardDef{});
    cards.emplace("BG32_MagicItem_200", CardDef{});
    cards.emplace("BG32_MagicItem_284", CardDef{});
    cards.emplace("BG32_MagicItem_111", CardDef{});
    cards.emplace("BG32_MagicItem_170", CardDef{});
    cards.emplace("BG32_MagicItem_364", CardDef{});
    cards.emplace("BG30_MagicItem_705", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
