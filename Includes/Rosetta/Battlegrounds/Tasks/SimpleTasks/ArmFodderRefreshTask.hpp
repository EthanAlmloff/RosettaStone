#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks { class ArmFodderRefreshTask { public: explicit ArmFodderRefreshTask(int n): refreshes(n), perRefresh(1) {} ArmFodderRefreshTask(int n, int amount): refreshes(n), perRefresh(amount) {} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); private:int refreshes; int perRefresh; }; }}
