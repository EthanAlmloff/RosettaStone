#pragma once
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Player; class Minion; namespace SimpleTasks { class ArmFodderRefreshTask { public: explicit ArmFodderRefreshTask(int n): refreshes(n), perRefresh(1) {} ArmFodderRefreshTask(int n, int amount): refreshes(n), perRefresh(amount) {} TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&); int Refreshes() const noexcept { return refreshes; } int Amount() const noexcept { return perRefresh; } private:int refreshes; int perRefresh; }; }}
