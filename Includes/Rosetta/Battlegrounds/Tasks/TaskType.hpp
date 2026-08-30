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
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FreeRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GainGoldTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ReduceTavernCostTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberEndTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberStartTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RebornAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTask.hpp>

#include <variant>

namespace RosettaStone::Battlegrounds
{
using TaskType =
    std::variant<SimpleTasks::AddCardTask, SimpleTasks::AddEnchantmentTask,
                 SimpleTasks::AddTavernCoinTask, SimpleTasks::BuyMinionTask, SimpleTasks::AttackTask,
                 SimpleTasks::CountTask, SimpleTasks::CounterBuffTask, SimpleTasks::DamageHeroTask,
                 SimpleTasks::DamageTask, SimpleTasks::GetGameTagTask,
                 SimpleTasks::FriendlyRaceEnchantmentTask,
                 SimpleTasks::FreeRefreshTask,
                 SimpleTasks::GainGoldTask,
                 SimpleTasks::GenerateBloodGemsTask,
                 SimpleTasks::IncludeTask, SimpleTasks::RandomTask,
                 SimpleTasks::RandomFriendlyRaceTask,
                 SimpleTasks::LeftmostFriendlyRaceTask,
                 SimpleTasks::ReduceTavernCostTask,
                 SimpleTasks::RepeatNumberEndTask,
                 SimpleTasks::RepeatNumberStartTask,
                 SimpleTasks::SetGameTagTask, SimpleTasks::SummonTask,
                 SimpleTasks::RallyBuffTask, SimpleTasks::RallyRaceBuffTask,
                 SimpleTasks::RebornAttackTask>;
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP
