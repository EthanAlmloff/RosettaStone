#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH16_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH16_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
namespace RosettaStone::Battlegrounds
{
//! Exact runtime-supported 36.4 economy minion family.
class ModernMinionBehaviorsBatch16
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}
#endif
