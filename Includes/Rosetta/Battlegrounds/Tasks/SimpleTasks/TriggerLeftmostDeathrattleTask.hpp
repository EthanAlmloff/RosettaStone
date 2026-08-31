#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks { class TriggerLeftmostDeathrattleTask { public: TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); }; }}
