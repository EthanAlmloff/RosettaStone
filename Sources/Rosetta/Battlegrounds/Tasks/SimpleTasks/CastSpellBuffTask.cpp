#include <Rosetta/Battlegrounds/Models/Player.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CastSpellBuffTask.hpp>
#include <algorithm>
namespace RosettaStone::Battlegrounds::SimpleTasks {
TaskStatus ApplyMinionStatBuffTask::Run(Player& p,Minion& source) {
  switch (m_target) {
    case MinionBuffTarget::FRIENDLY_BOARD_RACE: p.ApplySpellRaceBuff(m_race,m_attack,m_health,false); break;
    case MinionBuffTarget::FRIENDLY_BOARD_AND_HAND_RACE: p.ApplySpellRaceBuff(m_race,m_attack,m_health,true); break;
    case MinionBuffTarget::LEFTMOST_HAND: p.ApplySpellSpecialBuff(1,m_attack,m_health); break;
    case MinionBuffTarget::TAVERN_RACE_PERSISTENT: p.ApplyTavernRaceBuff(m_race,m_attack,m_health); break;
    case MinionBuffTarget::TAVERN_ALL_PERSISTENT:
      // Felemental Portrait adds an independent +2/+2 to every Felemental
      // Battlecry.  The source card remains authoritative for its ordinary
      // (and golden) payload; this aura is deliberately additive.
      if ((source.GetCardID() == "BG25_041" ||
           source.GetCardID() == "BG25_041_G") &&
          p.HasActivePortrait(PortraitEffect::FELEMENTAL_EXTRA_STATS)) {
        p.ApplyTavernRaceBuff(Race::INVALID, m_attack + 2, m_health + 2);
        break;
      }
      // Felblood Portrait makes both Felblood triggers grant Attack and
      // Health. The source check keeps unrelated Tavern-wide stat effects
      // unchanged while covering both Slimy's Battlecry and Fiery's
      // Deathrattle (including golden payloads).
      if ((source.GetCardID() == "BG29_873" ||
           source.GetCardID() == "BG29_873_G" ||
           source.GetCardID() == "BG29_877" ||
           source.GetCardID() == "BG29_877_G") &&
          p.HasActivePortrait(PortraitEffect::FELBLOOD_BOTH_STATS)) {
        // The ordinary Felblood payload is one-sided (Slimy is health,
        // Fiery is attack).  With the portrait, preserve that payload and
        // add the missing stat using the same magnitude, including golden
        // payloads.
        const int amount = std::max(m_attack, m_health);
        p.ApplyTavernRaceBuff(Race::INVALID, amount, amount);
      } else {
        p.ApplyTavernRaceBuff(Race::INVALID,m_attack,m_health);
      }
      break;
  }
  return TaskStatus::COMPLETE;
}
TaskStatus ApplyMinionStatBuffTask::Run(Player& p,Minion& source,Minion&) { return Run(p,source); }
}
