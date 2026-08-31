#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecrySpentGoldRaceHealthTask.hpp>
#include <Rosetta/Battlegrounds/Models/Player.hpp>
namespace RosettaStone::Battlegrounds::SimpleTasks { TaskStatus BattlecrySpentGoldRaceHealthTask::Run(Player&p,Minion&) { const int bonus=m_amount+p.season14.goldSpentThisTurn; p.recruitField.ForEachAlive([&](MinionData& d){if(d.value().HasRace(Race::PIRATE)){d.value().SetHealth(d.value().GetHealth()+bonus);}}); return TaskStatus::COMPLETE; } TaskStatus BattlecrySpentGoldRaceHealthTask::Run(Player&p,Minion& source,Minion&) { return Run(p,source); } }
