#ifndef ROSETTASTONE_BATTLEGROUNDS_MAGNETIZE_SATELLITE_TASK_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MAGNETIZE_SATELLITE_TASK_HPP
#include <Rosetta/Common/Enums/TaskEnums.hpp>
namespace RosettaStone::Battlegrounds { class Minion; class Player; namespace SimpleTasks {
class MagnetizeSatelliteTask { public:
 MagnetizeSatelliteTask(int attack,int health,int increment=0,int repeats=1):m_attack(attack),m_health(health),m_increment(increment),m_repeats(repeats){}
 TaskStatus Run(Player&,Minion&); TaskStatus Run(Player&,Minion&,Minion&);
 int Attack() const noexcept{return m_attack;} int Health() const noexcept{return m_health;} int Increment() const noexcept{return m_increment;} int Repeats() const noexcept{return m_repeats;}
 private:int m_attack,m_health,m_increment,m_repeats;
}; }}
#endif
