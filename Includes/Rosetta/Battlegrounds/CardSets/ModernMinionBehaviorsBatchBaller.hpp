#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH_BALLER_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH_BALLER_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
namespace RosettaStone::Battlegrounds { class ModernMinionBehaviorsBatchBaller { public: static void AddAll(std::map<std::string, CardDef>&); }; }
#endif
