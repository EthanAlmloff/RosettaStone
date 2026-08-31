#include <Rosetta/Battlegrounds/CardSets/ModernMinionBehaviorsBatch53.hpp>
#include <Rosetta/Battlegrounds/Enchants/Power.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomChooseOneCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastTavernSpellTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizeSatelliteTask.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
namespace RosettaStone::Battlegrounds {
void ModernMinionBehaviorsBatch53::AddAll(std::map<std::string, CardDef>& cards) {
    Power normal; normal.AddActivate({ActivateEffect::TRIGGER_RALLY, 1, 0, 0, 1});
    cards.emplace("BG36_243", CardDef{std::move(normal)});
    Power golden; golden.AddActivate({ActivateEffect::TRIGGER_RALLY, 1, 0, 0, 2});
    cards.emplace("BG36_243_G", CardDef{std::move(golden)});
    Power tunneler; tunneler.AddRallyTask(SimpleTasks::RandomChooseOneCardToHandTask{1});
    cards.emplace("BG36_331", CardDef{std::move(tunneler)});
    Power tunnelerGolden; tunnelerGolden.AddRallyTask(SimpleTasks::RandomChooseOneCardToHandTask{2});
    cards.emplace("BG36_331_G", CardDef{std::move(tunnelerGolden)});
    Power vindicator; vindicator.AddRallyTask(SimpleTasks::CastTavernSpellTask{"BG36_246", 1});
    cards.emplace("BG36_241", CardDef{std::move(vindicator)});
    Power vindicatorGolden; vindicatorGolden.AddRallyTask(SimpleTasks::CastTavernSpellTask{"BG36_246", 2});
    cards.emplace("BG36_241_G", CardDef{std::move(vindicatorGolden)});
    Power glambot;
    { Trigger t{TriggerType::AFTER_CAST_SPELL}; t.SetTriggerSource(TriggerSource::FRIENDLY);
      t.SetCondition(SelfCondition{[](Minion& target){ return target.HasRace(Race::MECHANICAL); }});
      t.SetTasks({SimpleTasks::MagnetizeSatelliteTask{4,4}}); glambot.AddTrigger(std::move(t)); }
    cards.emplace("BG36_853", CardDef{std::move(glambot)});
    Power glambotGolden;
    { Trigger t{TriggerType::AFTER_CAST_SPELL}; t.SetTriggerSource(TriggerSource::FRIENDLY);
      t.SetCondition(SelfCondition{[](Minion& target){ return target.HasRace(Race::MECHANICAL); }});
      t.SetTasks({SimpleTasks::MagnetizeSatelliteTask{6,6,0,2}}); glambotGolden.AddTrigger(std::move(t)); }
    cards.emplace("BG36_853_G", CardDef{std::move(glambotGolden)});
    Power snapper;
    { Trigger t{TriggerType::AFTER_PLAY_MINION}; t.SetTriggerSource(TriggerSource::FRIENDLY);
      t.SetCondition(SelfCondition{[](Minion& target){ return target.HasRace(Race::MECHANICAL); }});
      t.SetTasks({SimpleTasks::MagnetizeSatelliteTask{2,2,2}}); snapper.AddTrigger(std::move(t)); }
    cards.emplace("BG36_851", CardDef{std::move(snapper)});
    Power snapperGolden;
    { Trigger t{TriggerType::AFTER_PLAY_MINION}; t.SetTriggerSource(TriggerSource::FRIENDLY);
      t.SetCondition(SelfCondition{[](Minion& target){ return target.HasRace(Race::MECHANICAL); }});
      t.SetTasks({SimpleTasks::MagnetizeSatelliteTask{4,4,4}}); snapperGolden.AddTrigger(std::move(t)); }
    cards.emplace("BG36_851_G", CardDef{std::move(snapperGolden)});
    Power duplicator; duplicator.AddActivate({ActivateEffect::ARM_MAGNETIZATION, 1});
    cards.emplace("BG36_506", CardDef{std::move(duplicator)});
    Power duplicatorGolden; duplicatorGolden.AddActivate({ActivateEffect::ARM_MAGNETIZATION, 1});
    cards.emplace("BG36_506_G", CardDef{std::move(duplicatorGolden)});
}
}
