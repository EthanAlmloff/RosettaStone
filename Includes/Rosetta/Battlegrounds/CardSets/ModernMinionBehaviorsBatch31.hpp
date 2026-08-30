#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH31_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH31_HPP

#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>

namespace RosettaStone::Battlegrounds
{
class ModernMinionBehaviorsBatch31
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::Battlegrounds

#endif
