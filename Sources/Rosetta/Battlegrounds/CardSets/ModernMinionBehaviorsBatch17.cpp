#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch17.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddRaceBattlecry(std::map<std::string, CardDef>& cards, const char* id,
                      const char* enchantment, Race race, int attack,
                      int health)
{
    Power power;
    power.AddBattlecryTask(SimpleTasks::FriendlyRaceEnchantmentTask{
        enchantment, race, true });
    cards.emplace(id, CardDef{ std::move(power) });
    std::vector<Effect> effects;
    if (attack != 0) effects.emplace_back(Effects::AttackN(attack));
    if (health != 0) effects.emplace_back(Effects::HealthN(health));
    Power enchant;
    enchant.AddEnchant(Enchant{ std::move(effects) });
    cards.emplace(enchantment, CardDef{ std::move(enchant) });
}
}
void ModernMinionBehaviorsBatch17::AddAll(std::map<std::string, CardDef>& cards)
{
    AddRaceBattlecry(cards, "BG29_502", "BG17_502e", Race::PIRATE, 0, 4);
    AddRaceBattlecry(cards, "BG29_502_G", "BG17_502Ge", Race::PIRATE, 0, 8);
    AddRaceBattlecry(cards, "BG_EX1_103", "BG17_EX103e", Race::MURLOC, 0, 2);
    AddRaceBattlecry(cards, "TB_BaconUps_064", "BG17_EX103Ge", Race::MURLOC, 0, 4);
    AddRaceBattlecry(cards, "BG_GVG_048", "BG17_GVG048e", Race::MECHANICAL, 2, 0);
    AddRaceBattlecry(cards, "TB_BaconUps_066", "BG17_GVG048Ge", Race::MECHANICAL, 4, 0);
    AddRaceBattlecry(cards, "BGS_053", "BG17_BGS053e", Race::PIRATE, 3, 0);
    AddRaceBattlecry(cards, "TB_BaconUps_138", "BG17_BGS053Ge", Race::PIRATE, 6, 0);
    AddRaceBattlecry(cards, "BG34_636t", "BG17_34636e", Race::DRAGON, 1, 3);
    AddRaceBattlecry(cards, "BG34_636_Gt", "BG17_34636Ge", Race::DRAGON, 2, 6);
    AddRaceBattlecry(cards, "BG34_637t", "BG17_34637e", Race::DRAGON, 3, 1);
    AddRaceBattlecry(cards, "BG34_637_Gt", "BG17_34637Ge", Race::DRAGON, 6, 2);
}
}
