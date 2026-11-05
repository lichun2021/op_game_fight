//
//  HeroAttackAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "HeroAttackAction.h"
#include "cocos2d.h"
#include "HeroIdleAction.h"
#include "HeroActionController.h"
#include "HeroSkillController.h"
#include "../HeroHurtcalculate.h"
#include "../AIProjectile.h"
#include "../ExternalDelegateCombo.h"
#include "../scriptbind/AIScriptBindActionController.hpp"
#include "../scriptbind/AIScriptBindEffectCaster.hpp"
#include "../AIScriptBinder.hpp"

namespace OnePiece {

    void HeroAttackAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (AIScriptBinder::getInstance()->isAttackActionUpdateByLua(aiContext.getLuaState()))
            luahelper_attackaction_update(aiContext.getLuaState(), &aiCharacter);
        else
            UpdateNative(aiCharacter, aiContext);
    }

    static bool isRepeatSkillHasEnding(int aSkillId)
    {
        auto skillData = AIDataHelper::getSkillData(aSkillId);
        for (auto &an_exec : skillData->skillExec) {
            if (an_exec.execId > 0 && an_exec.param1 == 2)
                return true;
        }
        
        return false;
    }

    static int getRepeatSkillEndingTick(int aSkillId)
    {
        int endingTick = -1;
        auto skillData = AIDataHelper::getSkillData(aSkillId);
        for (auto &an_exec : skillData->skillExec) {
            if (an_exec.execId > 0 && an_exec.param1 == 2) {
                int anExecTick = an_exec.param0 + HeroSkillController::calculateExecLength(an_exec.execId);
                if (anExecTick > endingTick)
                    endingTick = anExecTick;
            }
        }
        
        return endingTick;
    }
    
    static void executeRepeatendingExecIdInTick(int skillId, int skillLv, AIContext &ai, int deltaTick, CharacterData *s, CharacterData *t)
    {
        auto skillData = AIDataHelper::getSkillData(skillId);
        if (!skillData)
            return;
        
        for (auto &an_exec : skillData->skillExec) {
            // 2 - 循环结束后执行
            if (an_exec.execId > 0 && an_exec.param1 == 2) {
                int compareTick = an_exec.param0;
                if (s->attackSpeed > 0 && s->attackSpeed != 100)
                    compareTick = compareTick * s->attackSpeed / 100;
                if (compareTick == deltaTick)
                    AIActionController::executeExec(ai, an_exec.execId, skillId, skillLv, s, nullptr, t, 0);
            }
        }
    }

    static void updateRepeatSkillEnding(CharacterData &c, AIContext &ai, int skillId, int skillLv)
    {
        if (c.localTick < c.startTick)
            return;
        
        executeRepeatendingExecIdInTick(skillId, skillLv, ai, c.localTick - c.startTick, &c, c.target);
        
        if (c.localTick - c.startTick > c.totalTick) {
            if (c.hasTag("repeatskillnotinterruptbeforeending"))
                c.addTag("attacknotinterrupt");
            HeroIdleAction::Enter(c, ai, 1, ActionType::IDLE_ACTION, c);
        }
    }

    void HeroAttackAction::UpdateNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiCharacter.localTick < aiCharacter.startTick)
            return;

		int yingzhi = HeroActionController::getYingzhi(aiCharacter.actionAtt);
		if (yingzhi > 0)
			HeroActionController::setYingzhi(aiCharacter.actionAtt, yingzhi-1);

        int skillID = HeroActionController::getSkillID(aiCharacter.actionAtt);

        if (aiCharacter.hasTag("repeatskill") && !aiCharacter.hasTag("repeatskillfirstbasicround") && !aiCharacter.hasTag("repeatskillnointerrupt") && aiCharacter.hasTag("repeatskillinterrupt")) {
            if (!isRepeatSkillHasEnding(skillID)) {
                HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION, aiCharacter);
                return;
            } else {
                aiCharacter.eraseTag("repeatskill");
                aiCharacter.addTag("repeatskillending");
                aiCharacter.startTick = aiCharacter.localTick + 1;
                aiCharacter.totalTick = getRepeatSkillEndingTick(skillID);
            }
        }

        if (aiCharacter.hasTag("repeatskillending")) {
            updateRepeatSkillEnding(aiCharacter, aiContext, skillID, HeroActionController::getSkillLv(aiCharacter.actionAtt));
            return;
        }
        
        HeroSkillController::executeExecIDInTick(aiContext, skillID,
			HeroActionController::getSkillLv(aiCharacter.actionAtt),
            aiCharacter.localTick - aiCharacter.startTick,
			aiCharacter,
			*aiCharacter.target);

        if (aiCharacter.hasTag("attacknobrokenwhenenter"))
            aiCharacter.eraseTag("attacknobrokenwhenenter");

        if (aiCharacter.hasTag("energyskill")) {
            auto theEnergySkillData = AIDataHelper::getSkillData(aiCharacter.currentEnergySkillId);
            int theEnergySkillMaxTick = aiCharacter.totalTick;
            if (theEnergySkillData) {
                if (!aiCharacter.hasTag("energyskillfirstround"))
                {
                    CharacterFunc::ApplyEnergyDelta(&aiCharacter, -theEnergySkillData->consumeEn);
                }
                if (theEnergySkillData->castParams.size() > 0)
                    theEnergySkillMaxTick = theEnergySkillData->castParams[0];
            }

            if (!aiCharacter.hasTag("energyskillfirstround")) {
                aiCharacter.currentEnergySkillTick += 1;

                if (aiCharacter.getEnergy() <= 0 || aiCharacter.currentEnergySkillTick > theEnergySkillMaxTick) {
                    aiCharacter.addTag("attacknotinterrupt");
                    HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION, aiCharacter);
                    return;
                }
            }
        } else if (aiCharacter.hasTag("repeatskill")) {
            if (aiCharacter.localTick - aiCharacter.startTick < aiCharacter.totalTick)
                aiCharacter.currentRepeatSkillTick += 1;
            int theReadyTick = HeroSkillController::getRepeatSkillReadyTick(aiCharacter.currentRepeatSkillId);
            if (aiCharacter.currentRepeatSkillTick > aiCharacter.currentRepeatSkillMaxRepeatTick + theReadyTick) {
                if (!isRepeatSkillHasEnding(skillID)) {
                    aiCharacter.addTag("attacknotinterrupt");
                    HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION, aiCharacter);
                } else {
                    aiCharacter.eraseTag("repeatskill");
                    aiCharacter.addTag("repeatskillending");
                    aiCharacter.addTag("repeatskillnotinterruptbeforeending");
                    aiCharacter.startTick = aiCharacter.localTick + 1;
                    aiCharacter.totalTick = getRepeatSkillEndingTick(skillID);
                }
                return;
            }

            if (AIREPEATSKILL_EXTEND_ON_HAS_SKILL == HeroSkillController::getRepeatSkillRepeatType(aiCharacter.currentRepeatSkillId)) {
                int theRepeatBasicTick = HeroSkillController::getRepeatSkillBasicTick(aiCharacter.currentRepeatSkillId);
                if (aiCharacter.currentRepeatSkillTick > theRepeatBasicTick + theReadyTick)
                {
                    if (aiCharacter.hasTag("repeatskillfirstbasicround"))
                        aiCharacter.eraseTag("repeatskillfirstbasicround");
                }
            }
        }

        if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick) {
            // searchTarget type:41
            AISkill* skillData = AIDataHelper::getSkillData(skillID);
            if (skillData && HeroSkillController::isTargetSameNoSummonOption(skillData->targetType, TargetSelfAttackRangeRandomEnemy)) {
                aiCharacter.mAttackConfirmedTarget = false;
            } else if (skillData && HeroSkillController::isTargetSameNoSummonOption(skillData->targetType, TargetOneEnemyRandomContinueHurtNotRandomTarget)) {
                aiCharacter.SetFlagFirstFound26Target(false);
            }

            if (aiCharacter.hasTag("repeatattacking")) {
                int repeatCnt = HeroActionController::getAccuAttackRepeat(aiCharacter.actionAtt);
                repeatCnt -= 2; // fixed bug in lua
                if (repeatCnt > 0) {
                    aiCharacter.startTick = aiCharacter.localTick + 1;
                    aiCharacter.totalTick = HeroSkillController::calculateAccumulateSkillReleaseLength(skillID);
                    HeroActionController::setAccuAttackRepeat(aiCharacter.actionAtt, repeatCnt);
                    return;
                }
            }

            if (aiCharacter.hasTag("energyskill")) {
                aiCharacter.startTick = aiCharacter.localTick + 1;
                if (aiCharacter.hasTag("energyskillfirstround")) {
                    aiCharacter.eraseTag("energyskillfirstround");
                    aiCharacter.totalTick = HeroSkillController::calculateAccumulateSkillReleaseLength(skillID);
                }
                return;
            } else if (aiCharacter.hasTag("repeatskill")) {
                aiCharacter.startTick = aiCharacter.localTick + 1;
                aiCharacter.totalTick = HeroSkillController::calculateAccumulateSkillReleaseLength(skillID);
                if (aiCharacter.hasTag("repeatskillfirstround"))
                    aiCharacter.eraseTag("repeatskillfirstround");
                return;
            }

            aiCharacter.addTag("attacknotinterrupt");
            HeroActionController::actionSelector(aiCharacter, aiContext);
        }
    }

    void HeroAttackAction::ReleaseAccumulate(CharacterData& aiCharacter, AIContext& aiContext)
    {
        HeroSkillController::endAccumulateSkill(aiContext, HeroActionController::getSkillID(aiCharacter.actionAtt),
            HeroActionController::getSkillLv(aiCharacter.actionAtt),
            aiCharacter.localTick - aiCharacter.startTick,
            aiCharacter,
            *aiCharacter.target);
        HeroSkillController::executeExecIDInTick(aiContext, HeroActionController::getSkillID(aiCharacter.actionAtt),
            HeroActionController::getSkillLv(aiCharacter.actionAtt),
            aiCharacter.localTick - aiCharacter.startTick,
            aiCharacter,
            *aiCharacter.target);
    }

    bool HeroAttackAction::ForceEndAccumulate(CharacterData& aiCharacter, AIContext& aiContext)
    {
        return luahelper_attackaction_force_end_accu(aiContext.getLuaState(), &aiCharacter);
    }

    bool HeroAttackAction::ForceEndAccumulateNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        auto curSkid = HeroActionController::getSkillID(aiCharacter.actionAtt);
        auto skdata = AIDataHelper::getSkillData(curSkid);
        if (skdata && eSkillCastConditionAccumulatePlus == skdata->castCondition)
        {
            HeroSkillController::forceReleaseAccumulateSkill(aiContext, curSkid,
                HeroActionController::getSkillLv(aiCharacter.actionAtt),
                aiCharacter.localTick - aiCharacter.startTick, aiCharacter, *aiCharacter.target);
            aiCharacter.totalTick = aiCharacter.localTick - aiCharacter.startTick;
            return false;
        }
        else if (aiCharacter.isInAccumulateState())
        {
            auto todoTick = HeroSkillController::calculateAccumulateSkillReleaseLength(HeroActionController::getSkillID(aiCharacter.actionAtt));
            aiCharacter.totalTick = aiCharacter.localTick - aiCharacter.startTick + todoTick;
            ReleaseAccumulate(aiCharacter, aiContext);
            return true;
        }
        else if (aiCharacter.isAfterAccumulateState())
        {
            aiCharacter.exitAccumulateState();
        }
        return false;
    }

    void HeroAttackAction::Enter(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill)
    {
        if (AIScriptBinder::getInstance()->isAttackActionEnterByLua(aiContext.getLuaState()))
            luahelper_attackaction_enter(aiContext.getLuaState(), &aiCharacter, skillSeq, skillID, skillLevel, resetCD, tickOffset, isUltraSkill);
        else
            EnterNative(aiCharacter, aiContext, skillSeq, skillID, skillLevel, resetCD, tickOffset, isUltraSkill);
    }

	void HeroAttackAction::EnterNative(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill)
	{
        /*
		if (aiContext.comboConstraints.find(skillID) == aiContext.comboConstraints.end())
		{
            std::unordered_map<int, ComboTrigger*>& mapTrigger = AIDataHelper::checkComboTriggerMap();
            std::unordered_map<int, ComboTrigger*>::iterator iter = mapTrigger.find(skillID);
            if (iter != mapTrigger.end())
			{
				OutPutFileLine;
				if (aiContext.localRandom->nextInt(0, 100) < iter->second->rate)
				{
					int count = iter->second->team.size();
					int hitNum = count;
					for (std::list<CharacterData*>::iterator friendIter = aiCharacter.getCurrentFriendsList()->begin(); friendIter != aiCharacter.getCurrentFriendsList()->end(); friendIter++)
					{
                        
                        if (!HeroActionController::isDeadAction((*friendIter)->currentAction))
						{
							for (int i = 0; i < count; i++)
							{
								if ((*friendIter)->getID() == iter->second->team.at(i) && --hitNum == 0)
								{
									aiContext.comboConstraints[skillID] = 1;
									HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
									HeroIdleAction::Enter(aiCharacter, aiContext, aiCharacter.getAInterval(), IDLE_ACTION, aiCharacter);
									if (aiContext.comboProxy != nullptr)
									{
										aiContext.comboProxy->executeExec(skillID, aiCharacter.target->myID, aiCharacter.target->getID(), aiCharacter.target->getX(), aiCharacter.target->getY(), iter->second->totalPeriod);
									}
									if (aiContext.aiCallback != nullptr)
									{
										aiContext.aiCallback->stopSchedule(skillID, aiCharacter.target->getX(), aiCharacter.target->getY(), aiCharacter.target->myID);
									}
                                    AIActionController::executeExec(aiContext, iter->second->hurtID, skillID, skillLevel, &aiCharacter, nullptr, aiCharacter.target, 0);
									return;
								}
							}
						}
					}
				}
			}
        }
        */
        if (TriggerComboNative(aiCharacter, aiContext, skillID, skillLevel))
            return;

        ImplEnter(aiCharacter, aiContext, skillSeq, skillID, skillLevel, resetCD, tickOffset, isUltraSkill, false);
	}

    void HeroAttackAction::ForceEnter(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill)
    {
        ForceEnterNative(aiCharacter, aiContext, skillSeq, skillID, skillLevel, resetCD, tickOffset, isUltraSkill);
    }

	// clair_wang start
	void HeroAttackAction::ForceEnterNative(CharacterData&aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, unsigned short tickOffset, bool isUltraSkill)
	{
        /*
        if (aiCharacter.currentAction == ATTACK_ACTION)
        {
            if (aiCharacter.hasTag("attacknobrokenwhenenter"))
                return;
        }

        HeroActionController::exitAction(aiCharacter, aiContext);
        HeroActionController::cleanup(aiCharacter, aiContext);

		HeroActionController::setOldAction(aiCharacter.actionAtt, aiCharacter.currentAction);
		HeroActionController::setOldTotalTick(aiCharacter.actionAtt, aiCharacter.totalTick);
		HeroActionController::setOldTick(aiCharacter.actionAtt, aiCharacter.localTick);
		aiCharacter.currentAction = ATTACK_ACTION;

		HeroActionController::setSkillID(aiCharacter.actionAtt, skillID);
		HeroActionController::setSkillLv(aiCharacter.actionAtt, skillLevel);

		HeroActionController::setYingzhi(aiCharacter.actionAtt, 0);

		aiCharacter.startTick = aiCharacter.localTick + 1;
		aiCharacter.totalTick = HeroSkillController::calculateSkillLength(HeroActionController::getSkillID(aiCharacter.actionAtt));
        int speedUp = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_CHANGE_ATTACK_SPEED, 100);
        if (speedUp)
        {
            aiCharacter.attackSpeed += speedUp;
            if (aiCharacter.attackSpeed > 0)
            {
                int extendedTick = aiCharacter.totalTick * aiCharacter.attackSpeed / 100;
                if (aiCharacter.totalTick > 0 && extendedTick <= 0)
                    aiCharacter.totalTick = 1;
                else
                    aiCharacter.totalTick = extendedTick;
            }
        }

        if (isUltraSkill)
            aiCharacter.addTag("ultraskill");

        AISkill* skillData = AIDataHelper::getSkillData(skillID);
        if (!skillData->interruptByUltra)
            aiCharacter.addTag("nointerruptbyultra");

        if (!skillData->canBrokenWhenEnter)
            aiCharacter.addTag("attacknobrokenwhenenter");

		AIContinuousMarkCaster::checkExcuteEffect(aiContext, aiCharacter, skillID, skillLevel, -1, -1, AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallBySkill);

		aiCharacter.onStartCastSkill(skillID);

        HeroActionController::didEnterAction(aiCharacter, aiContext);
        */

        ImplEnter(aiCharacter, aiContext, skillSeq, skillID, skillLevel, resetCD, tickOffset, isUltraSkill, true);
	}
    // clair_wang end
    
    bool HeroAttackAction::TriggerComboNative(CharacterData&aiCharacter, AIContext& aiContext, int skillID, int skillLevel)
    {
        if (aiContext.comboConstraints.find(skillID) == aiContext.comboConstraints.end())
		{
            std::unordered_map<int, ComboTrigger*>& mapTrigger = AIDataHelper::checkComboTriggerMap();
            std::unordered_map<int, ComboTrigger*>::iterator iter = mapTrigger.find(skillID);
            if (iter != mapTrigger.end())
			{
				OutPutFileLine;
				if (aiContext.localRandom->nextInt(0, 100) < iter->second->rate)
				{
					int count = iter->second->team.size();
					int hitNum = count;
					for (std::list<CharacterData*>::iterator friendIter = aiCharacter.getCurrentFriendsList()->begin(); friendIter != aiCharacter.getCurrentFriendsList()->end(); friendIter++)
					{
                        
                        if (!HeroActionController::isDeadAction((*friendIter)->currentAction))
						{
							for (int i = 0; i < count; i++)
							{
								if ((*friendIter)->getID() == iter->second->team.at(i) && --hitNum == 0)
								{
									aiContext.comboConstraints[skillID] = 1;
									HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
									HeroIdleAction::Enter(aiCharacter, aiContext, aiCharacter.getAInterval(), IDLE_ACTION, aiCharacter);
									if (aiContext.comboProxy != nullptr)
									{
										aiContext.comboProxy->executeExec(skillID, aiCharacter.target->myID, aiCharacter.target->getID(), aiCharacter.target->getX(), aiCharacter.target->getY(), iter->second->totalPeriod);
									}
									if (aiContext.aiCallback != nullptr)
									{
										aiContext.aiCallback->stopSchedule(skillID, aiCharacter.target->getX(), aiCharacter.target->getY(), aiCharacter.target->myID);
									}
                                    AIActionController::executeExec(aiContext, iter->second->hurtID, skillID, skillLevel, &aiCharacter, nullptr, aiCharacter.target, 0);
									return true;
								}
							}
						}
					}
				}
			}
        }
        
        return false;
    }

    void HeroAttackAction::ImplEnter(CharacterData& aiCharacter, AIContext& aiContext, int skillSeq, int skillID, int skillLevel, bool resetCD, int tickOffset, bool isUltraSkill, bool isForceEnter)
    {
        if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
        {
            if (aiCharacter.hasTag("attacknobrokenwhenenter"))
                return;

            if (!isForceEnter)
            {
                if (isUltraSkill && aiCharacter.hasTag("nointerruptbyultra"))
                    return;

                HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
                if (!isUltraSkill && !aiCharacter.hasTag("nocdattack"))
                {
                    return;
                }
            }
            else
            {
                aiCharacter.addTag("exitattackbyforceattack");
            }
        }

        if (!AIContext::getInstance()->isFighting)
        {
            AIContext::getInstance()->isFighting = true;
        }

        HeroActionController::exitAction(aiCharacter, aiContext);
        HeroActionController::cleanup(aiCharacter, aiContext);

        if (isForceEnter)
        {
            HeroActionController::setOldAction(aiCharacter.actionAtt, aiCharacter.currentAction);
            HeroActionController::setOldTotalTick(aiCharacter.actionAtt, aiCharacter.totalTick);
            HeroActionController::setOldTick(aiCharacter.actionAtt, aiCharacter.localTick);
        }
        aiCharacter.currentAction = ATTACK_ACTION;
        aiCharacter.actionCauserMyID = aiCharacter.myID;

        if (skillSeq > -1 && !isForceEnter)
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_OTHER_SKLIST, 0) == 1)
                aiCharacter.m_iSkillSeq2Index = skillSeq;
            else
                HeroActionController::setSkillSeq(aiCharacter.actionAtt, skillSeq);
        }

        HeroActionController::setSkillID(aiCharacter.actionAtt, skillID);
        HeroActionController::setSkillLv(aiCharacter.actionAtt, skillLevel);

        HeroActionController::setYingzhi(aiCharacter.actionAtt, 0);

        AISkill* skillData = AIDataHelper::getSkillData(skillID);
        if (!skillData)
        {
            HeroIdleAction::Enter(aiCharacter, aiContext, 1, ActionType::IDLE_ACTION, aiCharacter);
            return;
        }
        aiCharacter.startTick = aiCharacter.localTick + 1 - tickOffset;
        aiCharacter.totalTick = HeroSkillController::calculateSkillLength(skillID);
        if (IsAccumulateSkill(skillData->castCondition))
        {
            int theNextAltAccuTick = aiCharacter.getNextAlternateAccuMaxTick(skillID);
            if (theNextAltAccuTick >= 0)
            {
                aiCharacter.setCurrentAlternateAccuMaxTick(skillID, theNextAltAccuTick);
                aiCharacter.totalTick = HeroSkillController::calculateAccuSkillLength(skillID, theNextAltAccuTick);
            }
            else
            {
                if (aiCharacter.getCurrentAlternateAccuMaxTick(skillID) >= 0)
                    aiCharacter.setCurrentAlternateAccuMaxTick(skillID, -1);
            }
        }
        else if (IsRepeatSkill(skillData->castCondition))
        {
            aiCharacter.totalTick = HeroSkillController::calculateRepeatSkillFirstRoundTickLength(skillID);
            aiCharacter.currentRepeatSkillMaxRepeatTick = HeroSkillController::getRepeatSkillMaxRepeatTick(&aiCharacter, skillID);
            if (AIREPEATSKILL_EXTEND_ON_CRIT == HeroSkillController::getRepeatSkillRepeatType(skillID))
            {
                aiCharacter.addTag("extendrepeatoncrit");
                aiCharacter.addTag("repeatskillnointerrupt");
            }
        }
        else if (IsEnergySkill(skillData->castCondition))
        {
            aiCharacter.totalTick = HeroSkillController::calculateEnergySkillFirstRoundTickLength(skillID);
        }

        int speedUp = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_CHANGE_ATTACK_SPEED, 100);
        if (speedUp)
        {
            aiCharacter.attackSpeed += speedUp;
            if (aiCharacter.attackSpeed > 0)
            {
                int extendedTick = aiCharacter.totalTick * aiCharacter.attackSpeed / 100;
                if (aiCharacter.totalTick > 0 && extendedTick <= 0)
                    aiCharacter.totalTick = 1;
                else
                    aiCharacter.totalTick = extendedTick;
            }
        }

        if (isUltraSkill)
            aiCharacter.addTag("ultraskill");

        if (!skillData->interruptByUltra)
            aiCharacter.addTag("nointerruptbyultra");

        if (!skillData->canBrokenWhenEnter)
            aiCharacter.addTag("attacknobrokenwhenenter");
        
        if (IsEnergySkill(skillData->castCondition))
        {
            aiCharacter.currentEnergySkillId = skillID;
            aiCharacter.currentEnergySkillTick = 0;
            aiCharacter.addTag("energyskill");
            aiCharacter.addTag("energyskillfirstround");
        }
        else if (skillData->castCondition == eSkillCastConditionRepeatSkill)
        {
            aiCharacter.currentRepeatSkillId = skillID;
            aiCharacter.currentRepeatSkillTick = 0;
            aiCharacter.addTag("repeatskill");
            aiCharacter.addTag("repeatskillfirstround");
            aiCharacter.addTag("repeatskillfirstbasicround");

            aiContext.addEventListener("castultra", &aiCharacter);
        }

		aiCharacter.attackActionInfo.skillid = skillID;
		aiCharacter.attackActionInfo.skilllv = skillLevel;
		if (skillID == aiCharacter.normalSkill.first)
			aiCharacter.attackActionInfo.skilltype = AttackActionInfo::SKTYPE_NORMAL;
		else if (isUltraSkill)
			aiCharacter.attackActionInfo.skilltype = AttackActionInfo::SKTYPE_ULTRA;
		else
			aiCharacter.attackActionInfo.skilltype = AttackActionInfo::SKTYPE_OTHER;
		aiCharacter.attackActionInfo.isforce = isForceEnter;

        AIContinuousMarkCaster::checkExcuteEffect(aiContext, aiCharacter, skillID, skillLevel, -1, -1, AIContinuousMarkCaster::E_CheckExcuteEffectMarkCallType::kCheckMarkCallBySkill);

		if (aiCharacter.hasEffect(328))
			luahelper_innereffect_328_trigger(aiContext.getLuaState(), &aiCharacter, skillID, skillLevel, aiCharacter.attackActionInfo.skilltype, isForceEnter, 0);

        aiCharacter.onStartCastSkill(skillID);

        if (HeroSkillController::isTargetSameNoSummonOption(skillData->targetType, TargetOneEnemyRandomContinueHurtNotRandomTarget) && !isForceEnter)
        {
            aiCharacter.SetFlagFirstFound26Target(true);
            aiCharacter.SetFirstRoundConfirm26Target();
        }

        HeroActionController::didEnterAction(aiCharacter, aiContext);
    }

    void HeroAttackAction::Exit(CharacterData&aiCharacter, AIContext& aiContext)
    {
        luahelper_attackaction_on_exit(aiContext.getLuaState(), &aiCharacter);
    }

    void HeroAttackAction::ExitNative(CharacterData&aiCharacter, AIContext& aiContext)
    {
        //if (HeroActionController::getSkillID(aiCharacter.actionAtt) == aiCharacter.normalSkill.first && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, eEFFECT_FORCE_ATTACK_SELF, 0) > 0)
        //{
        //    AIContinuousEffectCaster::change_Special_Effect(aiContext, aiCharacter, eEFFECT_FORCE_ATTACK_SELF, 1);
        //}
        if (aiCharacter.hasTag("energyskill") && aiCharacter.currentEnergySkillId > 0)
        {
            int theEnergySkillLevel = aiCharacter.getSkillLevel(aiCharacter.currentEnergySkillId);
            HeroSkillController::executeExecOnExitEnergySkill(aiContext, aiCharacter.currentEnergySkillId, theEnergySkillLevel, aiCharacter, aiCharacter);
        }
        aiContext.removeEventListener("castultra", &aiCharacter); // 10.0 add for repeat skill only
        aiCharacter.attackSpeed = 100;
        aiCharacter.eraseTag("attacknobrokenwhenenter");
        aiCharacter.eraseTag("nocdattack");
        aiCharacter.eraseTag("nointerruptbyultra");
        aiCharacter.eraseTag("repeatattacking");
        aiCharacter.eraseTag("energyskill");
        aiCharacter.eraseTag("energyskillfirstround");
        aiCharacter.eraseTag("repeatskill");
        aiCharacter.eraseTag("repeatskillfirstround");
        aiCharacter.eraseTag("repeatskillnointerrupt");
        aiCharacter.eraseTag("extendrepeatoncrit");
		aiCharacter.eraseTag("besuppressed");
        aiCharacter.currentEnergySkillId = 0;
        aiCharacter.currentEnergySkillTick = 0;
        aiCharacter.currentRepeatSkillId = 0;
        aiCharacter.currentRepeatSkillTick = 0;
        aiCharacter.currentRepeatSkillMaxRepeatTick = 0;
        aiCharacter.exitAccumulateState();
        if (!aiCharacter.hasTag("attacknotinterrupt") && aiCharacter.hasTag("ultraskill"))
        {
            if (AIDataHelper::getAiUltraBrokenMode() == AIULTRABROKEN_STOP_ALL || AIDataHelper::getAiUltraBrokenMode() == AIULTRABROKEN_STOP_FREEZE)
            {
                aiContext.stopUltraFreeze(aiCharacter.myID);
            }
            
            if (aiContext.sexecProxy)
            {
                if (AIDataHelper::getAiUltraBrokenMode() == AIULTRABROKEN_STOP_ALL || AIDataHelper::getAiUltraBrokenMode() == AIULTRABROKEN_STOP_SCREEN_EFFECT)
                {
                    aiContext.sexecProxy->stopScreenExec(aiContext, &aiCharacter);
                }
            }
        }
        aiCharacter.eraseTag("attacknotinterrupt");
        aiCharacter.eraseTag("ultraskill");
        HeroActionController::setAccuAttackRepeat(aiCharacter.actionAtt, 0);
		aiCharacter.attackActionInfo.skillid = 0;
    }
}

