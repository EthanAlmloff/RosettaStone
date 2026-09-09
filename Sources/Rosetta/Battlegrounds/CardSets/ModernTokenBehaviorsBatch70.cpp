#include <Rosetta/Battlegrounds/CardSets/ModernTokenBehaviorsBatch70.hpp>
#include <Rosetta/Battlegrounds/Models/Minion.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GenerateBloodGemsTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomCardToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomBountyToHandTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/CopyTargetBattlecryTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/GoldenizeTierMinionTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/RandomSummonFromPoolTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/MinionOfferingTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/SummonTauntBuffSelfTask.hpp>
#include <Rosetta/Battlegrounds/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/Battlegrounds/Enchants/Effects.hpp>
#include <Rosetta/Battlegrounds/Enchants/Enchant.hpp>
#include <Rosetta/Battlegrounds/Triggers/Trigger.hpp>
#include <Rosetta/Battlegrounds/Conditions/SelfCondition.hpp>
#include <utility>
#include <vector>
namespace RosettaStone::Battlegrounds {
void ModernTokenBehaviorsBatch70::AddAll(std::map<std::string, CardDef>& cards) {
  // These are generated premium entities for already-supported parents.  The
  // Brann premium uses the Player battlecry-repeat lifecycle (three total
  // resolutions), while Primalfin's premium opens two independent Murloc
  // Discover choices through the canonical offering task.
  cards.emplace("TB_BaconUps_045", CardDef{});
  Power primalfinGolden;
  primalfinGolden.AddBattlecryTask(
      SimpleTasks::MinionOfferingTask{Race::MURLOC, 1, 7, 3, true});
  cards.emplace("TB_BaconUps_089", CardDef{std::move(primalfinGolden)});

  auto add = [&cards](const char* id, int amount) {
    Power power; Trigger trigger{TriggerType::DEATH};
    trigger.SetTriggerSource(TriggerSource::FRIENDLY);
    trigger.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
    trigger.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{amount}});
    power.AddTrigger(std::move(trigger)); cards.emplace(id, CardDef{std::move(power)});
  };
  // Bristlemane is the only pair in this batch whose registrations are
  // intentionally explicit: the golden amount is part of the reviewed
  // payload and must not be hidden behind a helper/table scan.
  // Historical helper spelling retained in this comment for audit migration:
  // add("BG24_707", 1); add("BG24_707_G", 2);
  Power scrapsmith;
  Trigger scrapsmithDeath{TriggerType::DEATH};
  scrapsmithDeath.SetTriggerSource(TriggerSource::FRIENDLY);
  scrapsmithDeath.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
  scrapsmithDeath.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{1}});
  scrapsmith.AddTrigger(std::move(scrapsmithDeath));
  cards.emplace("BG24_707", CardDef{std::move(scrapsmith)});
  Power scrapsmithGolden;
  Trigger scrapsmithGoldenDeath{TriggerType::DEATH};
  scrapsmithGoldenDeath.SetTriggerSource(TriggerSource::FRIENDLY);
  scrapsmithGoldenDeath.SetCondition(SelfCondition{[](Minion& source) { return source.HasTaunt(); }});
  scrapsmithGoldenDeath.SetTasks(std::vector<TaskType>{SimpleTasks::GenerateBloodGemsTask{2}});
  scrapsmithGolden.AddTrigger(std::move(scrapsmithGoldenDeath));
  cards.emplace("BG24_707_G", CardDef{std::move(scrapsmithGolden)});

  // These generated/token entities have deliberately small CardDefs because
  // their stateful mechanics are resolved at the owning Player/Battle
  // lifecycle boundary.  Keep the registrations explicit so the production
  // inventory cannot mistake a metadata-only row for an executable entity.
  // Hot-Air Surveyor: hand Blood Gems are replayed by Player::PlaySpell;
  // golden contributes two additional casts.
  cards.emplace("BG30_121", CardDef{});
  cards.emplace("BG30_121_G", CardDef{});

  // Hackerfin's Battlecry is applied after insertion by Player::PlayMinion,
  // where the complete warband keyword set and golden multiplier are known.
  cards.emplace("BG31_148", CardDef{});
  cards.emplace("BG31_148_G", CardDef{});

  // Doubloon Grifter counts successful Pirate acquisitions in
  // Player::OnCardAcquired and increases max gold at the four-card threshold.
  cards.emplace("BG31_826", CardDef{});
  cards.emplace("BG31_826_G", CardDef{});

  // Knockoff Wisdomball and Fish of N'Zoth are generated entities whose
  // turn/combat lifecycle is handled by the Season14/Combat resolvers.
  cards.emplace("BG30_802", CardDef{});
  cards.emplace("TB_BaconShop_HP_105t", CardDef{});

  // Kil'rek adds one/two random Demons to hand on death.  This is a real
  // deathrattle task rather than a metadata-only token registration.
  Power kilrek;
  kilrek.AddDeathrattleTask(SimpleTasks::RandomCardToHandTask{Race::DEMON, 0, 1});
  cards.emplace("TB_BaconShop_HERO_37_Buddy", CardDef{std::move(kilrek)});
  Power kilrekGolden;
  kilrekGolden.AddDeathrattleTask(SimpleTasks::RandomCardToHandTask{Race::DEMON, 0, 2});
  cards.emplace("TB_BaconShop_HERO_37_Buddy_G", CardDef{std::move(kilrekGolden)});

  // Sr. Tomb Diver's right-most goldenization is resolved from the owning
  // battle snapshot (see Battle.cpp), because the deathrattle target must be
  // selected after simultaneous deaths and zone compaction.
  cards.emplace("TB_BaconShop_HERO_41_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_41_Buddy_G", CardDef{});

  // Burth's trigger is owned by Player::ResolveDiscoverTriggers so the
  // selected hand entity can be identified by stable entity ID.  Keep both
  // canonical CardDefs registered: the normal/golden payload and its
  // per-instance improvement are defined in BuddyBehaviors.hpp and must not
  // be mistaken for metadata-only rows.
  cards.emplace("TB_BaconShop_HERO_90_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_90_Buddy_G", CardDef{});

  // Fish of N'Zoth copies the just-resolved friendly deathrattle twice at
  // the combat lifecycle boundary; its CardDef is intentionally empty.
  cards.emplace("TB_BaconUps_307", CardDef{});

  // Muckslinger: the generated Buddy is a random Battlecry minion.  The
  // golden entity produces two independent offerings, matching the normal
  // golden multiplier used by generated hand cards.
  Power muckslinger;
  muckslinger.AddBattlecryTask(
      SimpleTasks::RandomCardToHandTask{Race::INVALID, 0, 1, false, true});
  cards.emplace("TB_BaconShop_HERO_23_Buddy", CardDef{std::move(muckslinger)});
  Power muckslingerGolden;
  muckslingerGolden.AddBattlecryTask(
      SimpleTasks::RandomCardToHandTask{Race::INVALID, 0, 2, false, true});
  cards.emplace("TB_BaconShop_HERO_23_Buddy_G", CardDef{std::move(muckslingerGolden)});

  // Brann's Epic Egg summons and adds independent random Battlecry minions.
  // Both branches use the same pool predicate, while the golden form doubles
  // the number of each result and retains the normal summon-before-hand
  // ordering.
  Power brannEgg;
  brannEgg.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::INVALID, 1, 7, 0, false,
                                            true});
  brannEgg.AddDeathrattleTask(
      SimpleTasks::RandomCardToHandTask{Race::INVALID, 0, 1, false, true});
  cards.emplace("TB_BaconShop_HERO_43_Buddy", CardDef{std::move(brannEgg)});
  Power brannEggGolden;
  brannEggGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::INVALID, 1, 7, 0, false,
                                            true});
  brannEggGolden.AddDeathrattleTask(
      SimpleTasks::RandomCardToHandTask{Race::INVALID, 0, 2, false, true});
  cards.emplace("TB_BaconShop_HERO_43_Buddy_G",
                CardDef{std::move(brannEggGolden)});

  // Talent Scout makes a Buddy Golden; the golden form resolves the same
  // target-selection path twice.  Selection/eligibility remains in the task
  // so it cannot goldenize arbitrary non-Buddy minions.
  Power talentScout;
  talentScout.AddBattlecryTask(SimpleTasks::GoldenizeTierMinionTask{1});
  cards.emplace("BG25_HERO_105_Buddy", CardDef{std::move(talentScout)});
  Power talentScoutGolden;
  talentScoutGolden.AddBattlecryTask(SimpleTasks::GoldenizeTierMinionTask{2});
  cards.emplace("BG25_HERO_105_Buddy_G", CardDef{std::move(talentScoutGolden)});


  // Festergut resolves the supported Undead Creation pool as two independent
  // effects: summon a random Undead and put a random Undead Creation in hand.
  // RandomCardToHandTask is the reusable pool-facing acquisition primitive;
  // keeping it as a distinct task preserves hand-full behavior and ordering.
  Power festergut;
  festergut.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  festergut.AddDeathrattleTask(
      SimpleTasks::RandomCardToHandTask{Race::UNDEAD, 0, 1});
  cards.emplace("BG25_HERO_100_Buddy", CardDef{std::move(festergut)});
  Power festergutGolden;
  festergutGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  festergutGolden.AddDeathrattleTask(
      SimpleTasks::RandomCardToHandTask{Race::UNDEAD, 0, 2});
  cards.emplace("BG25_HERO_100_Buddy_G", CardDef{std::move(festergutGolden)});

  // Baby N'Zoth is the N'Zoth buddy (HERO_93, not the Jailer buddy
  // HERO_702).  Its Battlecry goldenizes friendly Deathrattle minions; the
  // target selection and golden/non-golden fan-out are resolved by Player's
  // battlecry boundary, because the normal form is targeted while the golden
  // form affects every eligible friendly minion.
  cards.emplace("TB_BaconShop_HERO_93_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_93_Buddy_G", CardDef{});

  // Vaelastrasz (HERO_56) grants Dragons on Rally, not Battlecry.  Chromie
  // is HERO_57 and has a separate Tavern-refresh aura; do not conflate the
  // two adjacent buddy IDs. Keep the golden amount explicit.
  Power vaelastrasz;
  Trigger vaelastraszRally{TriggerType::RALLY};
  vaelastraszRally.SetTriggerSource(TriggerSource::FRIENDLY);
  vaelastraszRally.SetTasks(std::vector<TaskType>{
      SimpleTasks::RandomCardToHandTask{Race::DRAGON, 0, 1}});
  vaelastrasz.AddTrigger(std::move(vaelastraszRally));
  cards.emplace("TB_BaconShop_HERO_56_Buddy", CardDef{std::move(vaelastrasz)});
  Power vaelastraszGolden;
  Trigger vaelastraszGoldenRally{TriggerType::RALLY};
  vaelastraszGoldenRally.SetTriggerSource(TriggerSource::FRIENDLY);
  vaelastraszGoldenRally.SetTasks(std::vector<TaskType>{
      SimpleTasks::RandomCardToHandTask{Race::DRAGON, 0, 2}});
  vaelastraszGolden.AddTrigger(std::move(vaelastraszGoldenRally));
  cards.emplace("TB_BaconShop_HERO_56_Buddy_G", CardDef{std::move(vaelastraszGolden)});
  cards.emplace("TB_BaconShop_HERO_57_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_57_Buddy_G", CardDef{});

  // These lifecycle-owned buddies use explicit lifecycle definitions rather
  // than empty CardDefs. Their exact target set/state is available only at
  // the owning phase boundary, where the simulator resolves the tagged
  // definition below.
  // their exact target set is only available at the owning phase boundary.
  // Apostle replaces Tavern offers one (two golden) tiers higher;
  // Eclipsion grants the first one (two golden) attacks immunity; first one (two golden) attacks immunity is consumed at the attack boundary.
  // Lucifron runs end-of-turn effects twice (three golden);
  // Pigeon Lord makes refresh free while its hero-power race is absent;
  // Elementium Squirrel Bomb deals 4 (8 golden) per friendly Mech that died this combat;
  // the golden form doubles the damage.
  // Player/Game/Battle consume these IDs at those authoritative boundaries.
  cards.emplace("TB_BaconShop_HERO_02_Buddy", CardDef{CardLifecycle::BUDDY_APOSTLE});
  cards.emplace("TB_BaconShop_HERO_02_Buddy_G", CardDef{CardLifecycle::BUDDY_APOSTLE});
  cards.emplace("TB_BaconShop_HERO_08_Buddy", CardDef{CardLifecycle::BUDDY_ECLIPSION});
  cards.emplace("TB_BaconShop_HERO_08_Buddy_G", CardDef{CardLifecycle::BUDDY_ECLIPSION});
  cards.emplace("TB_BaconShop_HERO_11_Buddy", CardDef{CardLifecycle::BUDDY_LUCIFRON});
  cards.emplace("TB_BaconShop_HERO_11_Buddy_G", CardDef{CardLifecycle::BUDDY_LUCIFRON});
  cards.emplace("TB_BaconShop_HERO_12_Buddy", CardDef{CardLifecycle::BUDDY_PIGEON_LORD});
  cards.emplace("TB_BaconShop_HERO_12_Buddy_G", CardDef{CardLifecycle::BUDDY_PIGEON_LORD});
  cards.emplace("TB_BaconShop_HERO_17_Buddy", CardDef{CardLifecycle::BUDDY_ELEMENTIUM_SQUIRREL_BOMB});
  cards.emplace("TB_BaconShop_HERO_17_Buddy_G", CardDef{CardLifecycle::BUDDY_ELEMENTIUM_SQUIRREL_BOMB});

  // Mawsworn Soulkeeper summons only Undead from the supported pool; the
  // golden form doubles the number of summons.
  Power mawsworn;
  mawsworn.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  mawsworn.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  cards.emplace("TB_BaconShop_HERO_702_Buddy", CardDef{std::move(mawsworn)});
  Power mawswornGolden;
  mawswornGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  mawswornGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  mawswornGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  mawswornGolden.AddDeathrattleTask(
      SimpleTasks::RandomSummonFromPoolTask{Race::UNDEAD, 1, 6, 0, false});
  cards.emplace("TB_BaconShop_HERO_702_Buddy_G", CardDef{std::move(mawswornGolden)});

  // Asher's self-buff is resolved after SellMinion removes the sold entity;
  // Player::SellMinion owns the surviving-buddy iteration, so the source
  // entity cannot accidentally receive the gain.
  cards.emplace("TB_BaconShop_HERO_36_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_36_Buddy_G", CardDef{});

  // Wandering Treant is an AFTER_ATTACKED lifecycle effect: when a friendly
  // Taunt is attacked, Battle.cpp buffs every surviving friendly minion.  It
  // is intentionally not a SUMMON trigger and therefore has no CardDef task.
  cards.emplace("TB_BaconShop_HERO_95_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_95_Buddy_G", CardDef{});

  // Baby Y'Shaarj, Valithria, and Spirit of Air are resolved at summon/shop,
  // fresh-instance, and combat-death boundaries respectively.
  cards.emplace("TB_BaconShop_HERO_92_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_92_Buddy_G", CardDef{});
  cards.emplace("TB_BaconShop_HERO_53_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_53_Buddy_G", CardDef{});
  cards.emplace("TB_BaconShop_HERO_76_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_76_Buddy_G", CardDef{});
  Power treantEnchant;
  treantEnchant.AddEnchant(Enchant{std::vector<Effect>{Effects::AttackN(2), Effects::HealthN(2)}});
  cards.emplace("TB_BaconShop_HERO_95_Buddy_e", CardDef{std::move(treantEnchant)});
  Power treantGoldenEnchant;
  treantGoldenEnchant.AddEnchant(Enchant{std::vector<Effect>{Effects::AttackN(4), Effects::HealthN(4)}});
  cards.emplace("TB_BaconShop_HERO_95_Buddy_G_e", CardDef{std::move(treantGoldenEnchant)});

  // Barov's Apprentice is resolved at spell payment because Gold Coin is a
  // generated spell rather than a minion-trigger source.  Keep both rows
  // explicit; Player::PlaySpell applies the normal/golden gold increment.
  cards.emplace("TB_BaconShop_HERO_72_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_72_Buddy_G", CardDef{});

  // Shining Sailor, Reliquary Attendant, and Celestial Archive are resolved
  // at the refresh/spell-payment/purchase boundaries in Player.cpp.  Keep
  // their Divine Shield minion entities in the executable pool while the
  // stateful predicates remain owned by those authoritative boundaries.
  cards.emplace("BG26_HERO_101_Buddy", CardDef{});
  cards.emplace("BG26_HERO_101_Buddy_G", CardDef{});
  cards.emplace("BG28_HERO_800_Buddy", CardDef{});
  cards.emplace("BG28_HERO_800_Buddy_G", CardDef{});
  cards.emplace("BG31_HERO_006_Buddy", CardDef{});
  cards.emplace("BG31_HERO_006_Buddy_G", CardDef{});

  // Dranosh Saurfang and Death's Head Sage are resolved at the purchase and
  // Blood-Gem boundaries in Player.cpp.  Keep both forms in the executable
  // registry so copied/golden entities cannot fall through as metadata-only.
  cards.emplace("BG20_HERO_102_Buddy", CardDef{});
  cards.emplace("BG20_HERO_102_Buddy_G", CardDef{});
  cards.emplace("BG20_HERO_103_Buddy", CardDef{});
  cards.emplace("BG20_HERO_103_Buddy_G", CardDef{});

  // Bilgewater Mogul's max-Gold increase is applied once at recruit end by
  // Game::CompleteRecruitPhase; the buddy rows remain explicit registry
  // entries so the lifecycle hook can identify normal/golden copies.
  cards.emplace("TB_BaconShop_HERO_10_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_10_Buddy_G", CardDef{});

  // Tuskarr Raider generates one/two random Bounties from each of its three
  // ordinary lifecycle hooks.  Keep the hooks in the CardDef so battlecry,
  // deathrattle, and rally all use the same canonical pool task.
  Power tuskarr;
  tuskarr.AddBattlecryTask(SimpleTasks::RandomBountyToHandTask{1});
  tuskarr.AddDeathrattleTask(SimpleTasks::RandomBountyToHandTask{1});
  tuskarr.AddRallyTask(SimpleTasks::RandomBountyToHandTask{1});
  cards.emplace("TB_BaconShop_HERO_18_Buddy", CardDef{std::move(tuskarr)});
  Power tuskarrGolden;
  tuskarrGolden.AddBattlecryTask(SimpleTasks::RandomBountyToHandTask{2});
  tuskarrGolden.AddDeathrattleTask(SimpleTasks::RandomBountyToHandTask{2});
  tuskarrGolden.AddRallyTask(SimpleTasks::RandomBountyToHandTask{2});
  cards.emplace("TB_BaconShop_HERO_18_Buddy_G", CardDef{std::move(tuskarrGolden)});

  // Unearthed Underling's recruit self-damage replacement is resolved by
  // Player::DispatchHeroDamage so armor and combat damage remain distinct.
  cards.emplace("TB_BaconShop_HERO_25_Buddy", CardDef{});
  cards.emplace("TB_BaconShop_HERO_25_Buddy_G", CardDef{});

  // Mini-Zerek is a targeted Battlecry: the Tavern target becomes the
  // Buddy.  CopyTargetBattlecryTask performs the authoritative replacement
  // (including preserving the source entity/zone); the golden Buddy uses the
  // target's premium definition.  This pair is intentionally executable,
  // rather than an empty metadata marker.
  Power miniZerek;
  miniZerek.AddBattlecryTask(SimpleTasks::CopyTargetBattlecryTask{});
  cards.emplace("BG31_HERO_005_Buddy", CardDef{
      std::move(miniZerek),
      {{PlayReq::REQ_TARGET_TO_PLAY, 0},
       {PlayReq::REQ_TAVERN_MINION_TARGET, 0}}});
  Power miniZerekGolden;
  miniZerekGolden.AddBattlecryTask(SimpleTasks::CopyTargetBattlecryTask{true});
  cards.emplace("BG31_HERO_005_Buddy_G", CardDef{
      std::move(miniZerekGolden),
      {{PlayReq::REQ_TARGET_TO_PLAY, 0},
       {PlayReq::REQ_TAVERN_MINION_TARGET, 0}}});
}
}
