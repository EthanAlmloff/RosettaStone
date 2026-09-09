#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch71.hpp>

#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/BattlecryTavernSpellDiscoverTask.hpp>
#include <utility>

namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch71::AddAll(std::map<std::string, CardDef>& cards) {
  // Outland Sunbeam discovers one (two while golden) Tavern spell costing at
  // least three.  The source DBF IDs are filtered by Player's canonical
  // Tavern-spell offering path; this keeps the modal sequential for golden.
  {
    Power p;
    p.AddBattlecryTask(SimpleTasks::BattlecryTavernSpellDiscoverTask{1});
    cards.emplace("BG31_HERO_003_Buddy", CardDef{std::move(p)});
  }
  {
    Power p;
    p.AddBattlecryTask(SimpleTasks::BattlecryTavernSpellDiscoverTask{2});
    cards.emplace("BG31_HERO_003_Buddy_G", CardDef{std::move(p)});
  }

  // Do not register the broad Buddy catalog here.  These rows have no
  // CardDef task graph in this batch, and an empty CardDef would make them
  // appear executable while silently dropping their phase-owned behavior.
  // Add each pair only with a focused Player/Battle lifecycle owner and test.
}
}
