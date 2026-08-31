#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch42.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ArmFodderRefreshTask.hpp>
#include <utility>
#include <tuple>
namespace RosettaStone::Battlegrounds { void ModernMinionBehaviorsBatch42::AddAll(std::map<std::string,CardDef>& cards) { Power normal; normal.AddDeathrattleTask(SimpleTasks::ArmFodderRefreshTask{3, 1}); cards.emplace("BG36_730", CardDef{std::move(normal)}); Power golden; golden.AddDeathrattleTask(SimpleTasks::ArmFodderRefreshTask{3, 2}); cards.emplace("BG36_730_G", CardDef{std::move(golden)}); } }
