// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_END_TURN_CONSUME_HIGHEST_TAVERN_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_END_TURN_CONSUME_HIGHEST_TAVERN_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class EndTurnConsumeHighestTavernTask { public:
 explicit EndTurnConsumeHighestTavernTask(int multiplier):m_multiplier(multiplier){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_multiplier=1; }; } }
#endif
