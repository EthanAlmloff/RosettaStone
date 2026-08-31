// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP
#define ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP

#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
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
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomHandMinionBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/LeapfroggerDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSummonFromPoolTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberEndTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RepeatNumberStartTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentBeetleBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChromadrakeToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizeSatelliteTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellDiscountTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnTavernSpellStatsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecrySpentGoldRaceHealthTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentTavernTierBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmRefreshRandomShopStatsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmFodderRefreshTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/TriggerLeftmostDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTrackedAvengeCardsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastSelfScalingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastOnSelfHealthTask.hpp>
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
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomTavernSpellToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DestroyUndeadBuffSelfTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateFishbaitTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ActivateRandomTavernSpellsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DealDamageSelfBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnStatTransferTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyHighestHandSummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyHighestHandMurlocBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemGolemTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemSelfTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnLastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemOtherTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyBloodGemAttackerTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCastAdjacentBloodGemTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpendGoldThresholdSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HeroDamageThresholdSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyGainTargetAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRemoveKeywordsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RallyRandomRaceKeywordTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ConfigureAttackThresholdTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellDiscoverTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnConsumeHighestTavernTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomGoldenTierMinionToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellAttackBonusTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GrowingSummonAttackTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DarkGiftRandomPoolTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DarkGiftGolemDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHandStatsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHighestHandMurlocSummonTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ExactCopyDeathrattleTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatDestroyAdjacentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatHandSelfCopyTask.hpp>

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
                 SimpleTasks::RandomFriendlyRaceTask, SimpleTasks::RandomEnemyDamageTask,
                 SimpleTasks::RandomHandMinionBuffTask,
                 SimpleTasks::RandomCardToHandTask,
                 SimpleTasks::LeapfroggerDeathrattleTask,
                 SimpleTasks::RandomSummonFromPoolTask,
                 SimpleTasks::LeftmostFriendlyRaceTask,
                 SimpleTasks::ReduceTavernCostTask,
                 SimpleTasks::RepeatNumberEndTask,
                 SimpleTasks::RepeatNumberStartTask,
                 SimpleTasks::SetGameTagTask, SimpleTasks::SummonTask,
                 SimpleTasks::RallyBuffTask, SimpleTasks::RallyRaceBuffTask,
                 SimpleTasks::CastTavernSpellTask,
                 SimpleTasks::RandomChooseOneCardToHandTask,
                 SimpleTasks::MagnetizeSatelliteTask,
                 SimpleTasks::RebornAttackTask,
                  SimpleTasks::PersistentRaceBuffTask, SimpleTasks::PersistentBeetleBuffTask, SimpleTasks::RandomChromadrakeToHandTask, SimpleTasks::BattlecryTavernSpellDiscountTask, SimpleTasks::EndTurnTavernSpellStatsTask, SimpleTasks::BattlecrySpentGoldRaceHealthTask, SimpleTasks::PersistentTavernTierBuffTask, SimpleTasks::ArmRefreshRandomShopStatsTask, SimpleTasks::ArmFodderRefreshTask, SimpleTasks::TriggerLeftmostDeathrattleTask, SimpleTasks::SummonTrackedAvengeCardsTask, SimpleTasks::SpellCastSelfScalingTask, SimpleTasks::SpellCastOnSelfHealthTask, SimpleTasks::ApplyMinionStatBuffTask, SimpleTasks::AttackingMinionBuffTask,
                 SimpleTasks::OnePerTypeRallyBuffTask,
                 SimpleTasks::SummonedMinionStatMultiplierTask,
                 SimpleTasks::RallyTavernSpellHealthBonusTask,
                 SimpleTasks::RandomTavernSpellToHandTask, SimpleTasks::DestroyUndeadBuffSelfTask, SimpleTasks::ActivateFishbaitTask, SimpleTasks::ActivateRandomTavernSpellsTask, SimpleTasks::DealDamageSelfBuffTask,
                 SimpleTasks::EndTurnStatTransferTask,
                 SimpleTasks::RallyHighestHandSummonTask,
                 SimpleTasks::RallyHighestHandMurlocBuffTask,
                 SimpleTasks::RallyBloodGemGolemTask,
                 SimpleTasks::RallyBloodGemSelfTask,
                 SimpleTasks::EndTurnLastTavernSpellTask,
                 SimpleTasks::RallyBloodGemOtherTask,
                 SimpleTasks::RallyBloodGemAttackerTask,
                 SimpleTasks::SpellCastAdjacentBloodGemTask,
                 SimpleTasks::SpendGoldThresholdSpellTask,
                 SimpleTasks::HeroDamageThresholdSpellTask,
                 SimpleTasks::RallyGainTargetAttackTask,
                 SimpleTasks::RallyRemoveKeywordsTask,
                 SimpleTasks::RallyRandomRaceKeywordTask,
                 SimpleTasks::RandomBountyToHandTask,
                 SimpleTasks::ConfigureAttackThresholdTask,
                 SimpleTasks::BattlecryTavernSpellDiscoverTask,
                 SimpleTasks::EndTurnConsumeHighestTavernTask,
                 SimpleTasks::RandomGoldenTierMinionToHandTask,
                 SimpleTasks::BattlecryTavernSpellAttackBonusTask,
                 SimpleTasks::GrowingSummonAttackTask,
                 SimpleTasks::MinionOfferingTask, SimpleTasks::DarkGiftRandomPoolTask,
                 SimpleTasks::DarkGiftGolemDeathrattleTask,
                 SimpleTasks::StartCombatHandStatsTask,
                 SimpleTasks::StartCombatHighestHandMurlocSummonTask,
                 SimpleTasks::ExactCopyDeathrattleTask,
                 SimpleTasks::StartCombatDestroyAdjacentTask,
                 SimpleTasks::StartCombatHandSelfCopyTask>;
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_TASK_TYPE_HPP




