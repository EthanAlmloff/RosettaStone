#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks { class ConfigureAttackThresholdTask { public: explicit ConfigureAttackThresholdTask(int t):threshold(t){} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int threshold; }; }}
