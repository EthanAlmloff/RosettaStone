#include <Rosetta/Battlegrounds/CardSets/TrinketBehaviors.hpp>

namespace RosettaStone::Battlegrounds
{
TrinketBehavior FindTrinketBehavior(std::string_view id) noexcept
{
    if (id == "BG30_MagicItem_416") return {TrinketEffect::SPELLCRAFT_TRANSFORM_HIGHER_TIER};
    // Putricide Sticker opens two sequential, tier-banded Undead component
    // Discovers.  The Player modal composes the selected components into a
    // fresh executable Creation; this marker is never treated as a generic
    // random Undead reward.
    if (id == "BG32_MagicItem_300") return {TrinketEffect::PUTRICIDE_STICKER};
    // These Patch 36.4 descriptors intentionally retain the printed card
    // identity instead of collapsing bespoke Discover/cadence effects into
    // a broader random-minion marker.  Their event executors consume the
    // typed effect and the payload fields below.
    // Putricide Sticker (BG32_MagicItem_300) says "Craft a custom Undead".
    // Player owns the pinned two-stage modal and materializes a fresh
    // non-golden custom minion after the second component is committed.  Do
    // not collapse this into a generic Undead Discover.
    if (id == "BG32_MagicItem_361") return {TrinketEffect::PORTABLE_FACTORY, 0, 0, 4, Race::INVALID, 4, 1};
    if (id == "BG32_MagicItem_361t") return {TrinketEffect::PORTABLE_FACTORY, 0, 0, 5, Race::INVALID, 5, 1};
    // Yogg-Tastic Pastry uses the locally pinned current 36.4 wheel payload.
    // The executor owns the six weighted outcomes; this registry row only
    // identifies the repeatable lifecycle trigger.
    if (id == "BG30_MagicItem_994") return {TrinketEffect::YOGG_WHEEL};
    if (id == "BG32_MagicItem_415") return {TrinketEffect::BATTLE_HORN, 0, 0, 2, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_417") return {TrinketEffect::TARECGOSA_STICKER, 0, 0, 0, Race::DRAGON};
    // Demonblood Gourd and Floating Candle Set emit Spellcraft tokens whose
    // complete target executors already live in TavernSpellBehaviors.hpp.
    // The typed markers keep acquisition separate from token resolution.
    if (id == "BG30_MagicItem_429") return {TrinketEffect::SPELLCRAFT_CONSUME_SHOP_STATS};
    // Vibrant Bubble supplies one BG32_MagicItem_892t Spellcraft token at
    // recruit start.  The token's TavernSpellBehavior owns the exact
    // Murloc-only random-missing-keyword resolution.
    if (id == "BG32_MagicItem_892") return {TrinketEffect::SPELLCRAFT_MURLOC_KEYWORD};
    if (id == "BG35_MagicItem_872") return {TrinketEffect::SPELLCRAFT_OPHIDIAN_STAFF};
    if (id == "BG36_MagicItem_208") return {TrinketEffect::SPELLCRAFT_TRIGGER_DEATHRATTLE};
    if (id == "BG35_MagicItem_755") return {TrinketEffect::SPELLCRAFT_CHILLMERE_MOSAIC};
    // Ancient Wishbone duplicates the resolved effect of one active Hero
    // Power activation. The bridge executor owns the multiplier; this marker
    // must never expand CanUseHeroPower or UseHeroPower.
    if (id == "BG30_MagicItem_804") return {TrinketEffect::HERO_POWER_TWICE};
    // Countdown system cards are public shop-opening markers.  They are
    // never offered as owned Trinkets; keeping the exact printed timers in
    // the typed registry prevents them from being mistaken for metadata-only
    // entities by coverage and card-definition audits.
    if (id == "BG30_Trinket_1st") return {TrinketEffect::TRINKET_SHOP_TIMER, 0, 0, 5};
    if (id == "BG30_Trinket_2nd") return {TrinketEffect::TRINKET_SHOP_TIMER, 0, 0, 8};
    // Sous Chef Sticker grants one additional Hero Power use each recruit
    // turn.  The shared marker is consumed by the recruit-start refresh.
    if (id == "BG35_MagicItem_801") return {TrinketEffect::HERO_POWER_EXTRA_USE};
    // Only effects whose complete acquisition and lifecycle semantics are
    // implemented are registered here.  Nether Pendant and both Cheese
    // Wheel forms improve on a counter; Innkeeper's Stein and Minion Bait
    // trigger on refresh; Goblin Wallet triggers at end of turn.  Treating
    // any of those as an unconditional aura/bonus would silently change the
    // game, so they remain fail-closed until their trigger state is modeled.
    if (id == "BG30_MagicItem_996") return {TrinketEffect::GOLD_AND_MAX_GOLD, 0, 0, 4};
    if (id == "BG30_MagicItem_998") return {TrinketEffect::IMMEDIATE_GOLD, 0, 0, 2};
    // Safety Patch casts the pinned Battlegrounds Ice Block secret and grants
    // five Gold immediately.  Lethal prevention is consumed by Hero::TakeDamage.
    if (id == "BG35_MagicItem_820") return {TrinketEffect::SAFETY_PATCH, 0, 0, 5};
    // Fortune Teller's Pearl grants its immediate two-Gold payload when
    // acquired; the generated reward is intentionally represented by the
    // same typed economy effect as Gold Coin Trinkets.
    if (id == "BG32_MagicItem_271") return {TrinketEffect::IMMEDIATE_GOLD, 0, 0, 2};
    if (id == "BG35_MagicItem_818") return {TrinketEffect::IMMEDIATE_GOLD_AND_LESSER_NEXT, 0, 0, 10};
    // Arcane Behemoth Portrait grants its canonical minion immediately.
    // Morgl Portrait likewise grants the canonical Tide Oracle entity.
    if (id == "BG32_MagicItem_926") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG27_513"};
    // Grifter Portrait grants the canonical Doubloon Grifter and makes the
    // first Pirate purchase each recruit turn free.  The purchase entitlement
    // is tracked per owned Trinket instance in Player::PurchaseMinion.
    if (id == "BG32_MagicItem_957") return {TrinketEffect::ACQUIRE_FIXED_CARD_FIRST_PIRATE_FREE, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_826"};
    if (id == "BG30_MagicItem_841") return {TrinketEffect::SHOP_STATS_AND_TAVERN_SLOTS, 3, 3, 7};
    if (id == "BG36_MagicItem_220") return {TrinketEffect::START_TURN_GOLD_PER_MINION_TYPE};
    // Ominous Stone opens one Tier-4 minion of the warband's most common
    // type, paired with a legal executable Dark Gift.  The target type and
    // gift are selected by the public modal at acquisition time.
    if (id == "BG36_MagicItem_206") return {TrinketEffect::OMINOUS_STONE_DISCOVER};
    if (id == "BG36_MagicItem_301") return {TrinketEffect::LOCKBOX_PORTRAIT};
    if (id == "BG36_MagicItem_309") return {TrinketEffect::WAX_LANCE_DISCOVER};
    if (id == "BG36_MagicItem_370") return {TrinketEffect::MALDRAXXUS_DAGGER_DISCOVER};
    // Lavish Cape casts one random legal Tavern spell for each distinct
    // friendly minion type, on acquisition and at every recruit start.
    if (id == "BG32_MagicItem_286") return {TrinketEffect::START_TURN_RANDOM_TAVERN_SPELLS_PER_TYPE};
    if (id == "BG32_MagicItem_858") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 4, 3};
    // Magician's Top Hat gives two random minions from each of Tiers 1, 2,
    // and 3 immediately.  Keep this as a typed batch rather than three
    // generic rewards so the acquisition boundary remains discoverable and
    // the exact tier partition cannot drift.
    if (id == "BG35_MagicItem_815") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS_TIER_BATCH};
    if (id == "BG30_MagicItem_993") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 7, 1, true};
    if (id == "BG30_MagicItem_430") return {TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 1, true, true, false};
    // Felbat Portrait has two independent acquisition-time effects: grant
    // the canonical Famished Felbat and make Bob's Tavern seven offers.
    // Keep both in one typed descriptor so neither half is silently lost.
    if (id == "BG30_MagicItem_991") return {TrinketEffect::ACQUIRE_FIXED_CARD_AND_TAVERN_SLOTS, 0, 0, 7, Race::INVALID, 0, 1, false, false, false, "BG21_005"};
    if (id == "BG32_MagicItem_172") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG31_176"};
    if (id == "BG30_MagicItem_543") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "EBG_Spell_032"};
    if (id == "BG35_MagicItem_303") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_072"};
    // Wisdomball Supply grants one Knockoff Wisdomball on acquisition and
    // repeats the same fixed card at every recruit start.  Keep this on the
    // validated fixed-card path so hand-cap and card-type checks remain
    // authoritative in Player::AcquireTrinket/GrantTrinketStartTurnCards.
    if (id == "BG31_MagicItem_903") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG30_802"};
    if (id == "BG35_MagicItem_305") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG28_601", PortraitEffect::NONE, false, 0, 2};
    if (id == "BG35_MagicItem_817") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG35_MagicItem_817t", PortraitEffect::NONE, false, 0, 2};
    // Horn of Summoning and Turbocharged Drill promise different minions;
    // use the shared filtered-pool path with without-replacement draws so the
    // hand never contains duplicate IDs from a single reward.
    if (id == "BG32_MagicItem_282") return {
        TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 0, 5,
        false, false, true, {}, PortraitEffect::NONE, false, 0, 1, {}, true};
    if (id == "BG32_MagicItem_304") return {
        TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::INVALID, 1, 6,
        false, false, false, {}, PortraitEffect::NONE, false, 0, 1, {}, true};
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
    // Felsteel Cleaver consumes the Tavern minion that was just targeted by
    // a spell, then transfers its final stats to a random friendly minion.
    // The target entity is snapshotted by Season14State before this hook.
    if (id == "BG36_MagicItem_831") return {TrinketEffect::AFTER_SPELL_ON_SHOP_CONSUME};
    if (id == "BG35_MagicItem_710") return {TrinketEffect::AFTER_TAVERN_SPELL_RACE_BUFF, 2, 2, 0, Race::PIRATE};
    if (id == "BG30_MagicItem_414") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 2, 1, 2};
    if (id == "BG30_MagicItem_414t") return {TrinketEffect::AFTER_BUY_RANDOM_FRIENDLY_BUFF, 4, 4, 2};
    if (id == "BG32_MagicItem_278") return {TrinketEffect::AFTER_BUY_MINION_MAGNETIC_SATELLITE};
    // Secret Schematic: each successfully purchased Mech adds one random
    // Tavern spell to hand.  `race` is the purchase filter and `amount` is
    // the number of spells per qualifying purchase; no cadence/usage cap is
    // implied by the printed text.
    if (id == "BG36_MagicItem_840") return {
        TrinketEffect::AFTER_BUY_MECH_RANDOM_SPELL, 0, 0, 0,
        Race::MECHANICAL, 0, 1};
    if (id == "BG30_MagicItem_984") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 3, 0};
    if (id == "BG30_MagicItem_984t") return {TrinketEffect::END_TURN_DIVINE_SHIELD_ATTACK, 7, 0};
    if (id == "BG30_MagicItem_900") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 4, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_900t") return {TrinketEffect::AFTER_PLAY_CARD_RANDOM_RACE_BUFF, 6, 4, 1, Race::DRAGON};
    if (id == "BG30_MagicItem_989") return {TrinketEffect::STATIC_RACE_STATS, 3, 0, 0, Race::UNDEAD};
    if (id == "BG30_MagicItem_989t") return {TrinketEffect::STATIC_RACE_STATS, 15, 0, 0, Race::UNDEAD};
    if (id == "BG32_MagicItem_204") return {TrinketEffect::AFTER_OUTSIDE_COMBAT_DESTROY_STATS, 2, 2};
    if (id == "BG32_MagicItem_205") return {TrinketEffect::AFTER_OUTSIDE_COMBAT_DESTROY_GOLD, 0, 0, 3};
    if (id == "BG36_MagicItem_205") return {TrinketEffect::AFTER_REBORN_STATS, 2, 2};
    // Funeral Wreath adds a plain copy of a friendly Reborn minion, up to
    // three times per combat.  The combat reset clears triggerProgress;
    // Player owns the hand-cap retry boundary.
    if (id == "BG36_MagicItem_217") return {TrinketEffect::AFTER_REBORN_COPY, 0, 0, 3};
    if (id == "BG35_MagicItem_731") return {TrinketEffect::AFTER_REBORN_UNDEAD_REBORN, 0, 0, 3};
    // S'Thara Sticker returns the first Demon that died this combat, with
    // its maximum (not combat-damaged) stats, after the owner's last minion
    // death.  The death executor owns the empty-board boundary.
    if (id == "BG32_MagicItem_907") return {TrinketEffect::AFTER_LAST_FRIENDLY_DEATH_DEMON, 0, 0, 1, Race::DEMON};
    if (id == "BG36_MagicItem_215") return {TrinketEffect::DUPLICATE_DRAGON_BATTLECRY};
    if (id == "BG36_MagicItem_811") return {TrinketEffect::FIRST_MINION_DIVINE_SHIELD};
    if (id == "BG36_MagicItem_202") return {TrinketEffect::BATTLECRY_BUY_DISCOUNT, 0, 0, 2};
    // Season 14's refresh/self-damage Trinkets retain their progress in the
    // player-owned state.  `value` is the trigger threshold and `amount` is
    // the permanent +A/+H improvement; the initial aura is attack/health.
    if (id == "BG30_MagicItem_879") return {TrinketEffect::REFRESH_SHOP_STATS, 1, 1, 4, Race::INVALID, 0, 1};
    if (id == "BG30_MagicItem_879t") return {TrinketEffect::REFRESH_SHOP_STATS, 2, 2, 4, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_891") return {TrinketEffect::REFRESH_MURLOC_SHOP_STATS, 5, 5};
    if (id == "BG32_MagicItem_935") return {TrinketEffect::REFRESH_EXTRA_TAVERN_SPELL, 0, 0, 1};
    // Warband Whistle is an acquisition-time, one-shot free refresh.  Its
    // shop replacement is performed by Player::RefreshTavern so normal
    // refresh payment/pool lifecycle remains shared.
    if (id == "BG35_MagicItem_930") return {TrinketEffect::WARBAND_COPY_REFRESH};
    if (id == "BG35_MagicItem_852") return {TrinketEffect::REFRESH_HIGHEST_ATTACK_TO_LOWEST_STATS};
    if (id == "BG30_MagicItem_541") return {TrinketEffect::HERO_DAMAGE_SHOP_STATS, 2, 2, 3, Race::INVALID, 0, 1};
    if (id == "BG30_MagicItem_423") return {TrinketEffect::HIGHER_TIER_REFRESH};
    // Guiding Candle makes only the first two successful refreshes each
    // recruit turn contain Tier 6 minions.  `value` is the per-turn cap;
    // Season14State tracks the consumed refreshes on this Trinket instance.
    // Guiding Candle is stricter than Innkeeper's Stein: for its first two
    // successful refreshes each recruit turn, every newly generated offer is
    // Tier 6 rather than merely adding one next-tier offer.
    if (id == "BG32_MagicItem_366") return {TrinketEffect::TIER_SIX_ONLY_REFRESH, 0, 0, 2};
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
    // Bloodfury Shield arms one Fodder for the next successful Tavern
    // refresh whenever its owner casts a Tavern spell.  The pending offer
    // lives in Season14State so it survives refresh preparation and is
    // consumed exactly once at the refresh boundary.
    if (id == "BG36_MagicItem_830") return {TrinketEffect::ARM_FODDER_REFRESH};
    // Copper Coil buffs the minion that was just magnetized.  `attack` and
    // `health` are the first-trigger payload; `value` is the per-trigger
    // improvement retained in the owned Trinket's triggerProgress counter.
    if (id == "BG35_MagicItem_300") return {TrinketEffect::AFTER_MAGNETIZE_STATS, 1, 1, 1};
    if (id == "BG35_MagicItem_300t") return {TrinketEffect::AFTER_MAGNETIZE_STATS, 3, 2, 1};
    // Electromagnetic Device keeps its magnetization aura on the same
    // target-local path as Copper Coil. Discover acquisition is a separate
    // modal concern; this descriptor credits the executable trigger.
    if (id == "BG30_MagicItem_709") return {TrinketEffect::AFTER_MAGNETIZE_STATS, 3, 3};
    if (id == "BG30_MagicItem_709t") return {TrinketEffect::AFTER_MAGNETIZE_STATS, 4, 4};
    // Tiger Carving's damage listener is player-owned and excludes the
    // damaged minion from the random permanent recipient.
    if (id == "BG30_MagicItem_427") return {TrinketEffect::AFTER_FRIENDLY_DAMAGE_STATS, 3, 1};
    if (id == "BG30_MagicItem_427t") return {TrinketEffect::AFTER_FRIENDLY_DAMAGE_STATS, 6, 2};
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
    // Lovely Locket mirrors every successful spell cast on a friendly
    // minion onto another friendly minion.  The cast resolver selects the
    // secondary target at resolution time, so this remains target-safe when
    // the original spell changes the board.
    if (id == "BG36_MagicItem_211") return {TrinketEffect::AFTER_FRIENDLY_SPELL_REPEAT};
    // Spitescale Sushi Roll repeats the first two Spellcraft casts each
    // recruit turn.  The per-turn counter lives on this Trinket instance.
    if (id == "BG30_MagicItem_920") return {TrinketEffect::SPELLCRAFT_REPEAT, 0, 0, 2, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_931") return {TrinketEffect::START_TURN_RANDOM_SPELLCRAFT, 0, 0, 0, Race::INVALID, 0, 3, true};
    // The Eye of Dalaran is event-driven: only a friendly no-type death
    // grants a random Tavern spell; typed deaths do not advance it.
    if (id == "BG30_MagicItem_981") return {TrinketEffect::AFTER_FRIENDLY_NO_TYPE_DEATH_RANDOM_SPELL};
    // Mug of the Sire buffs the warband whenever a friendly summon would not
    // fit. The failed-attempt boundary is handled by Player/SummonTask so a
    // successful summon never consumes this trigger.
    if (id == "BG30_MagicItem_438t") return {
        TrinketEffect::AFTER_SUMMON_OVERFLOW_STATS, 5, 0};
    // Divine Signet observes every friendly Divine Shield loss during combat
    // and grants at most four random Tavern spells per combat.
    if (id == "BG32_MagicItem_171") return {TrinketEffect::AFTER_DIVINE_SHIELD_LOST_RANDOM_SPELL, 0, 0, 4};
    // Bloodbound Ring replays a hand-cast Blood Gem onto every friendly
    // minion that still has Divine Shield.  ApplySpellBoardEffect owns the
    // post-cast boundary so generated/free Blood Gems do not recurse here.
    if (id == "BG35_MagicItem_435") return {TrinketEffect::AFTER_BLOOD_GEM_DIVINE_SHIELD};
    // Ghastly Sticker participates in the same final recruit end-of-turn
    // pass as other TURN_END effects, without duplicating bespoke end-turn
    // Trinket handlers.
    if (id == "BG32_MagicItem_367") return {TrinketEffect::END_TURN_EXTRA_TRIGGER};
    if (id == "BG35_MagicItem_752") return {TrinketEffect::END_TURN_BATTLECRY_TRIGGER};
    // War Drum: one Battlecry each recruit turn resolves two additional
    // times.  `amount` is the number of extra resolutions; triggerProgress
    // on the owned instance records that this turn's allowance was used.
    if (id == "BG32_MagicItem_416") return {TrinketEffect::BATTLECRY_EXTRA_TRIGGERS, 0, 0, 2};
    // Sphere of Memory grants three copies of the last Tavern spell cast at
    // the end of each recruit turn.  The actual spell ID is read from the
    // owner's lastTavernSpellDbfID state at resolution time.
    if (id == "BG36_MagicItem_372") return {TrinketEffect::END_TURN_LAST_TAVERN_SPELL, 0, 0, 3};
    // Inductive Gyroblade creates a canonical Magnetic Satellite at recruit
    // end.  `attack`/`health` are its base payload, while `value` is the
    // per-Tavern-spell improvement for this turn.  The golden form uses the
    // pinned 8/8 Satellite identity and the same scaling.
    if (id == "BG36_MagicItem_810") return {
        TrinketEffect::END_TURN_SPELL_SCALED_SATELLITE, 4, 4, 1,
        Race::INVALID, 0, 1, false, false, false, "BG34_Giant_610t"};
    if (id == "BG36_MagicItem_810t") return {
        TrinketEffect::END_TURN_SPELL_SCALED_SATELLITE, 8, 8, 1,
        Race::INVALID, 0, 1, false, false, false, "BG34_Giant_610_Gt"};
    // Emergency Gearblade casts Repair Job on the left-most friendly Mech at
    // recruit end. Resolve through the normal free-spell path so the
    // canonical +4/+8 payload and target-aware lifecycle are reused.
    if (id == "BG36_MagicItem_812") return {TrinketEffect::END_TURN_LEFTMOST_MECH_REPAIR};
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
    // Pearl of the Tides uses the same successful-spend observer with its
    // pinned +1/+1 pirate payload.
    if (id == "BG32_MagicItem_232") return {TrinketEffect::SPEND_GOLD_PIRATE_THRESHOLD_STATS, 1, 1, 10};
    // Extravagant Scale doubles friendly minions' Attack whenever 20 Gold is
    // spent, twice per game. Progress and lifetime uses belong to the owned
    // Trinket instance rather than global spend totals.
    if (id == "BG32_MagicItem_230") return {
        TrinketEffect::SPEND_GOLD_DOUBLE_ATTACK, 0, 0, 20,
        Race::INVALID, 0, 2};
    // Splinter of Aurum grants one random Golden Tier 5 minion on reaching
    // 15 Gold, once per game. The spend remainder is per owned Trinket.
    if (id == "BG32_MagicItem_350") return {
        TrinketEffect::SPEND_GOLD_RANDOM_GOLDEN, 0, 0, 15,
        Race::INVALID, 5, 1};
    // Designer Eyepatch changes the normal three-copy Pirate triple into a
    // two-copy triple. Player::ResolveDoubleTimeCopies calls the explicit
    // Minion merge primitive so both copies' state is retained and no coin is
    // generated.
    if (id == "BG30_MagicItem_439") return {
        TrinketEffect::TWO_COPIES_MAKE_GOLDEN};
    // These portraits have a fixed immediate minion grant plus a distinct
    // trigger.  Keep dedicated effects so acquisition and trigger semantics
    // cannot be accidentally separated or counted as a plain portrait.
    if (id == "BG30_MagicItem_548") return {TrinketEffect::ACQUIRE_FIXED_GLOWSCALE, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_035", PortraitEffect::GLOWSCALE_DIVINE_SHIELD_STATS, true};
    if (id == "BG36_MagicItem_201") return {TrinketEffect::ACQUIRE_FIXED_LIONFISH, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG36_201", PortraitEffect::LIONFISH_BEAST_STATS, true};
    // These portraits retain their printed fixed-card grant while extending
    // the generated minion's authoritative lifecycle hook.
    if (id == "BG30_MagicItem_868") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_174", PortraitEffect::SOUL_REWINDER_EXTRA_ATTACK, true};
    if (id == "BG35_MagicItem_156") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_500", PortraitEffect::FLAMING_ENFORCER_ADJACENT_STATS, true};
    if (id == "BG36_MagicItem_216") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG25_008_G", PortraitEffect::ETERNAL_KNIGHT_UNDEAD_STATS, true};
    if (id == "BG30_MagicItem_979") return {TrinketEffect::NEXT_TAVERN_SPELL_DISCOUNT, 0, 0, 1};
    if (id == "BG35_MagicItem_921") return {TrinketEffect::STAT_TAVERN_SPELL_DISCOUNT, 0, 0, 2};
    if (id == "BG30_MagicItem_986") return {TrinketEffect::FREE_TAVERN_SPELL_USES, 0, 0, 3};
    if (id == "BG32_MagicItem_823") return {TrinketEffect::START_TURN_GOLD_DAMAGE, 2, 0, 2};
    if (id == "BG35_MagicItem_150") return {TrinketEffect::REFRESH_TEMP_SHOP_STATS, 3, 3};
    // Frigid Blossom permanently lowers the next Tavern upgrade cost after
    // every successful refresh; it is not a static discount.
    if (id == "BG36_MagicItem_300") return {TrinketEffect::REFRESH_UPGRADE_COST_DISCOUNT, 0, 0, 1};
    if (id == "BG30_MagicItem_864") return {TrinketEffect::AVENGE_MINION_STATS, 4, 5, 3, Race::INVALID, 0, 1};
    if (id == "BG35_MagicItem_155") return {TrinketEffect::TAVERN_SPELL_TEMP_STATS_AFTER_DAMAGE, 1, 1};
    if (id == "BG30_MagicItem_923") return {TrinketEffect::PIRATE_ATTACK_GOLD, 0, 0, 1, Race::PIRATE, 0, 2};
    if (id == "BG30_MagicItem_925") return {TrinketEffect::ATTACKING_MINION_STATS, 4, 0};
    if (id == "BG30_MagicItem_995") return {TrinketEffect::START_COMBAT_HEALTH_FROM_ATTACK};
    if (id == "BG32_MagicItem_270") return {TrinketEffect::AVENGE_TAVERN_SPELL_ATTACK, 1, 0, 3};
    if (id == "BG30_MagicItem_973") return {TrinketEffect::REFRESH_EXTRA_SHOP_SLOTS, 0, 0, 2};
    if (id == "BG35_MagicItem_743") return {
        TrinketEffect::MAGNETIC_MECH_COST_AND_REFRESH_SLOT, 0, 0, 1,
        Race::INVALID, 0, 2};
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
    // Wildfeather Duster uses the shared thresholded Beast-summon reward.
    if (id == "BG35_MagicItem_700") return {TrinketEffect::SUMMON_BEAST_RANDOM_MINION, 0, 0, 6, Race::BEAST};
    if (id == "BG30_MagicItem_700") return {TrinketEffect::DEATHLY_PHYLACTERY, 0, 0, 1};
    if (id == "BG30_MagicItem_703") return {TrinketEffect::MYSTERY_CUBE_REPLACE_LESSER};
    if (id == "BG30_MagicItem_707") return {TrinketEffect::TICKATUS_DARKMOON_PRIZE, 0, 0, 3};
    if (id == "BG30_MagicItem_711") return {TrinketEffect::AFTER_PLAY_MINION_RANDOM_TIER_SPELL, 0, 0, 4};
    if (id == "BG30_MagicItem_777") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 3, Race::INVALID, 0, 1, false, false, false, "BG29_801", PortraitEffect::GOOSE_FLEDGLING_REWARD, true};
    if (id == "BG30_MagicItem_801") return {TrinketEffect::SINSTONE_DISCOVER_COPY, 0, 0, 2};
    if (id == "BG30_MagicItem_888") return {TrinketEffect::SOUVENIR_STAND_GREATER_COPY};
    if (id == "BG30_MagicItem_891") return {TrinketEffect::TRIP_VOUCHERS_REPLACE_GREATER, 0, 0, 2};
    if (id == "BG32_MagicItem_362") return {TrinketEffect::INNKEEPERS_HEARTH_DISCOVER};
    if (id == "BG32_MagicItem_362t") return {TrinketEffect::INNKEEPERS_HEARTH_DISCOVER};
    if (id == "BG32_MagicItem_400") return {TrinketEffect::TRANSFORM_WARBAND_TIER, 0, 0, 4};
    if (id == "BG32_MagicItem_809") return {TrinketEffect::AFTER_FIRST_SELL_BLOOD_GEMS_TAVERN, 0, 0, 3};
    if (id == "BG32_MagicItem_817") return {TrinketEffect::END_TURN_HIGHEST_TIER_TAVERN};
    if (id == "BG32_MagicItem_821") return {TrinketEffect::BUY_DEMON_HEALTH_ONCE_PER_TURN, 0, 0, 1, Race::DEMON};
    if (id == "BG32_MagicItem_824") return {TrinketEffect::DEMON_CONSUME_HIGHEST_HEALTH, 0, 0, 0, Race::DEMON, 0, 2, false, false, false, "BG29_140"};
    if (id == "BG32_MagicItem_902") return {TrinketEffect::AFTER_TAVERN_MINION_CONSUMED_RANDOM_SPELL, 0, 0, 2, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_933") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_830", PortraitEffect::PERMANENT_SPELLCRAFT, true};
    if (id == "BG32_MagicItem_951") return {TrinketEffect::GOLD_PENDANT_GOLDENIZE, 0, 0, 0, Race::INVALID, 4};
    if (id == "BG32_MagicItem_954") return {TrinketEffect::END_TURN_GOLDEN_LEFTMOST_STATS, 4, 3};
    if (id == "BG35_MagicItem_431t") return {TrinketEffect::AFTER_DEATHRATTLE_TEMP_BLOOD_GEM_BONUS, 2, 1};
    if (id == "BG35_MagicItem_433") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG35_437", PortraitEffect::VINESPEAKER_BLOOD_GEM_HEALTH, true};
    if (id == "BG35_MagicItem_434") return {TrinketEffect::JEWELRY_BOX_BLOOD_GEM};
    if (id == "BG35_MagicItem_714") return {TrinketEffect::START_COMBAT_POWDER_KEG};
    if (id == "BG35_MagicItem_732") return {TrinketEffect::START_COMBAT_SOUL_FERMENTER};
    if (id == "BG35_MagicItem_803" || id == "BG35_MagicItem_803t") return {TrinketEffect::AFTER_SELL_HERO_POWER_BUDDY};
    if (id == "BG35_MagicItem_812") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG35_MagicItem_812t"};
    if (id == "BG35_MagicItem_816") return {TrinketEffect::ACQUIRE_RANDOM_TRINKET, 0, 0, 0, Race::INVALID, 1};
    if (id == "BG35_MagicItem_816t") return {TrinketEffect::ACQUIRE_RANDOM_TRINKET, 0, 0, 4, Race::INVALID, 2};
    if (id == "BG35_MagicItem_821") return {TrinketEffect::KALEIDOSCOPE_DISCOVER};
    if (id == "BG35_MagicItem_821t") return {TrinketEffect::KALEIDOSCOPE_DISCOVER};
    if (id == "BG35_MagicItem_838") return {TrinketEffect::SPELLCRAFT_DOUBLE_STITCH};
    if (id == "BG35_MagicItem_840") return {TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES, 0, 0, 0, Race::INVALID, 0, 1, true};
    if (id == "BG35_MagicItem_840t") return {TrinketEffect::ACQUIRE_RANDOM_CHROMADRAKES, 0, 0, 7, Race::INVALID, 0, 2};
    if (id == "BG35_MagicItem_850") return {TrinketEffect::POCKET_CYCLONE, 0, 0, 1, Race::INVALID, 0, 1, true};
    if (id == "BG35_MagicItem_850t") return {TrinketEffect::POCKET_CYCLONE, 0, 0, 4, Race::INVALID, 0, 2, true};
    if (id == "BG32_MagicItem_998") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_360"};
    if (id == "BG35_MagicItem_861") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_FIXED_CARD, 0, 0, 10, Race::INVALID, 0, 1, false, false, false, "BG31_819"};
    if (id == "BG35_MagicItem_862") return {TrinketEffect::REFRESH_DOUBLE_HIGHEST_HEALTH};
    if (id == "BG35_MagicItem_863") return {TrinketEffect::ACQUIRE_FIXED_CARD_AFTER_SELL, 0, 0, 4, Race::INVALID, 0, 1, false, false, false, "BG33_899"};
    if (id == "BG35_MagicItem_925") return {TrinketEffect::SPELLCRAFT_MIGHT_OF_STORMWIND};
    if (id == "BG35_MagicItem_931") return {TrinketEffect::AFTER_BUY_MINION_COPY, 0, 0, 2};
    if (id == "BG35_MagicItem_931t") return {TrinketEffect::AFTER_BUY_MINION_COPY, 0, 0, 4};
    if (id == "BG30_MagicItem_952") return {TrinketEffect::START_COMBAT_ELEMENTAL_FROSTLING};
    if (id == "BG35_MagicItem_701") return {TrinketEffect::START_COMBAT_BEAST_SCALING, 1, 1};
    if (id == "BG30_MagicItem_442") return {TrinketEffect::START_COMBAT_QUILBOAR_BLOOD_GOLEM};
    if (id == "BG36_MagicItem_841") return {TrinketEffect::START_COMBAT_AUTO_ASSEMBLER};
    if (id == "BG30_MagicItem_902") return {TrinketEffect::START_COMBAT_EDGE_SHIELDS};
    if (id == "BG30_MagicItem_972") return {TrinketEffect::START_COMBAT_LEFT_COPY};
    if (id == "BG30_MagicItem_822") return {TrinketEffect::START_COMBAT_FIRST_SUMMON_COPY};
    if (id == "BG30_MagicItem_440") return {TrinketEffect::BOOM_CONTROLLER_FIRST_MECH_COPY};
    // Twin Sky Lanterns uses the same first-summon lifecycle as Sky Lanterns
    // but creates two copies.  Keep the multiplicity in the typed descriptor
    // so a full board still retries the complete reward later in combat.
    if (id == "BG30_MagicItem_822t2") return {
        TrinketEffect::START_COMBAT_FIRST_SUMMON_COPY, 0, 0, 0,
        Race::INVALID, 0, 2};
    if (id == "BG30_MagicItem_910") return {TrinketEffect::MECH_DIVINE_SHIELD_REPAIR, 0, 0, 3};
    if (id == "BG30_MagicItem_921") return {TrinketEffect::ACQUIRE_FLAGBEARER_PORTRAIT, 6, 0, 0, Race::PIRATE, 0, 1, false, false, false, "BG30_119"};
    if (id == "BG32_MagicItem_360") return {TrinketEffect::START_COMBAT_UNDEAD_EDGE_REBORN};
    if (id == "BG32_MagicItem_306") return {TrinketEffect::START_COMBAT_TRIGGER_DEATHRATTLES};
    if (id == "BG32_MagicItem_960") return {TrinketEffect::START_COMBAT_HIGHEST_HAND_MINION, 4, 4};
    if (id == "BG32_MagicItem_862") return {TrinketEffect::AFTER_DEATHRATTLE_RIGHTMOST_STATS, 2, 2};
    if (id == "BG35_MagicItem_432") return {TrinketEffect::AFTER_DEATHRATTLE_BLOOD_GEMS, 0, 0, 3};
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
    // Cloud Serpent Horn fires on the third friendly combat death.  The
    // source/recipient selection is deferred to the combat death boundary;
    // `value` is the per-instance Avenge cadence.
    if (id == "BG35_MagicItem_849") return {
        TrinketEffect::AVENGE_RIGHTMOST_ATTACK_TO_DRAGON, 0, 0, 3};
    if (id == "BG30_MagicItem_437") return {TrinketEffect::AVENGE_RANDOM_UNDEAD_REBORN, 0, 0, 5};
    if (id == "BG30_MagicItem_433") return {TrinketEffect::FIRST_DEATH_MAX_STATS_RANDOM};
    if (id == "BG30_MagicItem_545") return {TrinketEffect::AVENGE_RANDOM_MAGNETIC, 0, 0, 3};
    if (id == "BG30_MagicItem_546") return {TrinketEffect::AFTER_TWO_ATTACKS_QUILBOAR_GEM, 0, 0, 2};
    if (id == "BG32_MagicItem_930") return {TrinketEffect::SPELL_COUNT_RANDOM_NAGA, 0, 0, 7, Race::NAGA, 0, 1};
    if (id == "BG36_MagicItem_307") return {TrinketEffect::SPELL_COUNT_GOLD_ON_MINION, 0, 0, 3, Race::INVALID, 0, 1};
    // Bubble Crown improves every subsequent Tavern-spell stat payload by
    // +4/+4 once twelve successful Tavern spells have resolved.
    if (id == "BG35_MagicItem_920") return {
        TrinketEffect::SPELL_COUNT_TAVERN_SPELL_STATS, 4, 4, 12};
    if (id == "BG36_MagicItem_305") return {TrinketEffect::SPELL_COUNT_REPLACE_GREATER_NAGA, 0, 0, 15};
    // Fancy Spellbook is a one-shot seven-Gold threshold reward. Keep the
    // spend progress on the owned Trinket instance so purchases, refreshes,
    // upgrades, and spell payments all contribute through RecordGoldSpent.
    if (id == "BG30_MagicItem_999") return {TrinketEffect::SPEND_GOLD_SHINY_RING, 0, 0, 7};
    // Bloodbound Earrings count every successfully resolved Tavern spell
    // across recruit turns. The normal form repeats every four spells; the
    // golden form repeats every five and plays two gems per minion. The
    // printed "(4/5 left!)" is progress-to-next-trigger, not a finite
    // lifetime-use budget.
    if (id == "BG32_MagicItem_808") return {TrinketEffect::SPELL_COUNT_BLOOD_GEMS, 0, 0, 4, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_808t") return {TrinketEffect::SPELL_COUNT_BLOOD_GEMS, 0, 0, 5, Race::INVALID, 0, 2};
    if (id == "BG32_MagicItem_281") return {TrinketEffect::TAVERN_SPELL_NO_TYPE_STATS, 4, 4};
    if (id == "BG32_MagicItem_279") return {TrinketEffect::BLOOD_GEM_DIVINE_SHIELD};
    if (id == "BG32_MagicItem_200") return {TrinketEffect::ATTACKING_BEAST_SCALING, 2};
    if (id == "BG32_MagicItem_284") return {TrinketEffect::END_TURN_BLOOD_GEMS_PER_TYPE, 7};
    if (id == "BG32_MagicItem_111") return {TrinketEffect::END_TURN_MURLOC_STATS, 8, 8};
    if (id == "BG32_MagicItem_170") return {TrinketEffect::AFTER_MAGNETIC_MECH_REPAIR, 4, 8};
    // Fountain Pen uses the established Elemental-play shop aura path.  The
    // pinned normal and improved forms differ only in the persistent stat
    // payload, so both remain data-driven and share the same executor.
    if (id == "BG32_MagicItem_802") return {TrinketEffect::ELEMENTAL_STAT_GIVER_BONUS, 2, 1};
    if (id == "BG32_MagicItem_802t") return {TrinketEffect::ELEMENTAL_STAT_GIVER_BONUS, 4, 2};
    // Amplifying Essence starts at +1/+1 to Elemental stat-givers and
    // improves permanently to +2/+2 after five played Elementals.  Its
    // trigger progress and improvement level live on the owned effect.
    if (id == "BG36_MagicItem_380") return {
        TrinketEffect::ESCALATING_ELEMENTAL_STAT_GIVER_BONUS, 1, 1, 5};
    // Bluegill Flippers extends the reviewed spell-cast stat listener.  The
    // leftmost-target restriction is enforced by the shared hand/board spell
    // stat executor; the descriptor carries only the printed payload.
    if (id == "BG32_MagicItem_893") return {TrinketEffect::SPELL_CAST_LEFTMOST_MINION_STATS, 3, 3};
    // Charming Panpipes is the recruit-end leftmost-minion stat family.  Its
    // spell-improvement counter is maintained by the same typed end-turn
    // state used by the existing minion-stat trinkets.
    if (id == "BG32_MagicItem_922") return {TrinketEffect::END_TURN_LEFTMOST_MINION_STATS_PER_SPELL, 3, 3};
    if (id == "BG32_MagicItem_364") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_314"};
    // Jailer Sticker supplies the canonical Spellcraft token. The token's
    // typed TavernSpellBehavior enforces the printed Undead-only target and
    // random Undead hand reward; registering the source here also lets
    // RefreshSpellcraft recreate it at each recruit boundary.
    if (id == "BG35_MagicItem_306") return {
        TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1,
        false, false, false, "BG35_MagicItem_306t"};
    // The greater Jailer Sticker uses the same typed Spellcraft lifecycle,
    // with its generated token carrying the exact two-Undead reward count.
    // Keep the source identity registered so RefreshSpellcraft recreates one
    // temporary token per owned copy at each recruit boundary.
    if (id == "BG35_MagicItem_733") return {
        TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1,
        false, false, false, "BG35_MagicItem_733t"};
    if (id == "BG36_MagicItem_302") return {TrinketEffect::END_TURN_MINION_STATS, 2, 1};
    if (id == "BG36_MagicItem_302t") return {TrinketEffect::END_TURN_MINION_STATS, 4, 2};
    if (id == "BG36_MagicItem_371") return {TrinketEffect::TAVERN_SPELL_IMPROVE_AFTER_MINION_CAST, 1, 1};
    if (id == "BG36_MagicItem_390") return {TrinketEffect::ACQUIRE_RANDOM_BALLER, 0, 0, 1, Race::INVALID, 0, 1, true};
    // Beetle Band summons a Taunt-capable 2/2 Beetle after its Avenge
    // threshold.  Keep the threshold in value and the emitted count in
    // amount so the golden form is represented without a second executor.
    if (id == "BG32_MagicItem_860") return {TrinketEffect::AVENGE_SUMMON_BEETLES, 0, 0, 5, Race::INVALID, 0, 1};
    if (id == "BG32_MagicItem_860t") return {TrinketEffect::AVENGE_SUMMON_BEETLES, 0, 0, 7, Race::INVALID, 0, 2};
    if (id == "BG36_MagicItem_200") return {TrinketEffect::RALLY_ATTACK_FREE_REFRESH};
    if (id == "BG36_MagicItem_203") return {TrinketEffect::BATTLECRY_EDGE_STATS, 5, 5};
    if (id == "BG36_MagicItem_212") return {TrinketEffect::END_TURN_LEFT_DEATHRATTLES};
    if (id == "BG36_MagicItem_214") return {TrinketEffect::END_TURN_RALLY_TRIGGERS};
    // Murky Sticker grants its two left-most minions +1/+2 at recruit end,
    // improving both values by the lifetime Battlecry counter.
    if (id == "BG35_MagicItem_753") return {
        TrinketEffect::END_TURN_LEFT_STATS_PER_BATTLECRY, 1, 2};
    // Scraper Sticker: one random Magnetic Mech immediately and one more at
    // each subsequent recruit turn.  Keep this on the existing random-card
    // path so pool filtering and hand-cap behavior remain authoritative.
    if (id == "BG35_MagicItem_301") return {
        TrinketEffect::ACQUIRE_RANDOM_MINIONS, 0, 0, 0, Race::MECHANICAL,
        0, 1, true, false, true};
    // Trusty Crowbar: every acquired Pirate gives the left-most friendly
    // minion +12/+12.  This is acquisition-scoped (not purchase-scoped), so
    // generated/returned Pirates use the same lifecycle callback.
    if (id == "BG35_MagicItem_713") return {
        TrinketEffect::AFTER_GET_PIRATE_LEFTMOST_STATS, 12, 12, 0,
        Race::PIRATE};
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
    if (id == "BG30_MagicItem_301") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG25_008", PortraitEffect::ETERNAL_KNIGHT_TAUNT_REBORN, true};
    // Automaton Portrait summons the canonical Ancestral Automaton.  Keep
    // the identity in the descriptor alongside other fixed portrait rewards
    // so callers and static coverage checks do not need a second mapping.
    if (id == "BG30_MagicItem_303") return {
        TrinketEffect::START_COMBAT_AUTOMATON_SUMMON, 0, 0, 0, Race::INVALID,
        0, 1, false, false, false, "BG_TTN_401"};
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
    if (id == "BG30_MagicItem_431") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG28_707", PortraitEffect::LIVING_AZERITE_ELEMENTAL_STATS, true};
    if (id == "BG30_MagicItem_432") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_888", PortraitEffect::BELCHER_VENOMOUS_LOSS_STATS, true};
    if (id == "BG30_MagicItem_432t") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_888_G", PortraitEffect::BELCHER_VENOMOUS_LOSS_STATS, true};
    if (id == "BG30_MagicItem_555") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_175", PortraitEffect::SURPRISE_MORE_ELEMENTALS, true};
    // Promo Portrait grants the canonical Prized Promo-Drake and repeats the
    // first friendly Start-of-Combat effect once per combat.  The fixed-card
    // acquisition and portrait modifier stay in one typed descriptor so both
    // lifecycle halves are credited together.
    if (id == "BG30_MagicItem_918") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG21_014", PortraitEffect::PROMO_START_COMBAT_EXTRA_TRIGGER, true};
    // Hackerfin Portrait grants a Hackerfin and makes every Hackerfin repeat
    // its Battlecry at recruit end.  The per-instance marker is applied by
    // Player::AcquireTrinket/PlayCard, while Minion owns the end-turn hook.
    if (id == "BG32_MagicItem_925") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_148", PortraitEffect::HACKERFIN_END_TURN_BATTLECRY, true};
    if (id == "BG32_MagicItem_803") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BGS_078", PortraitEffect::MACAW_LEFTMOST_BATTLECRY, true};
    if (id == "BG35_MagicItem_740") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG35_342", PortraitEffect::SKY_GOLEM_DEATHRATTLE_STATS, true};
    if (id == "BG35_MagicItem_834") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_801", PortraitEffect::RYLAK_START_COMBAT_DEATHRATTLES, true};
    if (id == "BG30_MagicItem_803") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_BOT_606", PortraitEffect::KABOOM_BOT_DEATHRATTLE_DAMAGE, true};
    if (id == "BG30_MagicItem_825") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG21_013", PortraitEffect::WHELP_SMUGGLER_STATS_AND_DRAGON, true};
    if (id == "BG30_MagicItem_828") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_505", PortraitEffect::ZESTY_SHAKER_EXTRA_COPY, true};
    if (id == "BG30_MagicItem_869") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG29_873", PortraitEffect::FELBLOOD_BOTH_STATS, true};
    if (id == "BG30_MagicItem_971") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BGS_009", PortraitEffect::LIGHTFANG_ALL_TYPES, true};
    if (id == "BG32_MagicItem_274") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_157", PortraitEffect::BRISTLEBACH_ALL_MINIONS, true};
    // Scrapsmith Portrait grants the canonical Bristlemane Scrapsmith.  The
    // Scrapsmith Portrait grants the canonical normal Bristlemane and a
    // persistent portrait aura. The minion's own CardDef supplies the
    // hand-Gem death trigger; Player/Battle apply the permanent Gem to every
    // surviving Scrapsmith after each friendly Taunt death.
    if (id == "BG35_MagicItem_430") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG24_707", PortraitEffect::SCRAPSMITH_TAUNT_DEATH_GEMS, true};
    if (id == "BG32_MagicItem_301") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_350", PortraitEffect::BASSGILL_SUMMON_DIVINE_SHIELD, true};
    if (id == "BG32_MagicItem_804") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG_OG_221", PortraitEffect::SELFLESS_BATTLECRY, true};
    // These portraits use the shared fixed-card acquisition path and carry
    // typed executable auras for their printed modifiers.
    if (id == "BG32_MagicItem_806") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_HERO_801pt", PortraitEffect::BATTLECRUISER_REFRESH_UPGRADE, true};
    if (id == "BG32_MagicItem_820") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG21_006", PortraitEffect::IMPULSIVE_ADJACENT_DEATHRATTLE, true};
    if (id == "BG32_MagicItem_830") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG25_041", PortraitEffect::FELEMENTAL_EXTRA_STATS, true};
    if (id == "BG32_MagicItem_894") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG28_845"};
    if (id == "BG32_MagicItem_906") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_HERO_802pt7"};
    if (id == "BG32_MagicItem_920") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_924", PortraitEffect::PERMANENT_SPELLCRAFT, true};
    if (id == "BG32_MagicItem_950") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG31_891"};
    // Tide Raiser Portrait grants Tide Raiser and arms the combat spell-copy
    // listener.  The listener is resolved in CastTavernSpellFree, the shared
    // combat Spellcraft transaction boundary.
    if (id == "BG35_MagicItem_922") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_920", PortraitEffect::TIDE_RAISER_COMBAT_SPELL_COPY, true};
    if (id == "BG32_MagicItem_831") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BGS_115", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_741") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_149", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_742") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_147", PortraitEffect::NONE, true};
    // Leapfrogger Portrait has no additional account-wide modifier: its
    // canonical Timewarped Leapfrogger already carries the complete
    // deathrattle behavior in GeneratedBehaviorMappings.
    if (id == "BG35_MagicItem_870") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_031", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_924") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_035", PortraitEffect::GROUNDBREAKER_ADJACENT_STATS, true};
    // Empowerment Portrait: the fixed spell repeats at every recruit start.
    if (id == "BG32_MagicItem_944") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, true, false, false, "BG28_169"};
    // Privateer Portrait atomically grants the minion and two Bounties, then
    // repeats the Bounty grant at each recruit start.
    if (id == "BG35_MagicItem_712") return {TrinketEffect::ACQUIRE_FIXED_CARD_AND_BOUNTIES, 0, 0, 2, Race::INVALID, 0, 1, true, false, false, "BG33_825"};
    // Sunken Anchor grants two random Bounties on acquisition and at each
    // subsequent recruit start.
    if (id == "BG35_MagicItem_890") return {TrinketEffect::START_TURN_RANDOM_BOUNTIES, 0, 0, 2};
    // Bird Feeder is the non-portrait Avenge(2) +1/+1 Trinket.  Keep the
    // trigger threshold in `value`; Season14State owns its per-combat
    // progress and resets it at combat boundaries.
    if (id == "BG32_MagicItem_864") return {TrinketEffect::AVENGE_MINION_STATS, 1, 1, 2};
    // Golden Bird Feeder keeps the Avenge(2) cadence and uses its pinned
    // +4/+4 permanent payload.
    if (id == "BG32_MagicItem_864t") return {TrinketEffect::AVENGE_MINION_STATS, 4, 4, 2};
    // These portraits have executable fixed-card acquisition and explicit
    // lifecycle handlers for their printed modifiers (identity mutation and
    // spell-trigger stat extension).
    if (id == "BG32_MagicItem_179") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG26_ICC_901", PortraitEffect::DRAKKARI_ENCHANTER_ALL_TYPES, true};
    if (id == "BG32_MagicItem_283") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG28_741", PortraitEffect::CZARINA_DIVINE_SHIELD_HEALTH, true};
    // Goldgrubber Portrait grants two independent generated minions.  Keep
    // both identities in one typed descriptor so hand-cap handling is shared
    // and neither reward is silently dropped.
    if (id == "BG32_MagicItem_953") return {
        TrinketEffect::ACQUIRE_TWO_FIXED_CARDS, 0, 0, 0, Race::INVALID, 0,
        1, false, false, false, "BGS_066", PortraitEffect::NONE, true, 0, 1,
        "BG32_236"};
    // Curator Sticker grants the golden Mishmash buddy and its fixed
    // Venomous Amalgam.  Keep both generated identities in the shared
    // acquisition descriptor; the card defs carry the printed golden and
    // Venomous state, so no portrait-only approximation is needed.
    if (id == "BG32_MagicItem_807") return {
        TrinketEffect::ACQUIRE_TWO_FIXED_CARDS, 0, 0, 0, Race::INVALID, 0,
        1, false, false, false, "TB_BaconShop_HERO_33_Buddy_G",
        PortraitEffect::NONE, true, 0, 1, "TB_BaconShop_HP_033t"};
    // These two portraits' generated minions carry the printed lifecycle in
    // their authoritative CardDef (Showy Cyclist's permanent Deathrattle
    // stats and the golden Escapee's lockbox cadence respectively).
    if (id == "BG36_MagicItem_362") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG31_925", PortraitEffect::NONE, true};
    if (id == "BG36_MagicItem_363") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG36_523_G", PortraitEffect::NONE, true};
    if (id == "BG36_MagicItem_204") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_690", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_310") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_Giant_330", PortraitEffect::NONE, true};
    if (id == "BG35_MagicItem_848t") return {TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1, false, false, false, "BG34_639_G", PortraitEffect::NONE, true};
    if (id == "BG32_MagicItem_700") return {TrinketEffect::TAVERN_SPELL_GROWING_STATS, 1, 1};
    if (id == "BG32_MagicItem_801") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 1};
    if (id == "BG32_MagicItem_801t") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 1};
    if (id == "BG36_MagicItem_373") return {TrinketEffect::TAVERN_SPELL_STATS, 1, 2};
    // Flighty Portrait grants the canonical Flighty Scout immediately.  Its
    // post-spell +3/+3 payload is applied by the shared successful-spell
    // hook to every owned Scout, including copies already in hand.
    if (id == "BG36_MagicItem_820") return {
        TrinketEffect::ACQUIRE_FIXED_CARD, 3, 3, 0, Race::INVALID, 0, 1,
        false, false, false, "BG32_330",
        PortraitEffect::FLIGHTY_SCOUT_TAVERN_SPELL_STATS, true};
    if (id == "BG32_MagicItem_365") return {TrinketEffect::START_COMBAT_EXTRA_TRIGGER};
    if (id == "BG35_MagicItem_851") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_RANDOM_SPELL, 0, 0, 2};
    // Cookie's Stirring Rod grants two random Tavern spells after every five
    // Murlocs played.  `value` is the printed cadence and `amount` is the
    // reward count; progress is retained per owned Trinket and resets only
    // when the threshold is reached.
    if (id == "BG36_MagicItem_850") return {
        TrinketEffect::AFTER_PLAY_MURLOC_RANDOM_SPELL, 0, 0, 5,
        Race::MURLOC, 0, 2};
    // Magicfin Sticker gives one canonical 1/1 Murloc taught the purchased
    // Tavern spell, at most twice per recruit turn. The purchase executor
    // invokes the shared BuyTavernSpellMurlocTask so pool identity,
    // hand-capacity, and teaching semantics cannot drift from Magicfin.
    if (id == "BG35_MagicItem_750") return {
        TrinketEffect::AFTER_BUY_TAVERN_SPELL_MURLOC, 0, 0, 2,
        Race::MURLOC, 0, 1};
    // Recycling Sticker grants one free refresh after each successful
    // Elemental play; the player executor consumes this at that boundary.
    if (id == "BG32_MagicItem_888") return {TrinketEffect::AFTER_PLAY_ELEMENTAL_FREE_REFRESH, 0, 0, 1};
    // Shadowy Elixir grants five Armor on acquisition and deals one damage
    // after each recruit-phase Demon play.
    if (id == "BG32_MagicItem_887") return {TrinketEffect::AFTER_PLAY_DEMON_DAMAGE, 5, 0, 1};
    // Ur'zul Sticker consumes exactly one Tavern minion with another
    // friendly Demon after a successful Demon play.
    if (id == "BG35_MagicItem_154") return {TrinketEffect::AFTER_PLAY_DEMON_CONSUME};
    // Consuming Claw is resolved by the authoritative Tavern-consumption
    // executors. Keep this dedicated marker separate from ordinary Demon
    // buffs so failed consumes cannot fire it.
    if (id == "BG36_MagicItem_801") return {TrinketEffect::DEMON_CONSUME_BONUS_KEYWORDS, 5, 5};
    // Cliffdiver buffs exactly the left-most minion at recruit end; each
    // Battlecry triggered that turn improves both components by one.
    if (id == "BG32_MagicItem_890") return {TrinketEffect::END_TURN_LEFTMOST_STATS_PER_BATTLECRY, 3, 2};
    // Egg of the Endtimes Portrait grants one canonical Egg immediately and
    // repeats the grant every other recruit start.  The cadence is kept in
    // the per-Trinket trigger counter, so a full hand retries safely.
    if (id == "BG35_MagicItem_842") return {
        TrinketEffect::ACQUIRE_FIXED_CARD, 0, 0, 0, Race::INVALID, 0, 1,
        true, false, false, "BG34_639", PortraitEffect::NONE, false, 0, 2};
    // Demonic Tapestry arms one highest-tier Tavern minion to use Health as
    // its purchase currency after four successful refreshes.  The armed
    // marker is stored per owned Trinket, so separate copies keep cadence
    // independent and the effect cannot leak across players.
    if (id == "BG35_MagicItem_152") return {
        TrinketEffect::REFRESH_HIGHEST_TIER_HEALTH_PURCHASE, 0, 0, 4,
        Race::INVALID, 0, 3};
    // The Eye of Sargeras changes every fourth successful minion purchase
    // into a Health payment.  Keep cadence per owned copy and leave the
    // purchase executor responsible for the final post-buy payment, so a
    // full hand or rejected purchase cannot consume the counter.
    if (id == "BG30_MagicItem_701") return {
        TrinketEffect::BUY_MINION_HEALTH_CADENCE, 0, 0, 4};
    // Bazaar Sticker: one successful Tavern-spell purchase per recruit turn
    // is paid with Health instead of Gold.  The per-instance progress marker
    // is reset at recruit start and consumed only after a committed purchase.
    if (id == "BG32_MagicItem_822") return {
        TrinketEffect::TAVERN_SPELL_HEALTH_ONCE_PER_TURN, 0, 0, 1};
    // Gold-plated Compass's card-template payload is "next {0} ..."; the
    // local resolved specialization for this entity is Naga (the source's
    // script-data value 92 is not a purchase count).  Keep the specialization
    // in the typed descriptor so the purchase executor remains generic.
    if (id == "BG32_MagicItem_901") return {
        TrinketEffect::NEXT_RACE_MINION_GOLDEN, 0, 0, 5, Race::NAGA};
    // Primalfin Lookout owns the exact Murloc Discover Battlecry. The
    // portrait's after-Discover spell payload is handled at the shared
    // Discover commit boundary, where the selected minion is authoritative.
    if (id == "BG30_MagicItem_702") return {
        TrinketEffect::ACQUIRE_PRIMALFIN_PORTRAIT, 0, 0, 0,
        Race::INVALID, 0, 1, false, false, false, "BGS_020"};
    // Glass of Perspective grants one random Choose One minion immediately
    // and repeats the same reward at each recruit start.  Keep this distinct
    // from the generic random-minion path so the Choose One filter remains
    // authoritative and cannot silently widen to the entire pool.
    if (id == "BG36_MagicItem_303") return {
        TrinketEffect::ACQUIRE_RANDOM_CHOOSE_ONE, 0, 0, 1,
        Race::INVALID, 0, 1, false, false, false, {}, PortraitEffect::NONE,
        false, 0, 1};
    // The greater form follows the pinned text exactly: two independent
    // random Choose One minions on acquisition and two more at every recruit
    // start.  Keep the same typed path so hand-cap handling, fresh modifiers,
    // and per-copy cadence remain identical to the normal form.
    if (id == "BG36_MagicItem_303t") return {
        TrinketEffect::ACQUIRE_RANDOM_CHOOSE_ONE, 0, 0, 2,
        Race::INVALID, 0, 2, false, false, false, {}, PortraitEffect::NONE,
        false, 0, 1};
    // Errgl Sticker gives one random Mrrglton (Mama or Papa) immediately and
    // repeats that exact two-card choice pool at every recruit start. Keep the
    // pool typed so this cannot widen to arbitrary minions or confuse the
    // reward with Cousin Errgl's end-turn behavior.
    if (id == "BG35_MagicItem_309") return {
        TrinketEffect::ACQUIRE_RANDOM_MRGLTON, 0, 0, 1,
        Race::INVALID, 0, 1, true};
    if (id == "BG36_MagicItem_308")
        return {TrinketEffect::TRAILBLAZER_CHOOSE_ONE};
    if (id == "BG32_MagicItem_844")
        return {TrinketEffect::SACRIFICIAL_ALTAR};
    return {};
}

void TrinketBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Windfall Portrait's generated owner is EndTurnWindfallPortraitTask{1}
    // and its greater form is EndTurnWindfallPortraitTask{2}.
    // Pilgrimp Sticker: One Demon each turn costs Health instead of Gold.
    // Windfall Portrait owns a typed end-of-turn lifecycle in the generated
    // behavior table; keep the trinket registry explicit as well so coverage
    // cannot claim a row whose native owner was accidentally removed.
    cards.emplace("BG32_MagicItem_832", CardDef{});
    cards.emplace("BG32_MagicItem_361", CardDef{});
    cards.emplace("BG32_MagicItem_361t", CardDef{});
    cards.emplace("BG32_MagicItem_415", CardDef{});
    cards.emplace("BG32_MagicItem_417", CardDef{});
    cards.emplace("BG32_MagicItem_832t", CardDef{});
    cards.emplace("BG32_MagicItem_286", CardDef{});
    cards.emplace("BG30_Trinket_1st", CardDef{});
    cards.emplace("BG30_Trinket_2nd", CardDef{});
    cards.emplace("BG32_MagicItem_278", CardDef{});
    cards.emplace("BG30_MagicItem_416", CardDef{});
    cards.emplace("BG32_MagicItem_803", CardDef{});
    cards.emplace("BG35_MagicItem_740", CardDef{});
    cards.emplace("BG35_MagicItem_834", CardDef{});
    cards.emplace("BG36_MagicItem_812", CardDef{});
    cards.emplace("BG36_MagicItem_217", CardDef{});
    cards.emplace("BG35_MagicItem_731", CardDef{});
    cards.emplace("BG30_MagicItem_429", CardDef{});
    cards.emplace("BG32_MagicItem_892", CardDef{});
    cards.emplace("BG35_MagicItem_872", CardDef{});
    cards.emplace("BG36_MagicItem_208", CardDef{});
    cards.emplace("BG35_MagicItem_755", CardDef{});
    cards.emplace("BG30_MagicItem_996", CardDef{});
    cards.emplace("BG30_MagicItem_998", CardDef{});
    cards.emplace("BG35_MagicItem_820", CardDef{});
    cards.emplace("BG32_MagicItem_271", CardDef{});
    cards.emplace("BG35_MagicItem_818", CardDef{});
    cards.emplace("BG30_MagicItem_841", CardDef{});
    cards.emplace("BG36_MagicItem_220", CardDef{});
    cards.emplace("BG36_MagicItem_206", CardDef{});
    cards.emplace("BG36_MagicItem_301", CardDef{});
    cards.emplace("BG36_MagicItem_309", CardDef{});
    cards.emplace("BG36_MagicItem_308", CardDef{});
    cards.emplace("BG32_MagicItem_844", CardDef{});
    cards.emplace("BG32_MagicItem_858", CardDef{});
    cards.emplace("BG30_MagicItem_993", CardDef{});
    cards.emplace("BG30_MagicItem_430", CardDef{});
    cards.emplace("BG30_MagicItem_991", CardDef{});
    cards.emplace("BG32_MagicItem_172", CardDef{});
    cards.emplace("BG30_MagicItem_543", CardDef{});
    cards.emplace("BG35_MagicItem_303", CardDef{});
    cards.emplace("BG31_MagicItem_903", CardDef{});
    cards.emplace("BG35_MagicItem_305", CardDef{});
    cards.emplace("BG35_MagicItem_817", CardDef{});
    cards.emplace("BG32_MagicItem_282", CardDef{});
    cards.emplace("BG32_MagicItem_304", CardDef{});
    cards.emplace("BG30_MagicItem_847", CardDef{});
    cards.emplace("BG32_MagicItem_366", CardDef{});
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
    cards.emplace("BG36_MagicItem_840", CardDef{});
    cards.emplace("BG30_MagicItem_984", CardDef{});
    cards.emplace("BG30_MagicItem_984t", CardDef{});
    cards.emplace("BG30_MagicItem_900", CardDef{});
    cards.emplace("BG30_MagicItem_900t", CardDef{});
    cards.emplace("BG30_MagicItem_989", CardDef{});
    cards.emplace("BG30_MagicItem_989t", CardDef{});
    cards.emplace("BG32_MagicItem_204", CardDef{});
    cards.emplace("BG32_MagicItem_205", CardDef{});
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
    cards.emplace("BG30_MagicItem_918", CardDef{});
    cards.emplace("BG32_MagicItem_925", CardDef{});
    cards.emplace("BG30_MagicItem_803", CardDef{});
    cards.emplace("BG30_MagicItem_825", CardDef{});
    cards.emplace("BG30_MagicItem_828", CardDef{});
    cards.emplace("BG30_MagicItem_869", CardDef{});
    cards.emplace("BG30_MagicItem_971", CardDef{});
    cards.emplace("BG32_MagicItem_274", CardDef{});
    cards.emplace("BG35_MagicItem_430", CardDef{});
    cards.emplace("BG32_MagicItem_301", CardDef{});
    cards.emplace("BG32_MagicItem_804", CardDef{});
    cards.emplace("BG32_MagicItem_806", CardDef{});
    cards.emplace("BG32_MagicItem_820", CardDef{});
    cards.emplace("BG32_MagicItem_830", CardDef{});
    cards.emplace("BG32_MagicItem_894", CardDef{});
    cards.emplace("BG32_MagicItem_906", CardDef{});
    cards.emplace("BG32_MagicItem_920", CardDef{});
    cards.emplace("BG32_MagicItem_950", CardDef{});
    cards.emplace("BG32_MagicItem_907", CardDef{});
    cards.emplace("BG35_MagicItem_930", CardDef{});
    cards.emplace("BG35_MagicItem_922", CardDef{});
    cards.emplace("BG32_MagicItem_831", CardDef{});
    cards.emplace("BG35_MagicItem_741", CardDef{});
    cards.emplace("BG35_MagicItem_742", CardDef{});
    cards.emplace("BG35_MagicItem_870", CardDef{});
    cards.emplace("BG35_MagicItem_924", CardDef{});
    cards.emplace("BG32_MagicItem_944", CardDef{});
    cards.emplace("BG35_MagicItem_712", CardDef{});
    cards.emplace("BG35_MagicItem_890", CardDef{});
    cards.emplace("BG32_MagicItem_864", CardDef{});
    cards.emplace("BG32_MagicItem_864t", CardDef{});
    cards.emplace("BG32_MagicItem_179", CardDef{});
    cards.emplace("BG32_MagicItem_283", CardDef{});
    cards.emplace("BG32_MagicItem_953", CardDef{});
    cards.emplace("BG32_MagicItem_807", CardDef{});
    cards.emplace("BG36_MagicItem_362", CardDef{});
    cards.emplace("BG36_MagicItem_363", CardDef{});
    cards.emplace("BG36_MagicItem_204", CardDef{});
    cards.emplace("BG35_MagicItem_310", CardDef{});
    cards.emplace("BG35_MagicItem_848t", CardDef{});
    cards.emplace("BG35_MagicItem_849", CardDef{});
    cards.emplace("BG32_MagicItem_700", CardDef{});
    cards.emplace("BG32_MagicItem_801", CardDef{});
    cards.emplace("BG32_MagicItem_801t", CardDef{});
    cards.emplace("BG32_MagicItem_230", CardDef{});
    cards.emplace("BG32_MagicItem_350", CardDef{});
    cards.emplace("BG36_MagicItem_373", CardDef{});
    cards.emplace("BG32_MagicItem_365", CardDef{});
    cards.emplace("BG35_MagicItem_851", CardDef{});
    cards.emplace("BG36_MagicItem_850", CardDef{});
    cards.emplace("BG32_MagicItem_888", CardDef{});
    cards.emplace("BG32_MagicItem_887", CardDef{});
    cards.emplace("BG35_MagicItem_154", CardDef{});
    cards.emplace("BG36_MagicItem_801", CardDef{});
    cards.emplace("BG32_MagicItem_890", CardDef{});
    cards.emplace("BG30_MagicItem_879", CardDef{});
    cards.emplace("BG30_MagicItem_879t", CardDef{});
    cards.emplace("BG32_MagicItem_891", CardDef{});
    cards.emplace("BG32_MagicItem_935", CardDef{});
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
    cards.emplace("BG35_MagicItem_300", CardDef{});
    cards.emplace("BG35_MagicItem_300t", CardDef{});
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
    cards.emplace("BG36_MagicItem_211", CardDef{});
    cards.emplace("BG30_MagicItem_920", CardDef{});
    cards.emplace("BG32_MagicItem_931", CardDef{});
    cards.emplace("BG30_MagicItem_981", CardDef{});
    cards.emplace("BG30_MagicItem_438t", CardDef{});
    cards.emplace("BG32_MagicItem_171", CardDef{});
    cards.emplace("BG35_MagicItem_435", CardDef{});
    cards.emplace("BG32_MagicItem_367", CardDef{});
    cards.emplace("BG35_MagicItem_752", CardDef{});
    cards.emplace("BG32_MagicItem_416", CardDef{});
    cards.emplace("BG30_MagicItem_435", CardDef{});
    cards.emplace("BG30_MagicItem_982", CardDef{});
    cards.emplace("BG30_MagicItem_821t2", CardDef{});
    cards.emplace("BG30_MagicItem_943", CardDef{});
    cards.emplace("BG30_MagicItem_924", CardDef{});
    cards.emplace("BG30_MagicItem_924t", CardDef{});
    cards.emplace("BG32_MagicItem_232", CardDef{});
    cards.emplace("BG32_MagicItem_998", CardDef{});
    cards.emplace("BG32_MagicItem_926", CardDef{});
    cards.emplace("BG32_MagicItem_957", CardDef{});
    cards.emplace("BG30_MagicItem_548", CardDef{});
    cards.emplace("BG36_MagicItem_201", CardDef{});
    cards.emplace("BG30_MagicItem_979", CardDef{});
    cards.emplace("BG35_MagicItem_921", CardDef{});
    cards.emplace("BG30_MagicItem_986", CardDef{});
    cards.emplace("BG32_MagicItem_823", CardDef{});
    cards.emplace("BG35_MagicItem_150", CardDef{});
    cards.emplace("BG36_MagicItem_830", CardDef{});
    cards.emplace("BG36_MagicItem_831", CardDef{});
    cards.emplace("BG36_MagicItem_300", CardDef{});
    cards.emplace("BG35_MagicItem_700", CardDef{});
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
    cards.emplace("BG35_MagicItem_815", CardDef{});
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
    cards.emplace("BG30_MagicItem_822", CardDef{});
    cards.emplace("BG30_MagicItem_822t2", CardDef{});
    cards.emplace("BG30_MagicItem_910", CardDef{});
    cards.emplace("BG30_MagicItem_921", CardDef{});
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
    cards.emplace("BG36_MagicItem_390", CardDef{});
    cards.emplace("BG36_MagicItem_370", CardDef{});
    cards.emplace("BG32_MagicItem_860", CardDef{});
    cards.emplace("BG32_MagicItem_860t", CardDef{});
    cards.emplace("BG36_MagicItem_200", CardDef{});
    cards.emplace("BG36_MagicItem_203", CardDef{});
    cards.emplace("BG36_MagicItem_212", CardDef{});
    cards.emplace("BG36_MagicItem_214", CardDef{});
    cards.emplace("BG35_MagicItem_753", CardDef{});
    cards.emplace("BG35_MagicItem_301", CardDef{});
    cards.emplace("BG35_MagicItem_713", CardDef{});
    cards.emplace("BG30_MagicItem_710", CardDef{});
    cards.emplace("BG30_MagicItem_951", CardDef{});
    cards.emplace("BG30_MagicItem_713", CardDef{});
    cards.emplace("BG30_MagicItem_931", CardDef{});
    cards.emplace("BG35_MagicItem_302", CardDef{});
    cards.emplace("BG32_MagicItem_930", CardDef{});
    cards.emplace("BG32_MagicItem_932", CardDef{});
    cards.emplace("BG36_MagicItem_307", CardDef{});
    cards.emplace("BG35_MagicItem_920", CardDef{});
    cards.emplace("BG36_MagicItem_305", CardDef{});
    cards.emplace("BG32_MagicItem_281", CardDef{});
    cards.emplace("BG32_MagicItem_279", CardDef{});
    cards.emplace("BG32_MagicItem_363", CardDef{});
    cards.emplace("BG32_MagicItem_200", CardDef{});
    cards.emplace("BG32_MagicItem_284", CardDef{});
    cards.emplace("BG32_MagicItem_111", CardDef{});
    cards.emplace("BG32_MagicItem_170", CardDef{});
    cards.emplace("BG32_MagicItem_802", CardDef{});
    cards.emplace("BG32_MagicItem_802t", CardDef{});
    cards.emplace("BG32_MagicItem_893", CardDef{});
    cards.emplace("BG32_MagicItem_922", CardDef{});
    cards.emplace("BG32_MagicItem_364", CardDef{});
    cards.emplace("BG35_MagicItem_306", CardDef{});
    cards.emplace("BG35_MagicItem_733", CardDef{});
    cards.emplace("BG30_MagicItem_705", CardDef{});
    cards.emplace("BG35_MagicItem_842", CardDef{});
    cards.emplace("BG35_MagicItem_152", CardDef{});
    cards.emplace("BG35_MagicItem_432", CardDef{});
    cards.emplace("BG30_MagicItem_701", CardDef{});
    cards.emplace("BG32_MagicItem_822", CardDef{});
    cards.emplace("BG32_MagicItem_901", CardDef{});
    cards.emplace("BG30_MagicItem_702", CardDef{});
    cards.emplace("BG36_MagicItem_303", CardDef{});
    cards.emplace("BG36_MagicItem_303t", CardDef{});
    cards.emplace("BG35_MagicItem_309", CardDef{});
    cards.emplace("BG35_MagicItem_750", CardDef{});
    cards.emplace("BG30_MagicItem_427", CardDef{});
    cards.emplace("BG30_MagicItem_427t", CardDef{});
    cards.emplace("BG30_MagicItem_709", CardDef{});
    cards.emplace("BG30_MagicItem_709t", CardDef{});
    cards.emplace("BG30_MagicItem_700", CardDef{});
    cards.emplace("BG30_MagicItem_703", CardDef{});
    cards.emplace("BG30_MagicItem_707", CardDef{});
    cards.emplace("BG30_MagicItem_711", CardDef{});
    cards.emplace("BG30_MagicItem_777", CardDef{});
    cards.emplace("BG30_MagicItem_801", CardDef{});
    cards.emplace("BG30_MagicItem_888", CardDef{});
    cards.emplace("BG30_MagicItem_891", CardDef{});
    cards.emplace("BG32_MagicItem_362", CardDef{});
    cards.emplace("BG32_MagicItem_362t", CardDef{});
    cards.emplace("BG32_MagicItem_400", CardDef{});
    cards.emplace("BG32_MagicItem_809", CardDef{});
    cards.emplace("BG32_MagicItem_817", CardDef{});
    cards.emplace("BG32_MagicItem_821", CardDef{});
    cards.emplace("BG32_MagicItem_824", CardDef{});
    cards.emplace("BG32_MagicItem_902", CardDef{});
    cards.emplace("BG32_MagicItem_933", CardDef{});
    cards.emplace("BG32_MagicItem_951", CardDef{});
    cards.emplace("BG32_MagicItem_954", CardDef{});
    cards.emplace("BG35_MagicItem_431t", CardDef{});
    cards.emplace("BG35_MagicItem_433", CardDef{});
    cards.emplace("BG35_MagicItem_434", CardDef{});
    cards.emplace("BG35_MagicItem_714", CardDef{});
    cards.emplace("BG35_MagicItem_732", CardDef{});
    cards.emplace("BG35_MagicItem_803", CardDef{});
    cards.emplace("BG35_MagicItem_803t", CardDef{});
    cards.emplace("BG35_MagicItem_812", CardDef{});
    cards.emplace("BG35_MagicItem_812t", CardDef{});
    cards.emplace("BG35_MagicItem_816", CardDef{});
    cards.emplace("BG35_MagicItem_816t", CardDef{});
    cards.emplace("BG35_MagicItem_821", CardDef{});
    cards.emplace("BG35_MagicItem_821t", CardDef{});
    cards.emplace("BG35_MagicItem_838", CardDef{});
    cards.emplace("BG35_MagicItem_840", CardDef{});
    cards.emplace("BG35_MagicItem_840t", CardDef{});
    cards.emplace("BG35_MagicItem_850", CardDef{});
    cards.emplace("BG35_MagicItem_850t", CardDef{});
    cards.emplace("BG35_MagicItem_861", CardDef{});
    cards.emplace("BG35_MagicItem_862", CardDef{});
    cards.emplace("BG35_MagicItem_863", CardDef{});
    cards.emplace("BG35_MagicItem_925", CardDef{});
    cards.emplace("BG35_MagicItem_931", CardDef{});
    cards.emplace("BG35_MagicItem_931t", CardDef{});
}
}  // namespace RosettaStone::Battlegrounds
