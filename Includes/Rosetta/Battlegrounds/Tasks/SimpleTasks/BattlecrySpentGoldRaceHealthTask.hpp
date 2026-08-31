#ifndef ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_SPENT_GOLD_RACE_HEALTH_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_SPENT_GOLD_RACE_HEALTH_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class BattlecrySpentGoldRaceHealthTask { public:
 explicit BattlecrySpentGoldRaceHealthTask(int amount):m_amount(amount){}
 TaskStatus Run(Player&,Minion&);
 TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_amount=0;
}; }}
#endif
