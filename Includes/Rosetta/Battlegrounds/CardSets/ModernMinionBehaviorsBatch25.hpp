#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH25_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH25_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
namespace RosettaStone::Battlegrounds
{
class ModernMinionBehaviorsBatch25
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}
#endif
