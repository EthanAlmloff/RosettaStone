// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef ROSETTASTONE_BATTLEGROUNDS_POWER_HPP
#define ROSETTASTONE_BATTLEGROUNDS_POWER_HPP

#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Tasks/TaskType.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>

#include <optional>
#include <string>
#include <vector>

namespace RosettaStone::Battlegrounds
{
//! The small, explicitly supported subset of manual Activate effects.
//! Complex choice/random effects remain unregistered and therefore fail closed.
enum class ActivateEffect : unsigned char
{
    NONE,
    BUFF_TARGET,
    SET_TARGET_STATS,
    GAIN_GOLD,
    ADD_CARD,
    RANDOM_CARD,
    RANDOM_CHROMADRAKE,
    TAVERN_STATS_RANDOM_KEYWORD,
    TRIGGER_RALLY,
    TRIGGER_BATTLECRY,
    TAKE_HIGHEST_TAVERN,
    DEVOUR_TAVERN_DEMONS,
    ARM_MAGNETIZATION,
    GAIN_NEXT_BOUGHT_STATS,
    DISCOVER_TAVERN_SPELL,
    APPLY_REBORN,
    DESTROY_UNDEAD_BUFF_SELF,
    ACTIVATE_FISHBAIT,
    ACTIVATE_RANDOM_TAVERN_SPELLS,
};
enum class AvengeEffect : unsigned char { NONE, BUFF_SELF, BUFF_RACE, ADD_CARD, ADD_RANDOM_UNDEAD, PROGRESSIVE_END_TURN };
struct AvengeDefinition
{
    AvengeEffect effect = AvengeEffect::NONE;
    int threshold = 0;
    int attack = 0;
    int health = 0;
    Race race = Race::INVALID;
    bool permanent = false;
    std::string cardID;
    int cardCount = 1;
};

struct ActivateDefinition
{
    ActivateEffect effect = ActivateEffect::NONE;
    int cost = 0;
    int attack = 0;
    int health = 0;
    int amount = 0;
    bool nextTurn = false;
    std::string cardID;
    Race race = Race::INVALID;
};

//!
//! \brief Power class.
//!
//! This class stores task to perform power and enchant to apply it.
//!
class Power
{
 public:
    //! Clears a list of tasks and aura/enchant/trigger.
    void ClearData();

    //! Returns a list of battlecry tasks.
    //! \return A list of battlecry tasks.
    std::vector<TaskType>& GetBattlecryTask();
    const std::vector<TaskType>& GetBattlecryTask() const;

    //! Returns a list of start of combat tasks.
    //! \return A list of start of combat tasks.
    std::vector<TaskType>& GetStartCombatTask();
    const std::vector<TaskType>& GetStartCombatTask() const;

    //! Returns a list of deathrattle tasks.
    //! \return A list of deathrattle tasks.
    std::vector<TaskType>& GetDeathrattleTask();
    const std::vector<TaskType>& GetDeathrattleTask() const;

    //! Returns Rally tasks, resolved when this minion's friendly attacker
    //! declares an attack.
    std::vector<TaskType>& GetRallyTask();

    //! Returns the explicit manual Activate definition, when present.
    std::optional<ActivateDefinition>& GetActivate();
    const std::optional<ActivateDefinition>& GetActivate() const;
    std::optional<AvengeDefinition>& GetAvenge();
    const std::optional<AvengeDefinition>& GetAvenge() const;
    void AddAvenge(AvengeDefinition definition);

    //! Returns enchant.
    //! \return A reference to enchant.
    std::optional<Enchant>& GetEnchant();

    //! Returns trigger.
    //! \return A reference to trigger.
    std::optional<Trigger>& GetTrigger();

    //! Adds battlecry task.
    //! \param task A battlecry task to add.
    void AddBattlecryTask(TaskType&& task);

    //! Adds start of combat task.
    //! \param task A start of combat task to add.
    void AddStartCombatTask(TaskType&& task);

    //! Adds deathrattle task.
    //! \param task A deathrattle task to add.
    void AddDeathrattleTask(TaskType&& task);

    //! Adds a Rally task.
    void AddRallyTask(TaskType&& task);

    //! Adds a manual Activate definition. This is intentionally separate from
    //! Battlecry/Rally task storage: Activate is a player decision.
    void AddActivate(ActivateDefinition definition);

    //! Adds enchant.
    //! \param enchant An enchant to add.
    void AddEnchant(Enchant&& enchant);

    //! Adds trigger.
    //! \param trigger An trigger to add.
    void AddTrigger(Trigger&& trigger);

 private:
    std::vector<TaskType> m_battlecryTask;
    std::vector<TaskType> m_startCombatTask;
    std::vector<TaskType> m_deathrattleTask;
    std::vector<TaskType> m_rallyTask;
    std::optional<ActivateDefinition> m_activate;
    std::optional<AvengeDefinition> m_avenge;
    std::optional<Enchant> m_enchant;
    std::optional<Trigger> m_trigger;
};
}  // namespace RosettaStone::Battlegrounds

#endif  // ROSETTASTONE_BATTLEGROUNDS_POWER_HPP
