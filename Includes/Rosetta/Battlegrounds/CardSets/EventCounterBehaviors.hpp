#ifndef ROSETTASTONE_BATTLEGROUNDS_EVENT_COUNTER_BEHAVIORS_HPP
#define ROSETTASTONE_BATTLEGROUNDS_EVENT_COUNTER_BEHAVIORS_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>

#include <map>
#include <string>
#include <string_view>

namespace RosettaStone::Battlegrounds
{
//! Data needed by a minion whose text is driven by a counted lifecycle event.
//! The descriptor is deliberately independent of a card id: normal and golden
//! forms are rows with different scale values and share the same executor.
struct EventCounterSpec
{
    std::string_view id;
    std::string_view event;
    std::string_view threshold;
    std::string_view reset;
    std::string_view selector;
    std::string_view effect;
    int amount;
    int goldenScale;
    int health;
    int scaling;
    bool executable;
};

//! Small, deterministic counter primitive shared by event-driven effects.
//! Callers provide the event delta and receive the number of completed
//! thresholds; no card identity or simulator state is encoded here.
struct EventCounterState
{
    int value = 0;
};

inline int AdvanceEventCounter(EventCounterState& state, int delta,
                               int threshold, bool repeating) noexcept
{
    if (delta <= 0 || threshold <= 0)
        return 0;
    state.value += delta;
    if (!repeating)
    {
        if (state.value < threshold)
            return 0;
        state.value = threshold;
        return 1;
    }
    const int completed = state.value / threshold;
    state.value %= threshold;
    return completed;
}

inline constexpr EventCounterSpec EventCounterSpecs[] = {
    // The schema reserves "stat_buff_spell_scale" for rows whose stat grant is
    // scaled by a counted spell event; it is distinct from stat_buff.
    // Utility Drone: end-turn stat grant scales with each target's
    // magnetization count (the target selector is intentionally explicit).
    {"BG26_152", "turn_end", "magnetization", "lifetime", "magnetized_friendly_minions", "stat_buff_per_counter", 4, 1, 4, 1, true},
    {"BG26_152_G", "turn_end", "magnetization", "lifetime", "magnetized_friendly_minions", "stat_buff_per_counter", 8, 2, 8, 1, true},
    // Groundbreaker: Naga plays trigger the source; spell count improves the
    // grant once per three spells cast this game.
    {"BG31_035", "after_play_minion", "naga_play", "lifetime", "self", "spell_scaled_self_buff", 1, 1, 1, 3, true},
    {"BG31_035_G", "after_play_minion", "naga_play", "lifetime", "self", "spell_scaled_self_buff", 2, 2, 2, 3, true},
    // Dual-Wield Corsair uses a repeating per-turn gold threshold.
    {"BG31_824", "spend_gold", "gold", "per_turn", "random_friendly_pirates", "random_stat_buff", 4, 1, 5, 5, true},
    {"BG31_824_G", "spend_gold", "gold", "per_turn", "random_friendly_pirates", "random_stat_buff", 4, 2, 5, 5, true},
    // Fire-forged Evoker improves its start-combat grant permanently after a
    // Tavern spell; the executor owns the persistent improvement counter.
    {"BG32_822", "start_combat", "tavern_spell", "lifetime", "friendly_dragons", "persistent_stat_buff", 2, 1, 1, 1, true},
    {"BG32_822_G", "start_combat", "tavern_spell", "lifetime", "friendly_dragons", "persistent_stat_buff", 4, 2, 2, 1, true},
    // Additional pinned rows using the same counted-event schema.  Their
    // effect executors remain fail-closed until the corresponding simulator
    // primitive is available.
    {"BG26_810", "spend_gold", "gold", "per_turn", "friendly_pirates", "stat_buff", 2, 1, 0, 5, true},
    {"BG26_810_G", "spend_gold", "gold", "per_turn", "friendly_pirates", "stat_buff", 2, 2, 0, 5, true},
    {"BG27_083", "turn_end", "different_spell", "per_turn", "other_friendly_naga", "distinct_spell_race_buff", 2, 1, 1, 1, true},
    {"BG27_083_G", "turn_end", "different_spell", "per_turn", "other_friendly_naga", "distinct_spell_race_buff", 4, 2, 2, 1, true},
    {"BG31_925", "deathrattle", "spells_cast", "lifetime", "friendly_naga", "spell_scaled_race_buff", 2, 1, 1, 3, true},
    {"BG31_925_G", "deathrattle", "spells_cast", "lifetime", "friendly_naga", "spell_scaled_race_buff", 4, 2, 2, 3, true},
    {"BG33_334", "after_play_minion", "elemental_play", "per_turn", "played_elemental", "played_elemental_scale", 2, 1, 1, 1, true},
    {"BG33_334_G", "after_play_minion", "elemental_play", "per_turn", "played_elemental", "played_elemental_scale", 2, 2, 2, 1, true},
    {"BG35_334", "turn_end", "avenge_death", "lifetime", "friendly_minions", "progressive_avenge_end_turn", 1, 1, 1, 1, true},
    {"BG35_334_G", "turn_end", "avenge_death", "lifetime", "friendly_minions", "progressive_avenge_end_turn", 2, 2, 2, 1, true},
    {"BG36_851", "after_play_minion", "mech_play", "lifetime", "played_mech", "magnetize_and_improve", 2, 1, 2, 1, true},
    {"BG36_851_G", "after_play_minion", "mech_play", "lifetime", "played_mech", "magnetize_and_improve", 4, 2, 4, 1, true},
    {"BG34_780", "deathrattle", "tavern_spell", "lifetime", "highest_health_enemy", "damage_spell_scale", 2, 1, 0, 1, true},
    {"BG34_780_G", "deathrattle", "tavern_spell", "lifetime", "highest_health_enemy", "damage_spell_scale", 2, 2, 0, 1, true},
    {"BG35_890", "deathrattle", "magnetization", "lifetime", "friendly_mechs", "combat_stat_scale", 2, 1, 0, 1, true},
    {"BG35_890_G", "deathrattle", "magnetization", "lifetime", "friendly_mechs", "combat_stat_scale", 4, 2, 0, 1, true},
};

inline constexpr bool IsEventCounterExecutable(const EventCounterSpec& spec) noexcept
{
    return spec.executable;
}

//! Registers the descriptor-backed entries.  Unsupported effects remain
//! fail-closed (empty CardDef) until their executor is available.
class EventCounterBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif
