#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch25.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/FriendlyRaceEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddCardTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DamageHeroTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SetGameTagTask.hpp>
#include <utility>
#include <vector>
#include <optional>
namespace RosettaStone::Battlegrounds
{
namespace
{
void AddEnchant(std::map<std::string, CardDef>& cards, const char* id,
                int attack, int health)
{
    std::vector<Effect> effects;
    if (attack) effects.emplace_back(Effects::AttackN(attack));
    if (health) effects.emplace_back(Effects::HealthN(health));
    Power power;
    power.AddEnchant(Enchant{ std::move(effects) });
    cards.emplace(id, CardDef{ std::move(power) });
}
void AddTarget(std::map<std::string, CardDef>& cards, const char* id,
               const char* enchant, std::optional<Race> race, bool taunt = false)
{
    Power power;
    power.AddBattlecryTask(SimpleTasks::AddEnchantmentTask{ enchant, EntityType::TARGET });
    if (taunt)
        power.AddBattlecryTask(SimpleTasks::SetGameTagTask{ EntityType::TARGET, GameTag::TAUNT, 1 });
    std::map<PlayReq, int> reqs{ { PlayReq::REQ_TARGET_IF_AVAILABLE, 0 },
                                  { PlayReq::REQ_MINION_TARGET, 0 },
                                  { PlayReq::REQ_FRIENDLY_TARGET, 0 } };
    if (race) reqs.emplace(PlayReq::REQ_TARGET_WITH_RACE, static_cast<int>(*race));
    cards.emplace(id, CardDef{ std::move(power), std::move(reqs) });
}
void AddRaceBoth(std::map<std::string, CardDef>& cards, const char* id,
                 const char* enchant, Race race)
{
    Power power;
    power.AddBattlecryTask(SimpleTasks::FriendlyRaceEnchantmentTask{ enchant, race, true });
    power.AddDeathrattleTask(SimpleTasks::FriendlyRaceEnchantmentTask{ enchant, race, true });
    cards.emplace(id, CardDef{ std::move(power) });
}
void AddOtherRaceDamage(std::map<std::string, CardDef>& cards, const char* id,
                        const char* enchant, Race race, int repeats)
{
    Power power;
    for (int i = 0; i < repeats; ++i)
    {
        power.AddBattlecryTask(SimpleTasks::FriendlyRaceEnchantmentTask{ enchant, race, true });
        power.AddBattlecryTask(SimpleTasks::DamageHeroTask{ 2 });
    }
    cards.emplace(id, CardDef{ std::move(power) });
}
void AddOtherMinionsBoth(std::map<std::string, CardDef>& cards, const char* id,
                         const char* enchant)
{
    Power power;
    power.AddBattlecryTask(SimpleTasks::AddEnchantmentTask{ enchant, EntityType::MINIONS_NOSOURCE });
    power.AddDeathrattleTask(SimpleTasks::AddEnchantmentTask{ enchant, EntityType::MINIONS_NOSOURCE });
    power.AddRallyTask(SimpleTasks::AddEnchantmentTask{ enchant, EntityType::MINIONS_NOSOURCE });
    cards.emplace(id, CardDef{ std::move(power) });
}
}
void ModernMinionBehaviorsBatch25::AddAll(std::map<std::string, CardDef>& cards)
{
    AddEnchant(cards, "BG25_004e", 2, 7); AddTarget(cards, "BG25_004", "BG25_004e", Race::UNDEAD);
    AddEnchant(cards, "BG25_004Ge", 4, 14); AddTarget(cards, "BG25_004_G", "BG25_004Ge", Race::UNDEAD);
    AddEnchant(cards, "BG26_522e", 2, 2); AddOtherRaceDamage(cards, "BG26_522", "BG26_522e", Race::DEMON, 1);
    AddEnchant(cards, "BG26_522Ge", 2, 2); AddOtherRaceDamage(cards, "BG26_522_G", "BG26_522Ge", Race::DEMON, 2);
    AddEnchant(cards, "BG30_756e", 2, 3); AddTarget(cards, "BG30_756", "BG30_756e", Race::MURLOC, true);
    AddEnchant(cards, "BG30_756Ge", 4, 6); AddTarget(cards, "BG30_756_G", "BG30_756Ge", Race::MURLOC, true);
    AddEnchant(cards, "BG32_824e", 10, 0); AddRaceBoth(cards, "BG32_824", "BG32_824e", Race::DRAGON);
    AddEnchant(cards, "BG32_824Ge", 20, 0); AddRaceBoth(cards, "BG32_824_G", "BG32_824Ge", Race::DRAGON);
    AddEnchant(cards, "BG33_701e", 2, 2); AddOtherMinionsBoth(cards, "BG33_701", "BG33_701e");
    AddEnchant(cards, "BG33_701Ge", 4, 4); AddOtherMinionsBoth(cards, "BG33_701_G", "BG33_701Ge");
    {
        Power normal; normal.AddBattlecryTask(SimpleTasks::AddCardTask{ "BG27_002t", 2 });
        cards.emplace("BG27_002", CardDef{ std::move(normal) });
        Power golden; golden.AddBattlecryTask(SimpleTasks::AddCardTask{ "BG27_002t", 4 });
        cards.emplace("BG27_002_G", CardDef{ std::move(golden) });
    }
    {
        Power normal; normal.AddBattlecryTask(SimpleTasks::SetGameTagTask{ EntityType::TARGET, GameTag::TAUNT, 1 });
        cards.emplace("BG_AT_069", CardDef{ std::move(normal), {{ PlayReq::REQ_TARGET_IF_AVAILABLE, 0 }, { PlayReq::REQ_MINION_TARGET, 0 }} });
        Power golden; golden.AddBattlecryTask(SimpleTasks::SetGameTagTask{ EntityType::TARGET, GameTag::TAUNT, 1 });
        cards.emplace("BG_AT_069_G", CardDef{ std::move(golden), {{ PlayReq::REQ_TARGET_IF_AVAILABLE, 0 }, { PlayReq::REQ_MINION_TARGET, 0 }} });
    }
}
}
