/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <cstdlib>
#include <sstream>
#include "game_actor.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "game_battler.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_party.h"
#include "game_party_base.h"
#include "game_switches.h"
#include "game_system.h"
#include "main_data.h"
#include "output.h"
#include "player.h"
#include "rpg_animation.h"
#include "rpg_state.h"
#include "rpg_skill.h"
#include "rpg_item.h"
#include "sprite_battler.h"

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source) :
	source(source), no_target(true), first_attack(true) {
	Reset();

	current_target = targets.end();
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Battler* target) :
	source(source), no_target(false), first_attack(true) {
	Reset();

	SetTarget(target);
}

Game_BattleAlgorithm::AlgorithmBase::AlgorithmBase(Game_Battler* source, Game_Party_Base* target) :
	source(source), no_target(false), first_attack(true) {
	Reset();

	target->GetActiveBattlers(targets);
	current_target = targets.begin();
}

void Game_BattleAlgorithm::AlgorithmBase::Reset() {
	hp = -1;
	sp = -1;
	attack = -1;
	defense = -1;
	spirit = -1;
	agility = -1;
	switch_id = -1;
	healing = false;
	success = false;
	killed_by_attack_damage = false;
	critical_hit = false;
	absorb = false;
	animation = NULL;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedHp() const {
	return hp;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSp() const {
	return sp;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAttack() const {
	return attack;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedDefense() const {
	return defense;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSpirit() const {
	return spirit;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedAgility() const {
	return agility;
}

int Game_BattleAlgorithm::AlgorithmBase::GetAffectedSwitch() const {
	return switch_id;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsPositive() const {
	return healing;
}

const std::vector<RPG::State>& Game_BattleAlgorithm::AlgorithmBase::GetAffectedConditions() const {
	return conditions;
}

const RPG::Animation* Game_BattleAlgorithm::AlgorithmBase::GetAnimation() const {
	return animation;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsSuccess() const {
	return success;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsKilledByAttack() const {
	return killed_by_attack_damage;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsCriticalHit() const {
	return critical_hit;
}

bool Game_BattleAlgorithm::AlgorithmBase::IsFirstAttack() const {
	return first_attack;
}

std::string Game_BattleAlgorithm::AlgorithmBase::GetDeathMessage() const {
	if (!killed_by_attack_damage) {
		return "";
	}

	if (current_target == targets.end()) {
		return "";
	}

	if ((*current_target)->GetType() == Game_Battler::Type_Ally) {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_actor;
	} else {
		return (*current_target)->GetName() + (*current_target)->GetSignificantState()->message_enemy;
	}
}

void Game_BattleAlgorithm::AlgorithmBase::GetResultMessages(std::vector<std::string>& out) const {
	if (current_target == targets.end()) {
		return;
	}

	if (!success) {
		out.push_back((*current_target)->GetName() + Data::terms.dodge);
	}

	bool target_is_ally = (*current_target)->GetType() == Game_Battler::Type_Ally;

	if (GetAffectedHp() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if (IsPositive()) {
			if (!(*current_target)->IsDead()) {
				ss << " ";
				ss << Data::terms.health_points << " " << GetAffectedHp();
				ss << Data::terms.hp_recovery;
				out.push_back(ss.str());
			}
		}
		else {
			if (critical_hit) {
				out.push_back(target_is_ally ?
					Data::terms.actor_critical :
					Data::terms.enemy_critical);
			}

			if (GetAffectedHp() == 0) {
				ss << (target_is_ally ?
					Data::terms.actor_undamaged :
					Data::terms.enemy_undamaged);
			}
			else {
				if (absorb) {
					ss << " " << Data::terms.health_points << " " << GetAffectedHp();
					ss << (target_is_ally ?
						Data::terms.actor_hp_absorbed :
						Data::terms.enemy_hp_absorbed);
				}
				else {
					ss << " " << GetAffectedHp() << (target_is_ally ?
						Data::terms.actor_damaged :
						Data::terms.enemy_damaged);
				}
			}
			out.push_back(ss.str());
		}
	}

	if (GetAffectedSp() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if (IsPositive()) {
			ss << " ";
			ss << Data::terms.spirit_points << " " << GetAffectedSp();
			ss << Data::terms.hp_recovery;
		}
		else {
			if (absorb) {
				ss << " " << Data::terms.spirit_points << " " << GetAffectedSp();
				ss << (target_is_ally ?
					Data::terms.actor_hp_absorbed :
					Data::terms.enemy_hp_absorbed);
			}
			else {
				ss << " " << Data::terms.attack << " " << GetAffectedSp();
			}
		}
		out.push_back(ss.str());
	}

	if (GetAffectedAttack() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.attack << " " << GetAffectedSp();
		out.push_back(ss.str());
	}

	if (GetAffectedDefense() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.defense << " " << GetAffectedDefense();
		out.push_back(ss.str());
	}

	if (GetAffectedSpirit() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.spirit << " " << GetAffectedSpirit();
		out.push_back(ss.str());
	}

	if (GetAffectedAgility() != -1) {
		std::stringstream ss;
		ss << (*current_target)->GetName();
		ss << " " << Data::terms.agility << " " << GetAffectedAgility();
		out.push_back(ss.str());
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	for (; it != conditions.end(); ++it) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		if ((*current_target)->HasState(it->ID)) {
			if (IsPositive()) {
				ss << it->message_recovery;
				out.push_back(ss.str());
			}
			if (!it->message_already.empty()) {
				ss << it->message_already;
				out.push_back(ss.str());
			}
		} else {
			// Positive case doesn't report anything in case of uselessness
			if (IsPositive()) {
				continue;
			}

			if ((*current_target)->GetType() == Game_Battler::Type_Ally) {
				ss << it->message_actor;
			} else {
				ss << it->message_enemy;
			}
			out.push_back(ss.str());

			// Reporting ends with death state
			if (it->ID == 1) {
				return;
			}
		}
	}
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetSource() const {
	return source;
}

Game_Battler* Game_BattleAlgorithm::AlgorithmBase::GetTarget() const {
	if (current_target == targets.end()) {
		return NULL;
	}

	return *current_target;
}

float Game_BattleAlgorithm::AlgorithmBase::GetAttributeMultiplier(std::vector<bool> attributes_set) const {
	float multiplier = 0;
	int attributes_count = 0;
	 std::vector<uint8_t> targetAttributes = (*current_target)->GetAttributeRanks();
	for (int i = 0; i <attributes_set.size(); i++) {
		if (attributes_set[i]) {
			if (i < targetAttributes.size()) {
				attributes_count++;
				int temp;
				switch (targetAttributes[i]) {
				case 0:
					temp = Data::attributes[i].a_rate;
					break;
				case 1:
					temp = Data::attributes[i].b_rate;
					break;
				case 2:
					temp = Data::attributes[i].c_rate;
					break;
				case 3:
					temp = Data::attributes[i].d_rate;
					break;
				case 4:
					temp = Data::attributes[i].e_rate;
					break;
				default:
					temp = 0;
				}
				multiplier += temp;
			}
		}
	}
	if (attributes_count != 0) {
		multiplier /= (attributes_count * 100);
	}
	return multiplier;
}

void Game_BattleAlgorithm::AlgorithmBase::SetTarget(Game_Battler* target) {
	targets.clear();

	if (target) {
		targets.push_back(target);
		current_target = targets.begin();
	}
	else {
		// Set target is invalid
		current_target = targets.end();
	}
}

void Game_BattleAlgorithm::AlgorithmBase::Apply() {
	if (GetAffectedHp() != -1) {
		int hp = GetAffectedHp();
		int target_hp = (*current_target)->GetHp();
		(*current_target)->ChangeHp(IsPositive() ? hp : -hp);
		if (absorb) {
			// Only absorb the hp that were left
			int src_hp = std::min(target_hp, IsPositive() ? -hp : hp);
			source->ChangeHp(src_hp);
		}
	}

	if (GetAffectedSp() != -1) {
		int sp = GetAffectedSp();
		int target_sp = (*current_target)->GetSp();
		(*current_target)->SetSp((*current_target)->GetSp() + (IsPositive() ? sp : -sp));
		if (absorb) {
			int src_sp = std::min(target_sp, IsPositive() ? -sp : sp);
			source->ChangeSp(src_sp);
		}
	}

	if (GetAffectedAttack() != -1) {
		int atk = GetAffectedAttack();
		(*current_target)->SetAtkModifier(IsPositive() ? atk : -atk);
		if (absorb) {
			source->SetAtkModifier(IsPositive() ? -atk : atk);
		}
	}

	if (GetAffectedDefense() != -1) {
		int def = GetAffectedDefense();
		(*current_target)->SetDefModifier(IsPositive() ? def : -def);
		if (absorb) {
			source->SetDefModifier(IsPositive() ? -def : def);
		}
	}

	if (GetAffectedSpirit() != -1) {
		int spi = GetAffectedSpirit();
		(*current_target)->SetSpiModifier(IsPositive() ? spi : -spi);
		if (absorb) {
			source->SetSpiModifier(IsPositive() ? -spi : spi);
		}
	}

	if (GetAffectedAgility() != -1) {
		int agi = GetAffectedAgility();
		(*current_target)->SetAgiModifier(IsPositive() ? agi : -agi);
		if (absorb) {
			source->SetAgiModifier(IsPositive() ? -agi : agi);
		}
	}

	if (GetAffectedSwitch() != -1) {
		Game_Switches[GetAffectedSwitch()] = true;
	}

	std::vector<RPG::State>::const_iterator it = conditions.begin();

	for (; it != conditions.end(); ++it) {
		if (IsPositive()) {
			if ((*current_target)->IsDead() && it->ID == 1) {
				// Was a revive skill with an effect rating of 0
				(*current_target)->ChangeHp(1);
			}

			(*current_target)->RemoveState(it->ID);
		}
		else {
			(*current_target)->AddState(it->ID);
		}
	}

	source->SetDefending(false);
}

bool Game_BattleAlgorithm::AlgorithmBase::IsTargetValid() {
	if (no_target) {
		// Selected algorithm does not need a target because it targets
		// the source
		return true;
	}
	
	if (current_target == targets.end()) {
		// End of target list reached
		return false;
	}

	return (!(*current_target)->IsDead());
}

int Game_BattleAlgorithm::AlgorithmBase::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Idle;
}

bool Game_BattleAlgorithm::AlgorithmBase::TargetNext() {
	if (current_target == targets.end()) {
		return false;
	}

	if (current_target + 1 != targets.end()) {
		++current_target;
		first_attack = false;
		return true;
	}
	return false;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetStartSe() const {
	return NULL;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetResultSe() const {
	if (healing) {
		return NULL;
	}

	if (!success) {
		return &Game_System::GetSystemSE(Game_System::SFX_Evasion);
	}
	else {
		if (current_target != targets.end()) {
			return ((*current_target)->GetType() == Game_Battler::Type_Ally ?
				&Game_System::GetSystemSE(Game_System::SFX_AllyDamage) :
				&Game_System::GetSystemSE(Game_System::SFX_EnemyDamage));
		}
	}

	return NULL;
}

const RPG::Sound* Game_BattleAlgorithm::AlgorithmBase::GetDeathSe() const {
	return ((*current_target)->GetType() == Game_Battler::Type_Ally ?
		NULL : &Game_System::GetSystemSE(Game_System::SFX_EnemyKill));
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(source, target) {
	// no-op
}

Game_BattleAlgorithm::Normal::Normal(Game_Battler* source, Game_Party_Base* target) :
	AlgorithmBase(source, target) {
	// no-op
}

bool Game_BattleAlgorithm::Normal::Execute() {
	Reset();

	int to_hit;
	float multiplier = 1;
	int crit_chance = source->GetCriticalHitChance();
	if (source->GetType() == Game_Battler::Type_Ally) {
		Game_Actor* ally = static_cast<Game_Actor*>(source);
		int hit_chance = source->GetHitChance();
		int weaponID = ally->GetWeaponId() - 1;
		
		if (weaponID == -1) {
			// No Weapon
			// Todo: Two Sword style
			animation = &Data::animations[Data::actors[ally->GetId() - 1].unarmed_animation - 1];
		} else {
			animation = &Data::animations[Data::items[weaponID].animation_id - 1];
			RPG::Item weapon = Data::items[weaponID];
			hit_chance = weapon.hit;
			crit_chance = crit_chance += weapon.critical_hit;
			multiplier = GetAttributeMultiplier(weapon.attribute_set);
		}
		to_hit = (int)(100 - (100 - hit_chance));
		if(weaponID != -1 && !Data::items[weaponID].ignore_evasion) {
			to_hit *= (1 + (1.0 * (*current_target)->GetAgi() / ally->GetAgi() - 1) / 2);
		}
	} else {
		// Source is Enemy
		int hit = source->GetHitChance();
		to_hit = (int)(100 - (100 - hit) * (1 + (1.0 * (*current_target)->GetAgi() / source->GetAgi() - 1) / 2));
	}

	// Damage calculation
	if (rand() % 100 < to_hit) {
		if (!source->IsCharged() && rand() % 100 < crit_chance) {
			critical_hit = true;
		}

		int effect = (source->GetAtk() / 2 - (*current_target)->GetDef() / 4);

		if (effect < 0)
			effect = 0;

		int act_perc = (rand() % 40) - 20;
		// Change rounded up
		int change = (int)(std::ceil(effect * act_perc / 100.0));
		effect += change;
		effect *= multiplier;
		if(effect < 0) {
			effect = 0;
		}
		this->hp = (effect * (critical_hit ? 3 : 1) * (source->IsCharged() ? 2 : 1)) / ((*current_target)->IsDefending() ? 2 : 1);

		if ((*current_target)->GetHp() - this->hp <= 0) {
			// Death state
			killed_by_attack_damage = true;
			conditions.push_back(Data::states[0]);
		}
		else {
			if (source->GetType() == Game_Battler::Type_Ally) {
				int weaponID = static_cast<Game_Actor*>(source)->GetWeaponId() - 1;
				if (weaponID != -1) {
					RPG::Item item = Data::items[static_cast<Game_Actor*>(source)->GetWeaponId() - 1];
					for (int i = 0; i < item.state_set.size(); i++) {
						if (item.state_set[i] && rand() % 100 < (item.state_chance * (*current_target)->GetStateProbability(Data::states[i].ID) / 100)) {
							if (item.state_effect) {
								healing = true;
							}
							conditions.push_back(Data::states[i]);
						}
					}
				}
			}
		}
	}
	else {
		this->success = false;
		return this->success;
	}

	this->success = true;
	return this->success;
}

void Game_BattleAlgorithm::Normal::Apply() {
	AlgorithmBase::Apply();

	source->SetCharged(false);
	if (source->GetType() == Game_Battler::Type_Ally && static_cast<Game_Actor*>(source)->GetWeaponId() != 0) {
		source->ChangeSp(-Data::items[static_cast<Game_Actor*>(source)->GetWeaponId() -1].sp_cost);
	}
}

std::string Game_BattleAlgorithm::Normal::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.attacking;
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::Normal::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_LeftHand;
}

const RPG::Sound* Game_BattleAlgorithm::Normal::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		return &Game_System::GetSystemSE(Game_System::SFX_EnemyAttacks);
	}
	else {
		return NULL;
	}
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Battler* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source, target), skill(skill), item(item) {
	// no-op
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, Game_Party_Base* target, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source, target), skill(skill), item(item) {
	// no-op
}

Game_BattleAlgorithm::Skill::Skill(Game_Battler* source, const RPG::Skill& skill, const RPG::Item* item) :
	AlgorithmBase(source), skill(skill), item(item) {
	// no-op
}

bool Game_BattleAlgorithm::Skill::IsTargetValid() {
	if (no_target) {
		return true;
	}

	if (current_target == targets.end()) {
		return false;
	}

	if (source->GetType() == Game_Battler::Type_Ally) {
		if (skill.scope == RPG::Skill::Scope_ally ||
			skill.scope == RPG::Skill::Scope_party) {
			if ((*current_target)->IsDead()) {
				// Cures death
				return !skill.state_effects.empty() && skill.state_effects[0];
			}

			return true;
		}
	}

	return (!(*current_target)->IsDead());
}

bool Game_BattleAlgorithm::Skill::Execute() {
	if (item && item->skill_id != skill.ID) {
		assert(false && "Item skill mismatch");
	}

	Reset();

	animation = skill.animation_id == 0 ? NULL : &Data::animations[skill.animation_id - 1];

	this->success = false;

	this->healing =
		skill.scope == RPG::Skill::Scope_ally ||
		skill.scope == RPG::Skill::Scope_party ||
		skill.scope == RPG::Skill::Scope_self;

	if (skill.type == RPG::Skill::Type_normal ||
		skill.type >= RPG::Skill::Type_subskill) {
		if (this->healing) {
			this->success = true;

			if (skill.affect_hp)
				this->hp = skill.power;
			if (skill.affect_sp)
				this->sp = skill.power;
			if (skill.affect_attack)
				this->attack = skill.power;
			if (skill.affect_defense)
				this->defense = skill.power;
			if (skill.affect_spirit)
				this->spirit = skill.power;
			if (skill.affect_agility)
				this->agility = skill.power;

		}
		else if (rand() % 100 < skill.hit) {
			this->success = true;

			int effect = skill.power +
				source->GetAtk() * skill.physical_rate / 20 +
				source->GetSpi() * skill.magical_rate / 40;
			if (!skill.ignore_defense) {
				effect -= (*current_target)->GetDef() * skill.physical_rate / 40 -
					(*current_target)->GetSpi() * skill.magical_rate / 80;
			}
			effect *= GetAttributeMultiplier(skill.attribute_effects);

			if(effect < 0) {
				effect = 0;
			}

			effect += rand() % (((effect * skill.variance / 10) + 1) - (effect * skill.variance / 20));

			if (effect < 0)
				effect = 0;

			if (skill.affect_hp) {
				this->hp = effect / ((*current_target)->IsDefending() ? 2 : 1);

				if ((*current_target)->GetHp() - this->hp <= 0) {
					// Death state
					killed_by_attack_damage = true;
					conditions.push_back(Data::states[0]);
				}
			}

			if (skill.affect_sp) {
				this->sp = std::min<int>(effect, (*current_target)->GetSp());
			}
				
			if (skill.affect_attack)
				this->attack = effect;
			if (skill.affect_defense)
				this->defense = effect;
			if (skill.affect_spirit)
				this->spirit = effect;
			if (skill.affect_agility)
				this->agility = agility;
		}

		for (int i = 0; i < (int) skill.state_effects.size(); i++) {
			if (!skill.state_effects[i])
				continue;
			if (!healing && rand() % 100 >= skill.hit)
				continue;

			this->success = true;
			
			if (healing || rand() % 100 <= (*current_target)->GetStateProbability(Data::states[i].ID)) {
				conditions.push_back(Data::states[i]);
			}
		}
	}
	else if (skill.type == RPG::Skill::Type_switch) {
		switch_id = skill.switch_id;
		this->success = true;
	}
	else {
		assert(false && "Unsupported skill type");
	}

	absorb = skill.absorb_damage;
	if (absorb && sp != -1) {
		if ((*current_target)->GetSp() == 0) {
			this->success = false;
		}
	}


	return this->success;
}

void Game_BattleAlgorithm::Skill::Apply() {
	AlgorithmBase::Apply();

	if (item) {
		Main_Data::game_party->ConsumeItemUse(item->ID);
	}
	else {
		if (first_attack) {
			source->ChangeSp(- source->CalculateSkillCost(skill.ID));
		}
	}
}

std::string Game_BattleAlgorithm::Skill::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		if (item && item->using_message == 0) {
			// Use item message
			return Item(source, *item).GetStartMessage();
		}
		return source->GetName() + skill.using_message1 + '\n' + skill.using_message2;
	}
	else {
		return source->GetName() + ": " + skill.name;
	}
}

int Game_BattleAlgorithm::Skill::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_SkillUse;
}

const RPG::Sound* Game_BattleAlgorithm::Skill::GetStartSe() const {
	if (skill.type == RPG::Skill::Type_switch) {
		return &skill.sound_effect;
	}
	else {
		if (source->GetType() == Game_Battler::Type_Enemy) {
			return &Game_System::GetSystemSE(Game_System::SFX_EnemyAttacks);
		}
		else {
			return NULL;
		}
	}
}

void Game_BattleAlgorithm::Skill::GetResultMessages(std::vector<std::string>& out) const {
	if (!success) {
		std::stringstream ss;
		ss << (*current_target)->GetName();

		switch (skill.failure_message) {
			case 0:
				ss << Data::terms.skill_failure_a;
				break;
			case 1:
				ss << Data::terms.skill_failure_b;
				break;
			case 2:
				ss << Data::terms.skill_failure_c;
				break;
			case 3:
				ss << Data::terms.dodge;
				break;
			default:
				ss << " BUG: INVALID SKILL FAIL MSG";
		}
		out.push_back(ss.str());
		return;
	}

	AlgorithmBase::GetResultMessages(out);
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Battler* target, const RPG::Item& item) :
	AlgorithmBase(source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, Game_Party_Base* target, const RPG::Item& item) :
	AlgorithmBase(source, target), item(item) {
		// no-op
}

Game_BattleAlgorithm::Item::Item(Game_Battler* source, const RPG::Item& item) :
AlgorithmBase(source), item(item) {
	// no-op
}

bool Game_BattleAlgorithm::Item::IsTargetValid() {
	if (no_target) {
		return true;
	}

	if (current_target == targets.end()) {
		return false;
	}

	if ((*current_target)->IsDead()) {
		// Medicine curing death
		return item.type == RPG::Item::Type_medicine &&
			!item.state_set.empty() &&
			item.state_set[0];
	}

	return item.type == RPG::Item::Type_medicine;
}

bool Game_BattleAlgorithm::Item::Execute() {
	Reset();

	// All other items are handled as skills because they invoke skills
	switch (item.type) {
		case RPG::Item::Type_medicine:
		case RPG::Item::Type_switch:
			break;
		default:
			assert("Unsupported battle item type");
	}

	this->success = false;

	if (item.type == RPG::Item::Type_medicine) {
		this->healing = true;

		// HP recovery
		if (item.recover_hp != 0 || item.recover_hp_rate != 0) {
			this->hp = item.recover_hp_rate * (*current_target)->GetMaxHp() / 100 + item.recover_hp;
		}

		// SP recovery
		if (item.recover_sp != 0 || item.recover_sp_rate != 0) {
			this->sp = item.recover_sp_rate * (*current_target)->GetMaxSp() / 100 + item.recover_sp;
		}

		for (int i = 0; i < (int)item.state_set.size(); i++) {
			if (item.state_set[i]) {
				this->conditions.push_back(Data::states[i]);
			}
		}

		this->success = true;
	}
	else if (item.type == RPG::Item::Type_switch) {
		switch_id = item.switch_id;
		this->success = true;
	}

	return this->success;
}

void Game_BattleAlgorithm::Item::Apply() {
	AlgorithmBase::Apply();

	if (first_attack) {
		Main_Data::game_party->ConsumeItemUse(item.ID);
	}
}

std::string Game_BattleAlgorithm::Item::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + " " + item.name + Data::terms.use_item;
	}
	else {
		return source->GetName() + ": " + item.name;
	}
}

int Game_BattleAlgorithm::Item::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Item;
}

void Game_BattleAlgorithm::Item::GetResultMessages(std::vector<std::string>& out) const {
	AlgorithmBase::GetResultMessages(out);
}

const RPG::Sound* Game_BattleAlgorithm::Item::GetStartSe() const {
	if (item.type == RPG::Item::Type_switch) {
		return &Game_System::GetSystemSE(Game_System::SFX_UseItem);
	}
	else {
		if (source->GetType() == Game_Battler::Type_Enemy) {
			return &Game_System::GetSystemSE(Game_System::SFX_EnemyAttacks);
		}
		else {
			return NULL;
		}
	}
}

Game_BattleAlgorithm::NormalDual::NormalDual(Game_Battler* source, Game_Battler* target) :
	AlgorithmBase(source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::NormalDual::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + " TODO DUAL";
	}
	else {
		return "";
	}
}

const RPG::Sound* Game_BattleAlgorithm::NormalDual::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Enemy) {
		return &Game_System::GetSystemSE(Game_System::SFX_EnemyAttacks);
	}
	else {
		return NULL;
	}
}

bool Game_BattleAlgorithm::NormalDual::Execute() {
	Output::Warning("Battle: Enemy Double Attack not implemented");
	return true;
}

Game_BattleAlgorithm::Defend::Defend(Game_Battler* source) :
	AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Defend::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.defending;
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::Defend::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Defending;
}

bool Game_BattleAlgorithm::Defend::Execute() {
	return true;
}

void Game_BattleAlgorithm::Defend::Apply() {
	source->SetDefending(true);
}

Game_BattleAlgorithm::Observe::Observe(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Observe::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.observing;
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Observe::Execute() {
	// Observe only prints the start message
	return true;
}

Game_BattleAlgorithm::Charge::Charge(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Charge::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.focus;
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Charge::Execute() {
	return true;
}

void Game_BattleAlgorithm::Charge::Apply() {
	source->SetCharged(true);
}

Game_BattleAlgorithm::SelfDestruct::SelfDestruct(Game_Battler* source, Game_Party_Base* target) :
AlgorithmBase(source, target) {
	// no-op
}

std::string Game_BattleAlgorithm::SelfDestruct::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.autodestruction;
	}
	else {
		return "";
	}
}

int Game_BattleAlgorithm::SelfDestruct::GetSourceAnimationState() const {
	return Sprite_Battler::AnimationState_Dead;
}

const RPG::Sound* Game_BattleAlgorithm::SelfDestruct::GetStartSe() const {
	return &Game_System::GetSystemSE(Game_System::SFX_EnemyKill);
}

bool Game_BattleAlgorithm::SelfDestruct::Execute() {
	Reset();

	// Like a normal attack, but with double damage and always hitting
	// Never crits, ignores charge
	int effect = source->GetAtk() - (*current_target)->GetDef() / 2;

	if (effect < 0)
		effect = 0;

	// up to 20% stronger/weaker
	int act_perc = (rand() % 40) - 20;
	int change = (int)(std::ceil(effect * act_perc / 100.0));
	effect += change;

	if (effect < 0)
		effect = 0;
	
	this->hp = effect / ((*current_target)->IsDefending() ? 2 : 1);;

	if ((*current_target)->GetHp() - this->hp <= 0) {
		// Death state
		killed_by_attack_damage = true;
		conditions.push_back(Data::states[0]);
	}

	success = true;

	return true;
}

void Game_BattleAlgorithm::SelfDestruct::Apply() {
	AlgorithmBase::Apply();

	// Only monster can self destruct
	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
}

Game_BattleAlgorithm::Escape::Escape(Game_Battler* source) :
	AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::Escape::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		// Only monsters can escape during a battle phase

		if (source->GetType() == Game_Battler::Type_Enemy) {
			return source->GetName() + Data::terms.enemy_escape;
		}
	}

	return "";
}

int Game_BattleAlgorithm::Escape::GetSourceAnimationState() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetSourceAnimationState();
	}
	else {
		return Sprite_Battler::AnimationState_Dead;
	}
}

const RPG::Sound* Game_BattleAlgorithm::Escape::GetStartSe() const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		return AlgorithmBase::GetStartSe();
	}
	else {
		return &Game_System::GetSystemSE(Game_System::SFX_Escape);
	}
}

bool Game_BattleAlgorithm::Escape::Execute() {
	Reset();

	// Monsters always escape
	this->success = true;

	// TODO: Preemptive attack has 100% escape ratio

	if (source->GetType() == Game_Battler::Type_Ally) {
		int ally_agi = Main_Data::game_party->GetAverageAgility();
		int enemy_agi = Main_Data::game_enemyparty->GetAverageAgility();

		double to_hit = 1.5 * ((float)ally_agi / enemy_agi);

		// Every failed escape is worth 10% higher escape chance (see help file)
		for (int i = 0; i < Game_Battle::escape_fail_count; ++i) {
			to_hit += (to_hit * 0.1);
		}

		to_hit *= 100;

		this->success = rand() % 100 < (int)to_hit;
	}

	return this->success;
}

void Game_BattleAlgorithm::Escape::Apply() {
	if (!this->success) {
		Game_Battle::escape_fail_count += 1;
	}

	if (source->GetType() == Game_Battler::Type_Enemy) {
		static_cast<Game_Enemy*>(source)->SetHidden(true);
	}
}

void Game_BattleAlgorithm::Escape::GetResultMessages(std::vector<std::string>& out) const {
	if (source->GetType() == Game_Battler::Type_Ally) {
		if (this->success) {
			out.push_back(Data::terms.escape_success);
		}
		else {
			out.push_back(Data::terms.escape_failure);
		}
	}
}

Game_BattleAlgorithm::Transform::Transform(Game_Battler* source, int new_monster_id) :
AlgorithmBase(source), new_monster_id(new_monster_id) {
	// no-op
}

std::string Game_BattleAlgorithm::Transform::GetStartMessage() const {
	if (Player::IsRPG2k()) {
		return source->GetName() + Data::terms.enemy_transform;
	}
	else {
		return "";
	}
}

bool Game_BattleAlgorithm::Transform::Execute() {
	return true;
}

void Game_BattleAlgorithm::Transform::Apply() {
	static_cast<Game_Enemy*>(source)->Transform(new_monster_id);
}

Game_BattleAlgorithm::NoMove::NoMove(Game_Battler* source) :
AlgorithmBase(source) {
	// no-op
}

std::string Game_BattleAlgorithm::NoMove::GetStartMessage() const {
	const std::vector<int16_t>& states = source->GetStates();

	for (std::vector<int16_t>::const_iterator it = states.begin();
		it != states.end(); ++it) {
		if (Data::states[*it].restriction == RPG::State::Restriction_do_nothing) {
			std::string msg = Data::states[*it].message_affected;
			if (!msg.empty()) {
				return source->GetName() + msg;
			}
			return "";
		}
	}

	// State was healed before the actor got his turn
	return "";
}

bool Game_BattleAlgorithm::NoMove::Execute() {
	// no-op
	return true;
}

void Game_BattleAlgorithm::NoMove::Apply() {
	// no-op
}


