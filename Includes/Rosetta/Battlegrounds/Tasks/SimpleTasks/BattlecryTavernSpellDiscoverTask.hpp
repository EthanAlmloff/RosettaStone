// Copyright (c) 2026 Hearthstone BG AI contributors
#ifndef ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_TAVERN_SPELL_DISCOVER_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_BATTLECRY_TAVERN_SPELL_DISCOVER_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player;
namespace SimpleTasks { class BattlecryTavernSpellDiscoverTask { public:
 explicit BattlecryTavernSpellDiscoverTask(int amount):m_amount(amount){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 private:int m_amount=0; }; } }
#endif
