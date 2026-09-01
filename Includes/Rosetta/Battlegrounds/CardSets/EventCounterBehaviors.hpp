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
    // Utility Drone: end-turn stat grant scales with each target's
    // magnetization count (the target selector is intentionally explicit).
    {"BG26_152", "turn_end", "magnetization", "lifetime", "magnetized_friendly_minions", "stat_buff_per_counter", 4, 1, 4, 1},
    {"BG26_152_G", "turn_end", "magnetization", "lifetime", "magnetized_friendly_minions", "stat_buff_per_counter", 8, 2, 8, 1},
    // Groundbreaker: Naga plays trigger the source; spell count improves the
    // grant once per three spells cast this game.
    {"BG31_035", "after_play_minion", "naga_play", "lifetime", "self", "stat_buff_spell_scale", 1, 1, 1, 3},
    {"BG31_035_G", "after_play_minion", "naga_play", "lifetime", "self", "stat_buff_spell_scale", 2, 2, 2, 3},
    // Dual-Wield Corsair uses a repeating per-turn gold threshold.
    {"BG31_824", "spend_gold", "gold", "per_turn", "random_friendly_pirates", "random_stat_buff", 4, 1, 5, 5},
    {"BG31_824_G", "spend_gold", "gold", "per_turn", "random_friendly_pirates", "random_stat_buff", 4, 2, 5, 5},
    // Fire-forged Evoker improves its start-combat grant permanently after a
    // Tavern spell; the executor owns the persistent improvement counter.
    {"BG32_822", "start_combat", "tavern_spell", "lifetime", "friendly_dragons", "persistent_stat_buff", 2, 1, 1, 1},
    {"BG32_822_G", "start_combat", "tavern_spell", "lifetime", "friendly_dragons", "persistent_stat_buff", 4, 2, 2, 1},
};

//! Registers the descriptor-backed entries.  Unsupported effects remain
//! fail-closed (empty CardDef) until their executor is available.
class EventCounterBehaviors
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif
