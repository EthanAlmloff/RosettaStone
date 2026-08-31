#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class RallyHighestHandSummonTask { public: explicit RallyHighestHandSummonTask(int count):m_count(count){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_count;};}}
