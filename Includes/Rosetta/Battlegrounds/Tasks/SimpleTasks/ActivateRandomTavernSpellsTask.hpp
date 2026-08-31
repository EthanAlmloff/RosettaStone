#ifndef ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_RANDOM_TAVERN_SPELLS_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_ACTIVATE_RANDOM_TAVERN_SPELLS_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks { class ActivateRandomTavernSpellsTask { public: explicit ActivateRandomTavernSpellsTask(int amount):m_amount(amount){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_amount; }; }}
#endif
