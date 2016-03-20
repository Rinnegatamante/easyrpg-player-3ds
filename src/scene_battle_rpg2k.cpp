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


#include <algorithm>
#include <sstream>
#include "input.h"
#include "player.h"
#include "sprite.h"
#include "game_battler.h"
#include "game_system.h"
#include "game_temp.h"
#include "game_party.h"
#include "game_enemy.h"
#include "game_enemyparty.h"
#include "game_message.h"
#include "game_battle.h"
#include "game_battlealgorithm.h"
#include "battle_animation.h"
#include "scene_battle_rpg2k.h"
#include "scene_battle.h"
#include "scene_gameover.h"
#include "output.h"

Scene_Battle_Rpg2k::Scene_Battle_Rpg2k() : Scene_Battle(),
battle_action_wait(0),
battle_action_state(BattleActionState_ConditionHeal)
{
}

Scene_Battle_Rpg2k::~Scene_Battle_Rpg2k() {
}

void Scene_Battle_Rpg2k::Update() {
	battle_message_window->Update();

	Scene_Battle::Update();
}

void Scene_Battle_Rpg2k::CreateUi() {
	Scene_Battle::CreateUi();

	CreateBattleTargetWindow();
	CreateBattleCommandWindow();

	battle_message_window.reset(new Window_BattleMessage(0, (SCREEN_TARGET_HEIGHT - 80), SCREEN_TARGET_WIDTH, 80));

	if (!Game_Battle::IsEscapeAllowed()) {
		options_window->DisableItem(2);
	}
}

void Scene_Battle_Rpg2k::CreateBattleTargetWindow() {
	std::vector<std::string> commands;
	std::vector<Game_Battler*> enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(enemies);

	for (std::vector<Game_Battler*>::iterator it = enemies.begin();
		it != enemies.end(); ++it) {
		commands.push_back((*it)->GetName());
	}

	target_window.reset(new Window_Command(commands, 136, 4));
	target_window->SetHeight(80);
	target_window->SetY(SCREEN_TARGET_HEIGHT-80);
	target_window->SetZ(3001);
}

void Scene_Battle_Rpg2k::CreateBattleCommandWindow() {
	std::vector<std::string> commands;
	commands.push_back(Data::terms.command_attack);
	commands.push_back(Data::terms.command_skill);
	commands.push_back(Data::terms.command_defend);
	commands.push_back(Data::terms.command_item);

	command_window.reset(new Window_Command(commands, 76));
	command_window->SetHeight(80);
	command_window->SetX(SCREEN_TARGET_WIDTH - 76);
	command_window->SetY(SCREEN_TARGET_HEIGHT-80);
}

void Scene_Battle_Rpg2k::RefreshCommandWindow() {
	std::string skill_name = active_actor->GetSkillName();
	command_window->SetItemText(1,
		skill_name.empty() ? Data::terms.command_skill : skill_name);
}

void Scene_Battle_Rpg2k::SetState(Scene_Battle::State new_state) {
	previous_state = state;
	state = new_state;

	options_window->SetActive(false);
	status_window->SetActive(false);
	command_window->SetActive(false);
	item_window->SetActive(false);
	skill_window->SetActive(false);
	target_window->SetActive(false);
	battle_message_window->SetActive(false);

	switch (state) {
	case State_Start:
		battle_message_window->SetActive(true);
		break;
	case State_SelectOption:
		options_window->SetActive(true);
		break;
	case State_SelectActor:
		status_window->SetActive(true);
		break;
	case State_AutoBattle:
		break;
	case State_SelectCommand:
		command_window->SetActive(true);
		RefreshCommandWindow();
		break;
	case State_SelectEnemyTarget:
		select_target_flash_count = 0;
		break;
	case State_SelectAllyTarget:
		status_window->SetActive(true);
		break;
	case State_Battle:
		// no-op
		break;
	case State_SelectItem:
		item_window->SetActive(true);
		item_window->Refresh();
		break;
	case State_SelectSkill:
		skill_window->SetActive(true);
		skill_window->SetActor(active_actor->GetId());
		skill_window->SetIndex(0);
		break;
	case State_Victory:
	case State_Defeat:
		// no-op
	case State_Escape:
		battle_message_window->SetActive(true);
		break;
	}

	options_window->SetVisible(false);
	status_window->SetVisible(false);
	command_window->SetVisible(false);
	item_window->SetVisible(false);
	skill_window->SetVisible(false);
	help_window->SetVisible(false);
	target_window->SetVisible(false);
	battle_message_window->SetVisible(false);

	switch (state) {
	case State_Start:
		battle_message_window->SetVisible(true);
		break;
	case State_SelectOption:
		options_window->SetVisible(true);
		status_window->SetVisible(true);
		status_window->SetX(76);
		status_window->SetIndex(-1);
		status_window->Refresh();
		break;
	case State_SelectActor:
		SelectNextActor();
		break;
	case State_AutoBattle:
		SetState(State_SelectActor);
		break;
	case State_SelectCommand:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		status_window->SetX(0);
		break;
	case State_SelectEnemyTarget:
		status_window->SetVisible(true);
		command_window->SetVisible(true);
		target_window->SetActive(true);
		target_window->SetVisible(true);
		break;
	case State_SelectAllyTarget:
		status_window->SetVisible(true);
		status_window->SetX(0);
		command_window->SetVisible(true);
		break;
	case State_Battle:
		battle_message_window->SetVisible(true);
		break;
	case State_SelectItem:
		item_window->SetVisible(true);
		item_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);
		break;
	case State_SelectSkill:
		skill_window->SetVisible(true);
		skill_window->SetHelpWindow(help_window.get());
		help_window->SetVisible(true);
		break;
	case State_Victory:
	case State_Defeat:
		// no-op
		break;
	case State_Escape:
		battle_message_window->SetVisible(true);
		break;
	}
}

void Scene_Battle_Rpg2k::NextTurn() {
	Scene_Battle::NextTurn(nullptr);

	auto_battle = false;
}

void Scene_Battle_Rpg2k::ProcessActions() {
	switch (state) {
	case State_Start:
		if (DisplayMonstersInMessageWindow()) {
			SetState(State_SelectOption);
			CheckResultConditions();
		}
		break;
	case State_SelectOption:
		// No Auto battle/Escape when all actors are sleeping or similar
		if (!Main_Data::game_party->IsAnyControllable()) {
			SelectNextActor();
		}
		break;
	case State_SelectActor:
	case State_AutoBattle:
		CheckResultConditions();

		if (help_window->GetVisible() && message_timer > 0) {
			message_timer--;
			if (message_timer <= 0)
				help_window->SetVisible(false);
		}

		break;
	case State_Battle:
		if (!battle_actions.empty()) {
			if (battle_actions.front()->IsDead()) {
				// No zombies allowed ;)
				RemoveCurrentAction();
			}
			else if (ProcessBattleAction(battle_actions.front()->GetBattleAlgorithm().get())) {
				RemoveCurrentAction();
				battle_message_window->Clear();

				if (CheckResultConditions()) {
					return;
				}
			}
		} else {
			// Everybody acted
			actor_index = 0;

			SetState(State_SelectOption);
		}
		break;
	case State_SelectEnemyTarget: {
		std::vector<Game_Battler*> enemies;
		Main_Data::game_enemyparty->GetActiveBattlers(enemies);

		Game_Enemy* target = static_cast<Game_Enemy*>(enemies[target_window->GetIndex()]);
		Sprite_Battler* sprite = Game_Battle::GetSpriteset().FindBattler(target);
		if (sprite) {
			++select_target_flash_count;

			if (select_target_flash_count == 60) {
				sprite->Flash(Color(255, 255, 255, 100), 15);
				select_target_flash_count = 0;
			}
		}
		break;
	}
	case State_Victory:
		Scene::Pop();
		break;
	case State_Defeat:
		if (Player::battle_test_flag || Game_Temp::battle_defeat_mode != 0) {
			Scene::Pop();
		}
		else {
			Scene::Push(EASYRPG_MAKE_SHARED<Scene_Gameover>());
		}
		break;
	case State_Escape:
		Escape();
		break;
	default:
		break;
	}
}

bool Scene_Battle_Rpg2k::ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action) {
	if (Game_Battle::IsBattleAnimationWaiting()) {
		return false;
	}

	Sprite_Battler* source_sprite;

	switch (battle_action_state) {
		case BattleActionState_Start:
			if (battle_action_wait--) {
				return false;
			}
			battle_action_wait = 30;
			battle_message_window->Clear();

			if (!action->IsTargetValid()) {
				if (!action->GetTarget()) {
					// No target but not a target-only action.
					// Maybe a bug report will help later
					Output::Warning("Battle: BattleAction without valid target.");
					return true;
				}

				action->SetTarget(action->GetTarget()->GetParty().GetNextActiveBattler(action->GetTarget()));

				if (!action->IsTargetValid()) {
					// Nothing left to target, abort
					return true;
				}
			}

			action->Execute();

			battle_result_messages.clear();
			action->GetResultMessages(battle_result_messages);

			battle_message_window->Push(action->GetStartMessage());

			action->Apply();

			battle_result_messages_it = battle_result_messages.begin();

			if (action->IsFirstAttack()) {
				if (action->GetTarget() &&
					action->GetTarget()->GetType() == Game_Battler::Type_Enemy &&
					action->GetAnimation()) {

					Game_Battle::ShowBattleAnimation(
						action->GetAnimation()->ID,
						action->GetTarget());
				}
			}

			source_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetSource());
			if (source_sprite) {
				source_sprite->Flash(Color(255, 255, 255, 100), 15);
				source_sprite->SetAnimationState(
					action->GetSourceAnimationState(),
					Sprite_Battler::LoopState_DefaultAnimationAfterFinish);
			}

			if (action->IsFirstAttack() && action->GetStartSe()) {
				Game_System::SePlay(*action->GetStartSe());
			}

			battle_action_state = BattleActionState_Result;

			break;
		case BattleActionState_ConditionHeal:			

			if (action->IsFirstAttack()) {
				std::vector<int16_t> states_to_heal = action->GetSource()->NextBattleTurn();
				std::vector<int16_t> states_remaining = action->GetSource()->GetInflictedStates();
				action->GetSource()->ApplyConditions();
				bool message_to_show = false;
				if (!states_to_heal.empty() || !states_remaining.empty()) {
					battle_message_window->Clear();
					for (auto state : states_to_heal) {
						if (!Data::states[state - 1].message_recovery.empty()) {
							battle_message_window->Push(action->GetSource()->GetName() + Data::states[state- 1].message_recovery);
							message_to_show = true;
						}
					}
					for (auto state : states_remaining) {
						if (!Data::states[state - 1].message_affected.empty()) {
							battle_message_window->Push(action->GetSource()->GetName() + Data::states[state- 1].message_affected);
							message_to_show = true;
						}
					}
					if (message_to_show) {
						battle_action_wait = 30;
					}
					else {
						battle_action_wait = 0;
					}
				}
				else {
					battle_action_wait = 0;
				}
			}

			if (!action->GetTarget()) {
				battle_action_state = BattleActionState_Finished;
			}
			else {
				battle_action_state = BattleActionState_Start;
			}

			break;
		case BattleActionState_Result:
			if (battle_action_wait--) {
				return false;
			}
			battle_action_wait = 30;

			if (battle_result_messages_it != battle_result_messages.end()) {
				Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
				if (battle_result_messages_it == battle_result_messages.begin()) {
					if (action->IsSuccess() && target_sprite) {
						target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Damage);
					}

					if (action->GetResultSe()) {
						Game_System::SePlay(*action->GetResultSe());
					}
				} else {
					if (target_sprite) {
						target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Idle);
					}
				}

				if (battle_result_messages_it != battle_result_messages.begin()) {
					battle_message_window->Pop();
				}
				battle_message_window->Push(*battle_result_messages_it);
				++battle_result_messages_it;
			} else {
				if (action->IsKilledByAttack()) {
					battle_message_window->Push(action->GetDeathMessage());
				}
				battle_action_state = BattleActionState_Finished;
			}

			if (battle_result_messages_it == battle_result_messages.end()) {
				battle_action_state = BattleActionState_Finished;

				if (action->GetTarget() && action->GetTarget()->IsDead()) {
					if (action->GetDeathSe()) {
						Game_System::SePlay(*action->GetDeathSe());
					}

					Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
					if (target_sprite) {
						target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Dead);
					}
				}
			}

			break;
		case BattleActionState_Finished:
			if (battle_action_wait--) {
				return false;
			}
			battle_action_wait = 30;

			if (action->GetTarget() && !action->GetTarget()->IsDead()) {
				Sprite_Battler* target_sprite = Game_Battle::GetSpriteset().FindBattler(action->GetTarget());
				if (target_sprite) {
					target_sprite->SetAnimationState(Sprite_Battler::AnimationState_Idle);
				}
			}

			if (action->TargetNext()) {
				battle_action_state = BattleActionState_ConditionHeal;
				return false;
			}

			// Reset variables
			battle_action_state = BattleActionState_ConditionHeal;

			return true;
	}

	return false;
}

void Scene_Battle_Rpg2k::ProcessInput() {
	if (Input::IsTriggered(Input::DECISION)) {
		switch (state) {
		case State_Start:
			// no-op
			break;
		case State_SelectOption:
			// Interpreter message boxes pop up in this state
			if (!message_window->GetVisible()) {
				OptionSelected();
			}
			break;
		case State_SelectActor:
			SetState(State_SelectCommand);
			SelectNextActor();
			break;
		case State_AutoBattle:
			// no-op
			break;
		case State_SelectCommand:
			CommandSelected();
			break;
		case State_SelectEnemyTarget:
			EnemySelected();
			break;
		case State_SelectAllyTarget:
			AllySelected();
			break;
		case State_Battle:
			// no-op
			break;
		case State_SelectItem:
			ItemSelected();
			break;
		case State_SelectSkill:
			SkillSelected();
			break;
		case State_Victory:
		case State_Defeat:
		case State_Escape:
			// no-op
			break;
		}
	}

	if (Input::IsTriggered(Input::CANCEL)) {
		Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Cancel));
		switch (state) {
		case State_Start:
		case State_SelectOption:
			// no-op
			break;
		case State_SelectActor:
		case State_AutoBattle:
			SetState(State_SelectOption);
			break;
		case State_SelectCommand:
			--actor_index;
			SelectPreviousActor();
			break;
		case State_SelectEnemyTarget:
		case State_SelectItem:
		case State_SelectSkill:
			SetState(State_SelectCommand);
			break;
		case State_SelectAllyTarget:
			SetState(previous_state);
			break;
		case State_Battle:
			// no-op
			break;
		case State_Victory:
		case State_Defeat:
		case State_Escape:
			// no-op
			break;
		}
	}
}

void Scene_Battle_Rpg2k::OptionSelected() {
	switch (options_window->GetIndex()) {
		case 0: // Battle
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			CreateBattleTargetWindow();
			auto_battle = false;
			SetState(State_SelectActor);
			break;
		case 1: // Auto Battle
			auto_battle = true;
			SetState(State_AutoBattle);
			Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
			break;
		case 2: // Escape
			if (!Game_Battle::IsEscapeAllowed()) {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Buzzer));
			}
			else {
				Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));
				SetState(State_Escape);
			}
			break;
	}
}

void Scene_Battle_Rpg2k::CommandSelected() {
	Game_System::SePlay(Game_System::GetSystemSE(Game_System::SFX_Decision));

	switch (command_window->GetIndex()) {
		case 0: // Attack
			AttackSelected();
			break;
		case 1: // Skill
			SetState(State_SelectSkill);
			break;
		case 2: // Defense
			DefendSelected();
			break;
		case 3: // Item
			SetState(State_SelectItem);
			break;
		default:
			// no-op
			break;
	}
}

void Scene_Battle_Rpg2k::Escape() {
	if (begin_escape) {
		battle_message_window->Clear();

		Game_BattleAlgorithm::Escape escape_alg = Game_BattleAlgorithm::Escape(&(*Main_Data::game_party)[0]);

		escape_success = escape_alg.Execute();
		escape_alg.Apply();

		battle_result_messages.clear();
		escape_alg.GetResultMessages(battle_result_messages);

		battle_message_window->Push(battle_result_messages[0]);
		begin_escape = false;
	}
	else {
		++escape_counter;

		if (escape_counter > 60) {
			begin_escape = true;
			escape_counter = 0;

			if (escape_success) {
				Game_Temp::battle_result = Game_Temp::BattleEscape;

				Scene::Pop();
			}
			else {
				SetState(State_Battle);
				CreateEnemyActions();
				CreateExecutionOrder();

				NextTurn();
			}
		}
	}
}

void Scene_Battle_Rpg2k::SelectNextActor() {
	std::vector<Game_Actor*> allies = Main_Data::game_party->GetActors();

	if ((size_t)actor_index == allies.size()) {
		// All actor actions decided, player turn ends
		SetState(State_Battle);
		CreateEnemyActions();
		CreateExecutionOrder();

		NextTurn();
		return;
	}

	active_actor = allies[actor_index];
	status_window->SetIndex(actor_index);
	actor_index++;

	if (active_actor->IsDead()) {
		SelectNextActor();
		return;
	}

	Game_Battler* random_target = NULL;

	if (active_actor->CanAct()) {
		switch (active_actor->GetSignificantRestriction()) {
		case RPG::State::Restriction_attack_ally:
			random_target = Main_Data::game_party->GetRandomActiveBattler();
			break;
		case RPG::State::Restriction_attack_enemy:
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
			break;
		}
	}
	else {
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::NoMove>(active_actor));
		battle_actions.push_back(active_actor);
		SelectNextActor();
		return;
	}

	if (random_target || auto_battle || active_actor->GetAutoBattle()) {
		if (!random_target) {
			random_target = Main_Data::game_enemyparty->GetRandomActiveBattler();
		}

		// ToDo: Auto battle logic is dumb
		active_actor->SetBattleAlgorithm(EASYRPG_MAKE_SHARED<Game_BattleAlgorithm::Normal>(active_actor, random_target));
		battle_actions.push_back(active_actor);

		SelectNextActor();
		return;
	}

	SetState(Scene_Battle::State_SelectCommand);
}

void Scene_Battle_Rpg2k::SelectPreviousActor() {
	std::vector<Game_Actor*> allies = Main_Data::game_party->GetActors();

	if (allies[0] == active_actor) {
		SetState(State_SelectOption);
		actor_index = 0;
		return;
	}

	actor_index--;
	RemoveCurrentAction();
	active_actor = allies[actor_index];

	if (active_actor->IsDead()) {
		SelectPreviousActor();
		return;
	}

	if (active_actor->GetAutoBattle()) {
		SelectPreviousActor();
		return;
	}

	SetState(State_SelectActor);
}

static bool BattlerSort(Game_Battler* first, Game_Battler* second) {
	return first->GetAgi() > second->GetAgi();
}

void Scene_Battle_Rpg2k::CreateExecutionOrder() {
	std::sort(battle_actions.begin(), battle_actions.end(), BattlerSort);
}

void Scene_Battle_Rpg2k::CreateEnemyActions() {
	std::vector<Game_Battler*> active_enemies;
	Main_Data::game_enemyparty->GetActiveBattlers(active_enemies);

	std::vector<Game_Battler*>::const_iterator it;
	for (it = active_enemies.begin(); it != active_enemies.end(); ++it) {
		const RPG::EnemyAction* action = static_cast<Game_Enemy*>(*it)->ChooseRandomAction();
		if (action) {
			CreateEnemyAction(static_cast<Game_Enemy*>(*it), action);
		}
	}
}

bool Scene_Battle_Rpg2k::DisplayMonstersInMessageWindow() {
	if (encounter_message_first_monster) {
		enemy_iterator = Main_Data::game_enemyparty->GetEnemies().begin();
		encounter_message_first_monster = false;
	}

	if (encounter_message_sleep_until > -1) {
		if (Player::GetFrames() >= encounter_message_sleep_until) {
			// Sleep over
			encounter_message_sleep_until = -1;
		} else {
			return false;
		}
	}

	if (enemy_iterator == Main_Data::game_enemyparty->GetEnemies().end()) {
		battle_message_window->Clear();
		encounter_message_first_monster = true; // reset static var
		return true;
	}

	if (battle_message_window->GetLineCount() == 4) {
		battle_message_window->Clear();
	}

	battle_message_window->Push((*enemy_iterator)->GetName() + Data::terms.encounter);

	++enemy_iterator;

	if (enemy_iterator == Main_Data::game_enemyparty->GetEnemies().end() ||
		battle_message_window->GetLineCount() == 4) {
		// Half second sleep
		encounter_message_sleep_until = Player::GetFrames() + 60 / 2;
	} else {
		// 1/10 second sleep
		encounter_message_sleep_until = Player::GetFrames() + 60 / 10;
	}

	return false;
}

bool Scene_Battle_Rpg2k::CheckWin() {
	if (!Main_Data::game_enemyparty->IsAnyActive()) {
		Game_Temp::battle_result = Game_Temp::BattleVictory;
		SetState(State_Victory);

		int exp = Main_Data::game_enemyparty->GetExp();
		int money = Main_Data::game_enemyparty->GetMoney();
		std::vector<int> drops;
		Main_Data::game_enemyparty->GenerateDrops(drops);

		Game_Message::SetPositionFixed(true);
		Game_Message::SetPosition(2);
		Game_Message::SetTransparent(false);

		Game_Message::texts.push_back(Data::terms.victory);

		std::stringstream ss;
		ss << exp << Data::terms.exp_received;
		Game_Message::texts.push_back(ss.str());
		if (money > 0) {
			ss.str("");
			ss << Data::terms.gold_recieved_a << " " << money << Data::terms.gold << Data::terms.gold_recieved_b;
			Game_Message::texts.push_back(ss.str());
		}
		for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			ss.str("");
			ss << Data::items[*it - 1].name << Data::terms.item_recieved;
			Game_Message::texts.push_back(ss.str());
		}

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_Victory));

		// Update attributes
		std::vector<Game_Battler*> ally_battlers;
		Main_Data::game_party->GetActiveBattlers(ally_battlers);

		for (std::vector<Game_Battler*>::iterator it = ally_battlers.begin();
			it != ally_battlers.end(); ++it) {
				Game_Actor* actor = static_cast<Game_Actor*>(*it);
				actor->ChangeExp(actor->GetExp() + exp, true);
		}
		Main_Data::game_party->GainGold(money);
		for (std::vector<int>::iterator it = drops.begin(); it != drops.end(); ++it) {
			Main_Data::game_party->AddItem(*it, 1);
		}

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k::CheckLose() {
	if (!Main_Data::game_party->IsAnyActive()) {
		Game_Temp::battle_result = Game_Temp::BattleDefeat;
		SetState(State_Defeat);

		Game_Message::SetPositionFixed(true);
		Game_Message::SetPosition(2);
		Game_Message::SetTransparent(false);

		Game_Message::texts.push_back(Data::terms.defeat);

		Game_System::BgmPlay(Game_System::GetSystemBGM(Game_System::BGM_GameOver));

		return true;
	}

	return false;
}

bool Scene_Battle_Rpg2k::CheckAbort() {
	/*if (!Game_Battle::terminate)
		return;
	Game_Temp::battle_result = Game_Temp::BattleAbort;
	Scene::Pop();*/

	return false;
}

bool Scene_Battle_Rpg2k::CheckFlee() {
	/*if (!Game_Battle::allies_flee)
		return;
	Game_Battle::allies_flee = false;
	Game_Temp::battle_result = Game_Temp::BattleEscape;
	Scene::Pop();*/

	return false;
}

bool Scene_Battle_Rpg2k::CheckResultConditions() {
	return CheckLose() || CheckWin() || CheckAbort() || CheckFlee();
}
