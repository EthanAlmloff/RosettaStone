#include <Rosetta/Battlegrounds/CardSets/EventCounterBehaviors.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomFriendlyRaceTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PersistentRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SpellCountRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/DistinctSpellRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/PlayedElementalScalingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/ProgressiveAvengeEndTurnTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/HighestHealthEnemyDamageTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/StartCombatSpellScaledRaceBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizationCountBuffTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MagnetizationCombatBuffTask.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Common/Enums/TriggerEnums.hpp>
#include <utility>

namespace RosettaStone::Battlegrounds
{
void EventCounterBehaviors::AddAll(std::map<std::string, CardDef>& cards)
{
    // Canonical descriptor ownership for BG26_152, BG26_810, BG31_035,
    // BG31_824, BG32_822, and BG36_851 (normal and golden rows) lives in
    // EventCounterBehaviors.hpp; this executor is their sole runtime path.
    // Fail-closed descriptor rows must never use the empty fallback:
    // cards.emplace(std::string(spec.id), CardDef{});
    // Keep registration data-driven.  Empty definitions are intentional for
    // effects whose simulator executor is not yet available; CardLoader can
    // still recognize the card while execution fails closed.
    for (const auto& spec : EventCounterSpecs)
    {
        if (!IsEventCounterExecutable(spec)) continue;
        if (spec.effect == "damage_spell_scale")
        {
            Power power;
            power.AddDeathrattleTask(SimpleTasks::HighestHealthEnemyDamageTask{
                spec.amount, spec.goldenScale, spec.scaling});
            cards.emplace(std::string(spec.id), CardDef{std::move(power)});
            continue;
        }
        if (spec.effect == "persistent_stat_buff")
        {
            Power power;
            power.AddStartCombatTask(SimpleTasks::StartCombatSpellScaledRaceBuffTask{
                Race::DRAGON, spec.amount, spec.health, spec.scaling});
            cards.emplace(std::string(spec.id), CardDef{std::move(power)});
            continue;
        }
        // The Corsair pair shares this descriptor-driven executor.  The
        // source/selector and threshold are represented by the row; the
        // existing Player spend hook supplies one trigger per completed
        // threshold and RandomFriendlyRaceTask performs the exact mutation.
        Power power;
        Trigger trigger{spec.event == "turn_end" ? TriggerType::TURN_END : TriggerType::SPEND_GOLD};
        trigger.SetTriggerSource(TriggerSource::SELF);
        if (spec.effect == "stat_buff_per_counter")
        {
            trigger.SetTasks({SimpleTasks::MagnetizationCountBuffTask{
                spec.amount, spec.health}});
            power.AddTrigger(std::move(trigger));
            cards.emplace(std::string(spec.id), CardDef{std::move(power)});
            continue;
        }
        if (spec.effect == "combat_stat_scale")
        {
            Power deathrattle;
            deathrattle.AddDeathrattleTask(
                SimpleTasks::MagnetizationCombatBuffTask{spec.amount, spec.scaling});
            cards.emplace(std::string(spec.id), CardDef{std::move(deathrattle)});
            continue;
        }
        if (spec.effect == "magnetize_and_improve")
        {
            trigger.SetTriggerSource(TriggerSource::FRIENDLY);
            trigger.SetCondition(SelfCondition{[](Minion& played) {
                return played.HasRace(Race::MECHANICAL);
            }});
            trigger.SetTasks({SimpleTasks::MagnetizeSatelliteTask{
                spec.amount, spec.health, spec.amount, spec.goldenScale}});
            power.AddTrigger(std::move(trigger));
            cards.emplace(std::string(spec.id), CardDef{std::move(power)});
            continue;
        }
        if (spec.effect == "random_stat_buff")
            trigger.SetTasks({SimpleTasks::RandomFriendlyRaceTask{
                Race::PIRATE, spec.amount, spec.health, spec.goldenScale}});
        // The two-argument form is represented by Race::PIRATE, spec.amount, spec.health}});
        else if (spec.effect == "stat_buff")
        {
            const bool naga = spec.threshold == "different_spell";
            trigger.SetTasks({SimpleTasks::PersistentRaceBuffTask{
                naga ? Race::NAGA : Race::PIRATE, spec.amount, spec.health, naga}});
        }
        else if (spec.effect == "spell_scaled_race_buff" || spec.effect == "spell_scaled_self_buff")
        {
            const bool selfBuff = spec.effect == "spell_scaled_self_buff";
            if (!selfBuff)
            {
                power.AddDeathrattleTask(SimpleTasks::SpellCountRaceBuffTask{
                    Race::NAGA, spec.amount, spec.health, spec.scaling, false});
                cards.emplace(std::string(spec.id), CardDef{std::move(power)});
                continue;
            }
            trigger = Trigger{TriggerType::AFTER_PLAY_MINION};
            trigger.SetTriggerSource(selfBuff ? TriggerSource::SELF : TriggerSource::SELF);
            if (selfBuff)
                trigger.SetCondition(SelfCondition{[](Minion& played) { return played.HasRace(Race::NAGA); }});
            trigger.SetTasks({SimpleTasks::SpellCountRaceBuffTask{
                Race::NAGA, spec.amount, spec.health, spec.scaling, selfBuff}});
        }
        else if (spec.effect == "distinct_spell_race_buff")
            trigger.SetTasks({SimpleTasks::DistinctSpellRaceBuffTask{spec.amount, spec.health, spec.scaling}});
        else if (spec.effect == "played_elemental_scale")
        {
            trigger.SetTriggerSource(TriggerSource::MINIONS_EXCEPT_SELF);
            trigger.SetCondition(SelfCondition{[](Minion& played) { return played.HasRace(Race::ELEMENTAL); }});
            trigger.SetTasks({SimpleTasks::PlayedElementalScalingTask{spec.amount, spec.health, spec.goldenScale}});
        }
        else if (spec.effect == "progressive_avenge_end_turn")
        {
            power.AddAvenge(AvengeDefinition{AvengeEffect::PROGRESSIVE_END_TURN, 1, spec.scaling, spec.scaling, Race::INVALID, true});
            trigger.SetTasks({SimpleTasks::ProgressiveAvengeEndTurnTask{}});
        }
        power.AddTrigger(std::move(trigger));
        cards.emplace(std::string(spec.id), CardDef{std::move(power)});
    }
}
}  // namespace RosettaStone::Battlegrounds
