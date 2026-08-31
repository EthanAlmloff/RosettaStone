#pragma once
#include <map>
#include <string>
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
namespace RosettaStone::Battlegrounds {
class ModernMinionBehaviorsBatch32 { public: static void AddAll(std::map<std::string, CardDef>&); };
}
