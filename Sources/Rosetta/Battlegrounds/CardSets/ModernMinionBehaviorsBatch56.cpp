#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch56.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
void ModernMinionBehaviorsBatch56::AddAll(
    std::map<std::string, CardDef>& cards)
{
    // Tarecgosa has no task chain: combat copies already retain the full
    // enchantment list, so this registration makes the metadata-backed
    // dragon usable without inventing a second persistence path.
    cards.emplace("BG21_015", CardDef{});
    cards.emplace("BG21_015_G", CardDef{});
    // Lava Lurker's Spellcraft persistence is consumed by Player::PlaySpell
    // (Minion::ConsumeSpellcraftUse).  These empty definitions are deliberate:
    // CardLoader supplies the Naga/trigger metadata and the player lifecycle
    // supplies the once-per-turn (twice for golden) mutation.
    cards.emplace("BG23_009", CardDef{});
    cards.emplace("BG23_009_G", CardDef{});

    // Sellemental's sell lifecycle is self-scoped by Player::SellMinion;
    // AddCardTask creates the pinned Water Droplet (golden doubles it).
    Power normal;
    Trigger sold{TriggerType::SELL_MINION};
    sold.SetTriggerSource(TriggerSource::SELF);
    sold.SetTasks({SimpleTasks::AddCardTask{"BGS_115t", 1}});
    normal.AddTrigger(std::move(sold));
    cards.emplace("BGS_115", CardDef{std::move(normal)});
    Power golden;
    Trigger goldenSold{TriggerType::SELL_MINION};
    goldenSold.SetTriggerSource(TriggerSource::SELF);
    goldenSold.SetTasks({SimpleTasks::AddCardTask{"BGS_115t_G", 2}});
    golden.AddTrigger(std::move(goldenSold));
    cards.emplace("TB_BaconUps_156", CardDef{std::move(golden)});
}
}  // namespace RosettaStone::Battlegrounds
