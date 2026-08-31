#include <Rosetta/Battlegrounds/CardSets/BloodGemBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>

#include <utility>

namespace RosettaStone::Battlegrounds
{
void BloodGemBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Canonical generated spell plus families whose complete effects are
    // resolved by Player or the shared gem generator task.
    for (const char* id : {
             "BG20_GEM", "BG20_203",
             "BG20_203_G", "BG20_205", "BG20_205_G", "BG20_207",
             "BG20_207_G", "BG20_301", "BG20_301_G", "BG20_103",
             "BG20_103_G", "BG20_102", "BG20_102_G", "BG20_105",
             "BG20_105_G" })
    {
        cards.emplace(id, CardDef{});
    }

    for (const auto [id, amount] : { std::pair{"BG20_105", 1},
                                    std::pair{"BG20_105_G", 2} })
    {
        Power power;
        power.AddBattlecryTask(SimpleTasks::GenerateBloodGemsTask{ amount });
        power.AddDeathrattleTask(SimpleTasks::GenerateBloodGemsTask{ amount });
        cards[id] = CardDef{ std::move(power) };
    }
}
}  // namespace RosettaStone::Battlegrounds
