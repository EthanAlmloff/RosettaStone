#include "doctest_proxy.hpp"
#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch32.hpp>
#include <Rosetta/Battlegrounds/Cards/Cards.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
using namespace RosettaStone::Battlegrounds;

TEST_CASE("[Batch32] - pinned static taunt and divine shield keywords") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch32::AddAll(cards);
  for (const auto* id : {"BGS_034", "TB_BaconUps_149"}) {
    REQUIRE(cards.contains(id));
    const auto card = Cards::FindCardByID(id); REQUIRE(card.id == id);
    Minion m{card};
    CHECK(m.HasDivineShield() || m.HasTaunt());
    if (std::string_view{id} == "BGS_034" || std::string_view{id} == "TB_BaconUps_149") CHECK(m.HasReborn());
  }
}

TEST_CASE("[Batch32] - unsupported effects stay out of registry") {
  std::map<std::string, CardDef> cards; ModernMinionBehaviorsBatch32::AddAll(cards);
  CHECK(!cards.contains("BG25_001")); CHECK(!cards.contains("BG26_175"));
  CHECK(!cards.contains("BG29_611")); CHECK(!cards.contains("BG31_803"));
  CHECK(!cards.contains("BG32_341")); CHECK(!cards.contains("BG32_341_G"));
}
