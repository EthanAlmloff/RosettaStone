#ifndef ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH21_HPP
#define ROSETTASTONE_BATTLEGROUNDS_MODERN_MINION_BEHAVIORS_BATCH21_HPP
#include <Rosetta/Battlegrounds/Cards/CardDef.hpp>
#include <map>
#include <string>
#include <string_view>
namespace RosettaStone::Battlegrounds
{
struct SellBehaviorBatch21 { int tavernCoins = 0; };
inline SellBehaviorBatch21 FindSellBehaviorBatch21(std::string_view id)
{
    if (id == "BG34_230") return { 1 };
    if (id == "BG34_230_G") return { 2 };
    return {};
}
class ModernMinionBehaviorsBatch21 { public: static void AddAll(std::map<std::string, CardDef>&); };
}
#endif
