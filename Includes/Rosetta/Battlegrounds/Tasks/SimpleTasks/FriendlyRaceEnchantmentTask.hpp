// Copyright (c) 2026 Hearthstone BG AI contributors

#ifndef ROSETTASTONE_BATTLEGROUNDS_FRIENDLY_RACE_ENCHANTMENT_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_FRIENDLY_RACE_ENCHANTMENT_TASK_HPP

#include <Rosetta/Common/Enums/CardEnums.hpp>
#include <Rosetta/Common/Enums/TaskEnums.hpp>

#include <string_view>

namespace RosettaStone::Battlegrounds
{
class Minion;
class Player;

namespace SimpleTasks
{
//! Applies an enchantment to every living friendly minion of one tribe.
//!
//! This is intentionally narrower than a general selector.  It covers the
//! deterministic race-wide buffs used by the verified modern minion batch
//! while keeping the race filter in the simulator rather than approximating
//! it with a whole-board AddEnchantmentTask.
class FriendlyRaceEnchantmentTask
{
 public:
    //! Constructs a race-filtered friendly-board enchantment task.
    explicit FriendlyRaceEnchantmentTask(const std::string_view& cardID,
                                         Race race,
                                         bool excludeSource = false);

    //! Runs the task against the source minion.
    TaskStatus Run(Player& player, Minion& source);

    //! Runs the task against an explicit target (the target is ignored).
    TaskStatus Run(Player& player, Minion& source, Minion& target);

    std::string_view CardID() const noexcept { return m_cardID; }
    Race GetRace() const noexcept { return m_race; }
    bool ExcludesSource() const noexcept { return m_excludeSource; }

 private:
    std::string_view m_cardID;
    Race m_race = Race::INVALID;
    bool m_excludeSource = false;
};
}  // namespace SimpleTasks
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_FRIENDLY_RACE_ENCHANTMENT_TASK_HPP
