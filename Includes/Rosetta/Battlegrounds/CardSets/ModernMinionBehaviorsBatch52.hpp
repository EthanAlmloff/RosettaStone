#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH52_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH52_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
namespace RosettaStone::Battlegrounds {
//! Patch 36.4 Deft Deserter: recruit-only Tavern-wide Activate primitive.
class ModernMinionBehaviorsBatch52 { public: static void AddAll(std::map<std::string, CardDef>& cards); };
}
#endif
