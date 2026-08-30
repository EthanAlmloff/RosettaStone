#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch29.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <doctest/doctest.h>

using namespace RosettaStone::Battlegrounds;

TEST_CASE("Batch29 registers pinned Frenzy pairs with self combat triggers")
{
    std::map<std::string, CardDef> cards;
    ModernMinionBehaviorsBatch29::AddAll(cards);
    for (const auto* id : { "BG20_204", "BG20_204_G", "BG29_800", "BG29_800_G",
                            "BG29_846", "BG29_846_G", "BG34_312", "BG34_312_G" })
    {
        REQUIRE(cards.contains(id));
        REQUIRE(cards.at(id).power.GetTrigger().has_value());
        CHECK(cards.at(id).power.GetTrigger()->GetTriggerType() == TriggerType::TAKE_DAMAGE);
        CHECK(cards.at(id).power.GetTrigger()->GetTriggerSource() == TriggerSource::SELF);
    }
}
