#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks {
class RallyHighestHandMurlocBuffTask { public: explicit RallyHighestHandMurlocBuffTask(int repeats):m_repeats(repeats){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int m_repeats;};}}
