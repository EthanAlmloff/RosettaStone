// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP

#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddTavernCoinTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BuyMinionTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CountTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CounterBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageHeroTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GetGameTagTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/IncludeTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeftmostFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HandRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BloodGemRaceBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ReduceTavernCostTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomHandMinionBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberEndTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberStartTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastSpellBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AttackingMinionBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/OnePerTypeRallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RebornAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonedMinionStatMultiplierTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyTavernSpellHealthBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GrowingSummonAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>

#include <variant>

namespace RosettaStone::Battlegrounds
{
using TaskType =
    std::variant<SimpleTasks::AddCardTask, SimpleTasks::AddEnchantmentTask,
                 SimpleTasks::AddTavernCoinTask, SimpleTasks::BuyMinionTask, SimpleTasks::AttackTask,
                 SimpleTasks::CountTask, SimpleTasks::CounterBuffTask, SimpleTasks::DamageHeroTask,
                 SimpleTasks::DamageTask, SimpleTasks::GetGameTagTask,
                 SimpleTasks::FriendlyRaceEnchantmentTask,
                 SimpleTasks::HandRaceBuffTask,
                 SimpleTasks::FreeRefreshTask,
                 SimpleTasks::GainGoldTask,
                 SimpleTasks::GenerateBloodGemsTask,
                 SimpleTasks::BloodGemRaceBonusTask,
                 SimpleTasks::IncludeTask, SimpleTasks::RandomTask,
                 SimpleTasks::RandomFriendlyRaceTask,
                 SimpleTasks::RandomHandMinionBuffTask,
                 SimpleTasks::RandomCardToHandTask,
                 SimpleTasks::LeftmostFriendlyRaceTask,
                 SimpleTasks::ReduceTavernCostTask,
                 SimpleTasks::RepeatNumberEndTask,
                 SimpleTasks::RepeatNumberStartTask,
                 SimpleTasks::SetGameTagTask, SimpleTasks::SummonTask,
                 SimpleTasks::RallyBuffTask, SimpleTasks::RallyRaceBuffTask,
                 SimpleTasks::RebornAttackTask,
                 SimpleTasks::PersistentRaceBuffTask, SimpleTasks::CastSpellBuffTask, SimpleTasks::AttackingMinionBuffTask,
                 SimpleTasks::OnePerTypeRallyBuffTask,
                 SimpleTasks::SummonedMinionStatMultiplierTask,
                 SimpleTasks::RallyTavernSpellHealthBonusTask,
                 SimpleTasks::GrowingSummonAttackTask,
                 SimpleTasks::MinionOfferingTask>;
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP
