#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH20_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH20_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
namespace RosettaStone::Battlegrounds
{
//! Exact one-task economy family from pinned 36.4 data.
class ModernMinionBehaviorsBatch20
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}
#endif
