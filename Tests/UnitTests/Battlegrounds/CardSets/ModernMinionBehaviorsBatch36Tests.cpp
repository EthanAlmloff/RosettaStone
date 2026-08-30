#include "doctest_proxy.hpp"

#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch36.hpp>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch36] - deterministic Choose One source and option registry")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch36::AddAll(cards);
    for (const auto* id : {"BG27_084", "BG27_084_G", "BG27_084t",
                           "BG27_084_Gt", "BG27_084t2", "BG27_084_Gt2",
                           "BG30_123", "BG30_123_G", "BG30_123t",
                           "BG30_123_Gt", "BG30_123t2", "BG30_123_Gt2",
                           "BG36_330", "BG36_330_G", "BG36_330t",
                           "BG36_330_Gt", "BG36_330t2", "BG36_330_Gt2"})
        CHECK(cards.contains(id));
}
