//
//  HeroActionController.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/29/14.
//
//

#include "HeroActionController.h"
#include "../scriptbind/AIScriptBindActionController.hpp"
#include "../scriptbind/AIScriptBindCharacterData.hpp"
#include "../scriptbind/AIScriptBindSkillController.hpp"
#include "../AIContext.h"
#include "../HeroHurtcalculate.h"
#include "../AIContinuousProjectileCaster.h"
#include "../AIContinuousHurtCaster.h"
#include "../AIContinuousMoveCaster.h"
#include "HeroAttackAction.h"
#include "HeroIdleAction.h"
#include "HeroMoveAction.h"
#include "HeroSkillController.h"
#include "HeroEffectAction.h"
#include "HeroDeadAction.h"
#include "HeroInhurtAction.h"
#include "HeroDefenseAction.h"
#include "HeroStandupAction.h"
#include "HeroStruckBackAction.h"
#include "HeroStruckFlyAction.h"
#include "HeroTurnAction.h"
#include "HeroCelebrateAction.h"
#include "HeroStruckFalldownAction.h"
#include "HeroStruckHoverAction.h"
#include "../AITargetHelper.h"
#include "../AIScriptBinder.hpp"
#include "../scriptbind/AIScriptBindEffectCaster.hpp"

#define AI_LOG_TEST 0

#if AI_LOG_TEST 
#include<fstream>
#endif

namespace OnePiece {
	using namespace std;

	HeroActionController::HeroActionController(){}

	HeroActionController::~HeroActionController(){}

	HeroActionController* HeroActionController::getInstance()
	{
		static HeroActionController* isme = NULL;
		if (isme == NULL) isme = new HeroActionController();

		return isme;
	}

	void HeroActionController::update(CharacterData& aiCharacter, AIContext& aiContext)
	{

#if AI_LOG_TEST && COCOS2D_DEBUG
		///if (aiCharacter.myID != 0){
			std::ofstream outfream_ai;
			outfream_ai.open("d:\\AiLog.txt", ios::app | ios::ate);
			outfream_ai << "tick:" << aiContext.currentTick << ",id:" << aiCharacter.myID << ",CardId:" << aiCharacter.getID() << ",Target:" << (aiCharacter.target?  aiCharacter.target->myID : 9999) << ",Hp:" << aiCharacter.getHP() << ",x:" << aiCharacter.getX() << ",y:" << aiCharacter.getY() << ",action:" << aiCharacter.currentAction << ",seed:" << aiContext.localRandom->getSeed() << endl;
			outfream_ai.close();
		//}
#endif

		
		if (aiCharacter.k < 1)
		{
			if (!aiContext.monsterCanMove) return;
		}

		switch (aiCharacter.currentAction)
		{
		case ActionType::ATTACK_ACTION:
		case ActionType::DEATH_TRIGGERED_ATTACK_ACTION:
			//if (aiCharacter.isFrozen) return;
			HeroAttackAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::IDLE_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroIdleAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::MOVE_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroMoveAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::EFFECT_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroEffectAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::DEAD_ACTION1:
			HeroDeadAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::DEAD_ACTION2:
			HeroDeadAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::DEAD_ACTION3:
			HeroDeadAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::IN_HURT_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroInhurtAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::DEFENSE_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroDefenseAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::STANDUP_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroStandupAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::STRUCK_BACKWORD_ACTION:
			if (aiCharacter.isFrozen) aiCharacter.localTick++;
			HeroStruckBackAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::STRUCK_FLY_AWAY_ACTION:
			if (aiCharacter.isFrozen) aiCharacter.localTick++;
			HeroStruckFlyAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::FALL_DOWN_ACTION:
			if (aiCharacter.isFrozen) aiCharacter.localTick++;
			HeroStruckFalldownAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::TURN_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroTurnAction::Excute(aiCharacter, aiContext);
			break;
		case ActionType::CELEBRATE_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroCelebrateAction::Excute(aiCharacter, aiContext);
			break;
        case ActionType::STRUCK_HOVER_ACTION:
            if (aiCharacter.isFrozen) return;
            HeroStruckHoverAction::Excute(aiCharacter, aiContext);
            break;
		case ActionType::CONTROLLED_EFFECT_ACTION:
			if (aiCharacter.isFrozen) return;
			HeroOutofControlAction::Excute(aiCharacter, aiContext);
			break;
		default:
			break;
		}

		if (aiCharacter.isFrozen) return;

		if (getSkillActionCD(aiCharacter.actionAtt) > 0) {
			setSkillActionCD(aiCharacter.actionAtt, getSkillActionCD(aiCharacter.actionAtt) - 1);
		}

        HeroSkillController::updateSpecialSkillCD(&aiCharacter);
	}

	void HeroActionController::cleanup(CharacterData& aiCharacter, AIContext& aiContext, bool ifHurtEffect, bool isCleanTransform)
	{
		for (std::list<AIContinuousProjectileCaster*>::iterator iter = aiCharacter.projectileCasts.begin(); iter != aiCharacter.projectileCasts.end();)
		{
			delete *iter++;
		}
		aiCharacter.projectileCasts.clear();

		for (std::list<AIContinuousMoveCaster*>::iterator iter3 = aiCharacter.moveCasts.begin(); iter3 != aiCharacter.moveCasts.end();++iter3)
		{
            if (ifHurtEffect || !(*iter3)->shouldKeepCaster())
            {
                //(*iter3)->totalTick = 0;
                (*iter3)->shouldDeleteSelf = true;
            }
		}

		if (ifHurtEffect)
		{
			for (std::list<AIContinuousEffectCaster*>::iterator iter1 = aiCharacter.effectCasts.begin(); iter1 != aiCharacter.effectCasts.end();)
			{
				delete *iter1++;
			}
			aiCharacter.effectCasts.clear();

			for (std::list<AIContinuousHurtCaster*>::iterator iter2 = aiCharacter.hurtCasts.begin(); iter2 != aiCharacter.hurtCasts.end();)
			{
				//delete *iter2++;
                //aiCharacter.hurtCastsToDelete.push_back(*iter2);
                AIContinuousHurtCaster::addRemainCaster(*iter2);
                (*iter2++)->onFree();
			}
			aiCharacter.hurtCasts.clear();

			for (std::list<AIContinuousSummonCaster*>::iterator iter4 = aiCharacter.summonCasts.begin(); iter4 != aiCharacter.summonCasts.end();)
			{
				delete *iter4++;
			}
			aiCharacter.summonCasts.clear();

            if (isCleanTransform)
                aiCharacter.clearTransformCast();

            aiCharacter.markDeadImmunes.clear();

            aiCharacter.innerEffectIntervals.clear();
		}
    }
    
    void HeroActionController::safeCleanupCasters(CharacterData& aiCharacter, AIContext& aiContext)
    {
        std::list<AIContinuousProjectileCaster *> projList;
        projList.swap(aiCharacter.projectileCasts);
        for (auto pc : projList)
            delete pc;
        
        std::list<AIContinuousMoveCaster *> moveList;
        moveList.swap(aiCharacter.moveCasts);
        for (auto pc : moveList)
            delete pc;
        
        std::list<AIContinuousEffectCaster *> effectList;
        effectList.swap(aiCharacter.effectCasts);
        for (auto ac : effectList)
            delete ac;
        
        std::list<AIContinuousHurtCaster *> hurtList;
        hurtList.swap(aiCharacter.hurtCasts);
        for (auto pc : hurtList)
            delete pc;
        
        std::list<AIContinuousSummonCaster *> summonList;
        summonList.swap(aiCharacter.summonCasts);
        for (auto pc : summonList)
            delete pc;
        
        aiCharacter.clearTransformCast();
    }
    
    void HeroActionController::exitAction(CharacterData& aiCharacter, AIContext& aiContext)
    {
        luahelper_actioncontroller_exit_action(aiContext.getLuaState(), &aiCharacter);
    }

    void HeroActionController::exitActionNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        switch (aiCharacter.currentAction) {
        case ActionType::STRUCK_FLY_AWAY_ACTION:
            HeroStruckFlyAction::Exit(aiCharacter, aiContext);
            break;
        case ActionType::STRUCK_BACKWORD_ACTION:
            HeroStruckBackAction::Exit(aiCharacter, aiContext);
            break;
        case ActionType::FALL_DOWN_ACTION:
		case ActionType::CONTROLLED_EFFECT_ACTION:
            HeroOutofControlAction::Exit(aiCharacter, aiContext);
            break;
        case ActionType::STRUCK_HOVER_ACTION:
            HeroStruckHoverAction::Exit(aiCharacter, aiContext);
            break;
        case ActionType::EFFECT_ACTION:
            HeroEffectAction::Exit(aiCharacter, aiContext);
            break;
        case ActionType::ATTACK_ACTION:
            HeroAttackAction::Exit(aiCharacter, aiContext);
            break;
        default:break;
        }

        AICastByStateEffect::checkExecuteExitAction(aiContext, &aiCharacter, aiCharacter.currentAction);
    }

    void HeroActionController::didEnterAction(CharacterData& aiCharacter, AIContext& aiContext)
    {
        luahelper_actioncontroller_did_enter_action(aiContext.getLuaState(), &aiCharacter);
    }
    
    void HeroActionController::didEnterActionNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiCharacter.currentAction != EFFECT_ACTION)
        {
            HeroActionController::setEffectID(aiCharacter.actionAtt, 0);
        }
        AICastBySpecState::checkExecute(aiContext, &aiCharacter, aiCharacter.actionCauserMyID, aiCharacter.currentAction);
        AICastByStateEffect::checkExecuteEnterAction(aiContext, &aiCharacter, aiCharacter.currentAction);
        AIEffect281::checkExecuteEnterAction(aiContext, &aiCharacter);
    }

    void HeroActionController::actionSelector(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (AIScriptBinder::getInstance()->isActionSelectorByLua(aiContext.getLuaState()))
            luahelper_actioncontroller_action_selector(aiContext.getLuaState(), &aiCharacter);
        else
            actionSelectorNative(aiCharacter, aiContext);
    }

	void HeroActionController::actionSelectorNative(CharacterData& aiCharacter, AIContext& aiContext)
	{
        if (aiCharacter.isCaptain())
        {
            captainActionSelector(aiCharacter, aiContext);
            return;
        }

        if (isDeadAction(aiCharacter.currentAction))
            return;

		// 295  
		if (luahelper_innereffect_295_checkdo(AIContext::getInstance()->getLuaState(), &aiCharacter, aiCharacter.currentAction) > 0)
		{
			return;
		}

        // 129
		if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_WUDI_NOSKILL_DISPEAR, 0) == 1)
		{
			return;
		}

        //if (aiCharacter.currentAction == DEATH_TRIGGERED_ATTACK_ACTION)
        if (isDeadAttacking(&aiCharacter))
		{
            //aiCharacter.currentAction = ATTACK_ACTION;
            //setOldAction(aiCharacter.actionAtt, ActionType::DEATH_TRIGGERED_ATTACK_ACTION);
            aiCharacter.eraseTag("deadattacking");
            if (aiCharacter.getHP() > 0)
            {
                setOldAction(aiCharacter.actionAtt, ActionType::ATTACK_ACTION);
                HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION, aiCharacter);
            }
            else
            {
                HeroDeadAction::Enter(aiCharacter, aiContext, 1);
            }
			return;
		}

        // MARK: 每次切换状态都重新选择目标(mod by h7ing)
        //CharacterData* target = aiCharacter.target;
        //
        //if (target == nullptr || isDeadAction(target->currentAction) || !target->canBeAttacked())
        //    target = aiCharacter.lockTarget(aiContext);

        // MARK: 6.0后,针对技能目标是寻找最高最低属性的,需要用技能目标判断是否需转向,而不是最近的目标
        //CharacterData* target = aiCharacter.lockTarget(aiContext);

		if (aiCharacter.needAdjusting)
		{
			HeroMoveAction::Enter(aiCharacter, aiContext);
			return;
		}

        // 266
        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_STATE_VERTIGO, 0) > 0)
        {
            if (AIStateVertigoEffect::checkExecute(aiContext, &aiCharacter))
                return;
        }


        //if (!target) {
        //    HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION);
        //    return;
        //}

		//HeroActionController::setCurrentTarget(aiCharacter, target);

		////这里考虑被魅惑后需要的朝向
        //bool cannotTurn = (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_NO_TURN_ACTION, 0) > 0);
        //if ((target->getX() - aiCharacter.getX())*aiCharacter.direction < 0 && !cannotTurn) {
		//	HeroTurnAction::Enter(aiCharacter, aiContext);
		//	return;
		//}

        SkillSeqInfo skilldata(-1,-1,-1,-1);
        if (AIScriptBinder::getInstance()->isPickSkillByLua(aiContext.getLuaState()))
            luahelper_skillcontroller_pickskill(aiContext.getLuaState(), &aiCharacter, &skilldata);
        else
            HeroSkillController::pickSkill(aiCharacter, aiContext, &skilldata);

		if (skilldata.skillIDIndex == -1 && skilldata.specialSkillID <= 0) {
			HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
			return;
		}

		int no_attack = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_NO_ATTACK_MARK, 0); // 115
		int no_skill = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_NO_SKILL_MARK, 0); // 114
		int only_skill1 = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_ONLY_FINAL_SKILL, 0); // 124
        bool ignore_silence = HeroSkillController::isTargetIgnoreSilenceBySkillIndex(aiContext, &aiCharacter, skilldata.skillIDIndex);
        if (ignore_silence)
        {
            no_attack = 0;
            no_skill = 0;
        }

		if (skilldata.skillIDIndex == 0) //normal attack
		{
			if (no_attack > 0 || only_skill1 > 0)
			{
				if (skilldata.attackSeqIndex > -1)
				{
                    // 126
					if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0) == 1)
					{
						aiCharacter.m_iSkillSeq2Index = skilldata.attackSeqIndex;
					}
					else
					{
						setSkillSeq(aiCharacter.actionAtt, skilldata.attackSeqIndex);
					}

					setSkillID(aiCharacter.actionAtt, HeroSkillController::calculateSkillID(aiContext, aiCharacter, skilldata.attackSeqIndex));
				}

				HeroIdleAction::Enter(aiCharacter, aiContext, aiCharacter.getAInterval(), IDLE_ACTION, aiCharacter);
				return;
			}

		}
		else {	//skill attack
			if (no_skill > 0)
				skilldata.skillIDIndex = 0;
			else if (only_skill1 > 0 && skilldata.skillIDIndex != 1) {
				if (skilldata.attackSeqIndex > -1) {
					if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0) == 1)
						aiCharacter.m_iSkillSeq2Index = skilldata.attackSeqIndex;
					else
						setSkillSeq(aiCharacter.actionAtt, skilldata.attackSeqIndex);

					setSkillID(aiCharacter.actionAtt, HeroSkillController::calculateSkillID(aiContext, aiCharacter, skilldata.attackSeqIndex));
				}

				HeroIdleAction::Enter(aiCharacter, aiContext, aiCharacter.getAInterval(), IDLE_ACTION, aiCharacter);
				return;
			}
		}

        int skillID = skilldata.specialSkillID;
        if (skillID <= 0)
		    skillID = CharacterFunc::GetSkillIDByIndex(aiCharacter, skilldata.skillIDIndex);
		std::vector<std::pair<int, int>>::iterator* pSkillIter = nullptr;
		std::vector<std::pair<int, int>>::iterator iter = aiCharacter.skillList.begin();
		while (iter != aiCharacter.skillList.end()) {
			if (iter->first == skillID) {
                pSkillIter = &iter;
				break;
			}
            ++iter;
		}

		skillID = aiCharacter.normalSkill.first;
		int skillLV = aiCharacter.normalSkill.second;
        // 248
        bool isInForceAttackSelf = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_FORCE_ATTACK_SELF, 0);
        if (isInForceAttackSelf)
        {
            pSkillIter = nullptr;
        }

        if (pSkillIter != nullptr) {
            if (!HeroSkillController::isSkillSwapWithNormal((*pSkillIter)->first)) {
                skillID = (*pSkillIter)->first;
                skillLV = (*pSkillIter)->second;
			}
		}


        if (!skillID) {
            HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
            return;
        }

        AISkill* skill = AIDataHelper::getSkillData(skillID);
        if (!skill) {
            HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
            return;
        }

        bool isJudgeBy297 = HeroSkillController::isSkillTargetForSingle(skill);
        CharacterData* target = aiCharacter.lockTarget(aiContext, isJudgeBy297);
        aiCharacter.specialFoundSkillTarget = nullptr;
        //int realTargetType = HeroSkillController::getRealSearchInfo(skill->targetType).realSearchType;
        int realTargetType = AITargetHelper::parseTargetType(skill->targetType, nullptr, nullptr, nullptr, nullptr);
        if (isInForceAttackSelf) {
            target = &aiCharacter;
            HeroActionController::setCurrentTarget(aiCharacter, target);
        } else if (realTargetType == TargetSelfAttackRangeRandomEnemy) {
            bool shouldRetarget = true;
            if (aiCharacter.mAttackConfirmedTarget) {
                shouldRetarget = (!aiCharacter.target || HeroActionController::isDeadAction(aiCharacter.target->currentAction));
            }

            if (shouldRetarget) {
                if (AITargetHelper::isUseNewHelper()) {
                    AITargetContext ctx;
                    ctx.source = &aiCharacter;
                    ctx.target = target;
                    ctx.skill_caster = &aiCharacter;
                    ctx.skill_id = skillID;
                    ctx.is_using_skill = true;
                    ctx.search_type = skill->targetType;
                    ctx.range.x1 = -1 * skill->range;
                    ctx.range.x2 = skill->range;
                    ctx.range.y1 = -1 * skill->range;
                    ctx.range.y2 = skill->range;
                    ctx.search_params = &skill->vTargetParms;
                    std::list<CharacterData *> currentTargets;
                    AITargetHelper::searchTarget(&ctx, &currentTargets);
                    if (currentTargets.size()) {
                        target = currentTargets.front();
                        HeroActionController::setCurrentTarget(aiCharacter, target);
                        aiCharacter.mAttackConfirmedTarget = true;
                    }
                } else {
                    auto currentTargets = AIActionController::searchTarget(aiCharacter, target, nullptr, aiContext, skill->targetType, -1 * skill->range, skill->range, -1 * skill->range, skill->range, -1 * skill->range, skill->range, &skill->vTargetParms, aiCharacter, true, skillID);
                    /* TargetSelfAttackRangeRandomEnemy,// = 41
                     考虑自身攻击范围的随机敌人，比如近战A随机到远程B，那么A必须走到B附近才能攻击B。
                     只适用于随机一个敌人的情况
                     */
                    if (currentTargets && currentTargets->size() > 0) {
                        target = currentTargets->front();
                        HeroActionController::setCurrentTarget(aiCharacter, target);
                        aiCharacter.mAttackConfirmedTarget = true;
                    }
                }
            }
		} else {
            bool findSkillTarget = false;
            // 30,31,32,45,46,47
            switch (realTargetType) {
            case TargetAllAttrHighest:
            case TargetFriendAttrHighest:
            case TargetEnemyAttrHighest:
            case TargetAllAttrLowest:
            case TargetFriendAttrLowest:
            case TargetEnemyAttrLowest:
                findSkillTarget = true;
                break;
            default:break;
            }

            if (findSkillTarget) {
                if (AITargetHelper::isUseNewHelper()) {
                    AITargetContext ctx;
                    ctx.source = &aiCharacter;
                    ctx.target = target;
                    ctx.skill_caster = &aiCharacter;
                    ctx.skill_id = skillID;
                    ctx.is_using_skill = true;
                    ctx.search_type = skill->targetType;
                    ctx.range.x1 = -1 * skill->range;
                    ctx.range.x2 = skill->range;
                    ctx.range.y1 = -1 * skill->range;
                    ctx.range.y2 = skill->range;
                    ctx.search_params = &skill->vTargetParms;
                    std::list<CharacterData *> currentTargets;
                    AITargetHelper::searchTarget(&ctx, &currentTargets);
                    if (currentTargets.size())
                        aiCharacter.specialFoundSkillTarget = currentTargets.front();
                } else {
                    auto targets = AIActionController::searchTarget(aiCharacter, target, nullptr, aiContext, skill->targetType, -1 * skill->range, skill->range, -1 * skill->range, skill->range, -1 * skill->range, skill->range, &skill->vTargetParms, aiCharacter, true, skillID);
                    if (targets->size() > 0)
                        aiCharacter.specialFoundSkillTarget = targets->front();
                }
            }

            HeroActionController::setCurrentTarget(aiCharacter, target);
        }

        if (aiCharacter.hasTag("skonbegin"))
            return;

        if (!target) {
            HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
            return;
        }

		if (HeroSkillController::isTargetInSkilllRange(&aiCharacter, skillID, target) && 
            aiContext.getOwnerIsInVaildBarrier(&aiCharacter)) {

            int skillIDForSkipCD = 0;
            int skillLvForSkipCD = 0;
			if (HeroActionController::getSkillActionCD(aiCharacter.actionAtt) > 0)
            {
                if (HeroSkillController::isSkillIgnoreCD(skillID))
                    HeroActionController::setSkillActionCD(aiCharacter.actionAtt, 0);

                // 240
                skillIDForSkipCD = AISkipCDCastEffect::checkExcute(aiContext, &aiCharacter);
                if (skillIDForSkipCD > 0)
                {
                    for (auto theIter = aiCharacter.skillList.begin(); theIter != aiCharacter.skillList.end(); ++theIter)
                    {
                        if (skillIDForSkipCD == theIter->first)
                        {
                            skillLvForSkipCD = theIter->second;
                            break;
                        }
                    }
                }

                if (skillIDForSkipCD > 0 && skillLvForSkipCD > 0)
                {
                    // 240效果触发技能优先级比技能释放条件16低
                    if (skilldata.specialSkillID > 0 && skilldata.specialSkillCastContiond == eSkillCastConditionOnHurtPlus)
                    {
                        skillIDForSkipCD = 0;
                        skillLvForSkipCD = 0;
                    }

                    HeroActionController::setSkillActionCD(aiCharacter.actionAtt, 0);
                }
                else
                {
                    HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION, aiCharacter);
                    return;
                }
			}

            //这里考虑被魅惑后需要的朝向
            auto targetForDirection = target;
            if (aiCharacter.specialFoundSkillTarget)
                targetForDirection = aiCharacter.specialFoundSkillTarget;
            // 182
            bool cannotTurn = (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_NO_TURN_ACTION, 0) > 0);
            if ((targetForDirection->getX() - aiCharacter.getX())*aiCharacter.direction < 0 && !cannotTurn) {
                //HeroTurnAction::Enter(aiCharacter, aiContext);
                luahelper_actioncontroller_on_actionselector_turntotarget(aiContext.getLuaState(), &aiCharacter);
                return;
            }

            if (skillIDForSkipCD > 0 && skillLvForSkipCD > 0)
            {
                // 240
                AIContinuousEffectCaster::change_Special_Effect(aiContext, aiCharacter, eEFFECT_SKIP_SKILLCD_NCAST, 1);
                HeroAttackAction::ForceEnter(aiCharacter, aiContext, skilldata.attackSeqIndex - 1, skillIDForSkipCD, skillLvForSkipCD, true, 0, false);
                aiCharacter.addTag("nocdattack");
            }
            else
            {
                HeroAttackAction::Enter(aiCharacter, aiContext, skilldata.attackSeqIndex, skillID, skillLV, true, 0, false);
            }

            // 239
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_SLIP_NCAST, 0) > 0)
            {
                auto theSlipEff = AISlipCastEffect::checkExcute(aiContext, &aiCharacter);
                if (theSlipEff && theSlipEff->slipTickLen >= 0)
                {
                    // TODO: EffectHelper::getExtendHitdownNFlyChangeValue
                    HeroOutofControlAction::Enter(aiCharacter, aiContext, theSlipEff->slipTickLen, ActionType::FALL_DOWN_ACTION, *theSlipEff->origin);
                    HeroActionController::setEffectID(aiCharacter.actionAtt, 0);
                    AIExcuteOnHurtResultEffect::checkExcute(&aiCharacter);
                }
            }
		}
		else
		{
            HeroActionController::setNextSkillID(aiCharacter.actionAtt, skillID);
			HeroMoveAction::Enter(aiCharacter, aiContext);
		}
    }
    
    void HeroActionController::captainActionSelector(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (AIScriptBinder::getInstance()->isCaptainSelectorByLua(aiContext.getLuaState()))
            luahelper_actioncontroller_captain_action_selector(aiContext.getLuaState(), &aiCharacter);
        else
            captainActionSelectorNative(aiCharacter, aiContext);
    }

    void HeroActionController::captainActionSelectorNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiCharacter.currentAction == ATTACK_ACTION)
        {
            HeroIdleAction::Enter(aiCharacter, aiContext, 0, ActionType::IDLE_ACTION, aiCharacter);
            return;
        }

        int currentteamhpratio = -1;
        int currentenemyhpratio = -1;
        int deadcnt = -1;
        int enemydeadcnt = -1;
        double teamhptop = aiCharacter.teamSide == TeamSide::kOur ? aiContext.currentClusterHPTopOur : aiContext.currentClusterHPTopEnemy;
        if (teamhptop <= 0)
            teamhptop = 1;
        double enemyhptop = aiCharacter.teamSide == TeamSide::kOur ? aiContext.currentClusterHPTopEnemy : aiContext.currentClusterHPTopOur;
        if (enemyhptop <= 0)
            enemyhptop = 1;

        for (auto iter = aiCharacter.remainCaptainSkills.begin(); iter != aiCharacter.remainCaptainSkills.end();)
        {
            bool isnext = false;
            auto &&theskill = *iter;

            if (theskill.seqType == CaptainSeqType::kSeqTime)
            {
                if (theskill.seqValue <= aiContext.currentTick)
                {
                    isnext = true;
                }
            }
            else if (theskill.seqType == CaptainSeqType::kSeqHurt)
            {
                if (currentteamhpratio < 0)
                {
                    auto friends = aiCharacter.getOriginFriends();
                    double currentHP = 0;
                    for (auto frienditer = friends->begin(); frienditer != friends->end(); ++frienditer)
                    {
                        auto pCharacter = *frienditer;
                        if (!pCharacter->isSummon())
                            currentHP += pCharacter->getHP();
                    }

                    float ratio = currentHP / teamhptop;
                    currentteamhpratio = ratio * 100.0;
                }

                if (currentteamhpratio <= theskill.seqValue)
                {
                    isnext = true;
                }
            }
            else if (theskill.seqType == CaptainSeqType::kSeqDead)
            {
                if (deadcnt < 0)
                {
                    deadcnt = 0;

                    auto friends = aiCharacter.getOriginFriends();
                    for (auto frienditer = friends->begin(); frienditer != friends->end(); ++frienditer)
                    {
                        auto pCharacter = *frienditer;
                        if (!pCharacter->isSummon() && HeroActionController::isDeadAction(pCharacter->currentAction))
                            deadcnt++;
                    }
                }

                if (deadcnt >= theskill.seqValue)
                {
                    isnext = true;
                }
            }
            else if (theskill.seqType == CaptainSeqType::kSeqHurtEnemy)
            {
                if (currentenemyhpratio < 0)
                {
                    auto cs = aiCharacter.getOriginRivals();
                    double currentHP = 0;
                    for (auto citer = cs->begin(); citer != cs->end(); ++citer)
                    {
                        auto c = *citer;
                        if (!c->isSummon())
                            currentHP += c->getHP();
                    }

                    float ratio = currentHP / enemyhptop;
                    currentenemyhpratio = ratio * 100.0;
                }

                if (currentenemyhpratio <= theskill.seqValue)
                {
                    isnext = true;
                }
            }
            else if (theskill.seqType == CaptainSeqType::kSeqDeadEnemy)
            {
                if (enemydeadcnt < 0)
                {
                    enemydeadcnt = 0;

                    auto cs = aiCharacter.getOriginRivals();
                    for (auto citer = cs->begin(); citer != cs->end(); ++citer)
                    {
                        auto c = *citer;
                        if (!c->isSummon() && HeroActionController::isDeadAction(c->currentAction))
                            enemydeadcnt++;
                    }
                }

                if (enemydeadcnt >= theskill.seqValue)
                {
                    isnext = true;
                }
            }

            if (isnext)
            {
                aiCharacter.nextCaptainSkills.push_back(theskill);
                iter = aiCharacter.remainCaptainSkills.erase(iter);
            }
            else
                ++iter;
        }

        if (aiCharacter.nextCaptainSkills.size() > 0)
        {
            auto nextIter = aiCharacter.nextCaptainSkills.begin();
            auto &&nextSkill = *nextIter;
            if (nextSkill.skillID > 0)
            {
                HeroAttackAction::Enter(aiCharacter, aiContext, nextSkill.seqIdx, nextSkill.skillID, nextSkill.skillLevel, true, 0, false);
                for (auto castid : nextSkill.castIDs)
                    aiContext.scheduleExcute(castid, nextSkill.skillID, nextSkill.skillLevel, &aiCharacter, nullptr, &aiCharacter, 0);
            }

            aiCharacter.nextCaptainSkills.erase(nextIter);
        }
    }

	void HeroActionController::setCurrentTarget(CharacterData& aiCharacter, CharacterData* target)
	{
		if (target)
		{
            if (aiCharacter.isInDuel() || target->isInDuel())
            {
                if (!aiCharacter.isDuelWithTarget(target))
                    return;
            }
			//aiCharacter.setZ(target->getZ());
			aiCharacter.target = target;
		}
	}

	int HeroActionController::distanceToCharacter(CharacterData* chara1, CharacterData* chara2)
	{
        //return sqrt(distanceSquare(chara1, chara2));
        int x = chara1->getX() - chara2->getX();
        int y = chara1->getY() - chara2->getY();
        int distance = sqrt(x*x + y*y);
        return distance;
	}

    //uint32_t HeroActionController::distanceSquare(CharacterData* chara1, CharacterData* chara2)
    //{
    //    int x = chara1->getX() - chara2->getX();
    //    int y = chara1->getY() - chara2->getY();
    //    return x*x + y*y;
    //}

    bool HeroActionController::isDeadAction(ActionType action)
    {
        return (ActionType::DEAD_ACTION1 == action ||
                ActionType::DEAD_ACTION2 == action ||
                ActionType::DEAD_ACTION3 == action);
    }

    bool HeroActionController::isDeadAttacking(CharacterData *aiCharacter)
    {
        return (ATTACK_ACTION == aiCharacter->currentAction && aiCharacter->hasTag("deadattacking"));
    }

    bool HeroActionController::releaseAccumulateSkill(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (ActionType::ATTACK_ACTION != aiCharacter.currentAction)
            return false;

        auto todoTick = HeroSkillController::calculateAccumulateSkillReleaseLength(HeroActionController::getSkillID(aiCharacter.actionAtt));
        aiCharacter.totalTick = aiCharacter.localTick - aiCharacter.startTick + todoTick;
        HeroAttackAction::ReleaseAccumulate(aiCharacter, aiContext);
        //HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION);
        return true;
    }

    void HeroActionController::releaseSkillOnBegin(AIContext& aiContext, CharacterData *c, bool isProtective, int lazyTick)
    {
        c->setTagValue("skonbegin", isProtective ? 1 : 2);
        if (c->isCaptain())
            captainActionSelector(*c, aiContext);
        else
            HeroActionController::actionSelector(*c, aiContext);

        for (auto skilliter = c->skillsOnBegin.begin(); skilliter != c->skillsOnBegin.end(); ++skilliter)
        {
            auto skillID = skilliter->first;
            auto skillLevel = skilliter->second;
            AISkill* aiSkill = AIDataHelper::getSkillData(skillID);
            if (aiSkill && !HeroActionController::isDeadAction(c->currentAction))
            {
                HeroSkillController::executeBeginSkillExecIDInTick(aiContext, skillID, skillLevel, 0, c, c->target, aiSkill, isProtective, lazyTick);

				if (aiContext.aiCallback && c->isCaptain())
				{
					aiContext.aiCallback->captainCastBeginSkill(c->myID, c->getID(), c->teamSide, skillID, isProtective);
				}
            }
        }

        if (!isProtective)
        {
            CharacterFunc::CopyOldFashionAttributes(c->attributes, c->attributesAfterBegin.get());
        }

        c->eraseTag("skonbegin");
    }

    void HeroActionController::teamReleaseSkillOnBegin(AIContext& aiContext, CharacterDataListPtr pCharacterList, bool isProtective, int lazyTick)
    {
        for (auto iter = pCharacterList->begin(); iter != pCharacterList->end(); ++iter)
        {
            releaseSkillOnBegin(aiContext, *iter, isProtective, lazyTick);
        }
    }
}

