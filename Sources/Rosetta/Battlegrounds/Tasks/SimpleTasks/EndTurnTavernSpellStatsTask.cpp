#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/EndTurnTavernSpellStatsTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus EndTurnTavernSpellStatsTask::Run(Player&p,Minion&) { p.season14.AddTavernSpellAttackBonus(m_attack); p.season14.AddTavernSpellHealthBonus(m_health); return TaskStatus::COMPLETE; } TaskStatus EndTurnTavernSpellStatsTask::Run(Player&p,Minion& source,Minion&) { return Run(p,source); } }
