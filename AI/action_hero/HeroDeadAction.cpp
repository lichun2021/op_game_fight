//
//  HeroDeadAction.cpp
//  OnePiece
//
//  Created by Beileng Shan on 3/17/14.
//
//

#include "../AIContext.h"
#include "HeroDeadAction.h"
#include "HeroActionController.h"
#include "HeroIdleAction.h"
#include "HeroAttackAction.h"
#include "../CommonType.h"
#include "HeroSkillController.h"
#include "../AISpecialEffectLabel.h"
#include "../scriptbind/AIScriptBindActionController.hpp"
#include "../AIScriptBinder.hpp"

#include "cocos2d.h"

namespace OnePiece {
	void HeroDeadAction::Excute(CharacterData& aiCharacter, AIContext& aiContext)
	{
        //if (aiContext.winlose == 0)
        //{
        //    if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_RESTORE, 0) > 0)
        //    {
        //        if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick) {
        //            aiCharacter.currentAction = IDLE_ACTION;
        //            aiCharacter.updateHP(AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_RESTORE, 0));
        //            AIContinuousEffectCaster::zero_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_RESTORE);
        //        }
        //    }
        //}

        // HQ: 8.0新复活效果
        if (aiCharacter.isSummon())
            return;
        int reviveHP = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_REVIVE, aiCharacter.getHPTop());
        if (reviveHP > 0)
        {
            if (aiCharacter.localTick > aiCharacter.startTick + aiCharacter.totalTick)
            {
                aiCharacter.updateHP(reviveHP);
                EffectHelper::checkActivateReviveEffect(aiContext, &aiCharacter);
                AIContinuousEffectCaster::zero_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_REVIVE);

                HeroOutofControlAction::Enter(aiCharacter, aiContext, 0, ActionType::IDLE_ACTION, aiCharacter);

                if (aiContext.animationProxy)
                    aiContext.animationProxy->reviveCharacter(aiContext, &aiCharacter);

                AdjustTeamPos(aiCharacter, aiContext, true);
                aiCharacter.addTag("revived");
                
                /// 12.0
                if (aiContext.animationProxy && AIScriptBinder::getInstance()->isReshowUIEffectsWhenRevived(aiContext.getLuaState()))
                    EffectHelper::reshowUIEffectsWhenCharacterRevive(aiContext, &aiCharacter);

                if (aiContext.markProxy && AIScriptBinder::getInstance()->isReshowMarkIconsWhenRevived(aiContext.getLuaState()))
                    AIContinuousMarkCaster::reshowUIEffects(aiContext, aiCharacter);
                ///
            }
        }
    }
    
    void HeroDeadAction::Enter(CharacterData& aiCharacter, AIContext& aiContext, short deadType, bool isFatal)
    {
        luahelper_actioncontroller_enter_deadaction(aiContext.getLuaState(), &aiCharacter, deadType, isFatal);
    }

    void HeroDeadAction::EnterNative(CharacterData& aiCharacter, AIContext& aiContext, short deadType, bool isFatal)
    {
		if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
		{
            if (aiCharacter.hasTag("deadattacking"))
                return;

            if (aiCharacter.isInAccumulateState())
            {
                auto curSkid = HeroActionController::getSkillID(aiCharacter.actionAtt);
                auto skdata = AIDataHelper::getSkillData(curSkid);
                if (skdata && eSkillCastConditionAccumulatePlus == skdata->castCondition)
                    HeroAttackAction::ForceEndAccumulate(aiCharacter, aiContext);
            }

			HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
			//CCLOG("DDDDDDDD:%d--%d--%d--%d", aiCharacter.myID, aiCharacter.attributes[0], aiCharacter.actionAtt[1], aiCharacter.localTick);
		}

        if (HeroActionController::isDeadAction(aiCharacter.currentAction))
			return;

        //if (ActionType::DEATH_TRIGGERED_ATTACK_ACTION == aiCharacter.currentAction)
        //    return;

        if (!AIAttackIgnoreNoDieEffect::checkExcute(aiContext, &aiCharacter))
		{
			//check if has restore effect
			if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_NO_DIE, 0) > 0)
				return;
		}

        auto pDeadSkill = aiCharacter.getSpecialSkill(ESkillCastCondition::eSkillCastConditionOnDeath);
        if (pDeadSkill && pDeadSkill->canCast(&aiCharacter))
        {
            aiCharacter.addTag("deadattacking");
            HeroAttackAction::ForceEnter(aiCharacter, aiContext, -1, pDeadSkill->skillID, pDeadSkill->skillLevel, true, 0, pDeadSkill->skillID == aiCharacter.staticData->sk[1]);
            //aiCharacter.addTag("deadattacking");
            return;
        }
		
        HeroActionController::exitAction(aiCharacter, aiContext);
		HeroActionController::cleanup(aiCharacter, aiContext);
		aiCharacter.currentAction = deadType == 1 ? DEAD_ACTION1 : deadType == 2 ? DEAD_ACTION2 : DEAD_ACTION3;
        aiCharacter.actionCauserMyID = aiCharacter.killerUID;
		aiCharacter.updateHP(0);
		aiCharacter.startTick = aiCharacter.localTick + 1;
		aiCharacter.totalTick = 40;

        aiCharacter.exitDuel();
		
		//if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, aiCharacter, EffectType::eEFFECT_RESTORE, 0)>0) return;

        AdjustTeamPos(aiCharacter, aiContext, false);
		
		if (CharacterFunc::IsEnemyHero(aiCharacter) && !aiCharacter.hasTag("revived"))
		{
            ClaimMonsterDeadReward(aiCharacter, aiContext);
		}

        if (CharacterFunc::IsOurHero(aiCharacter) && !aiCharacter.hasTag("revived"))
        {
            DoHeroDead(aiCharacter, aiContext);
        }

        //aiCharacter.marksOnSelfEnterDead();

        //// 新增标记执行释放条件：达到层数，有人死亡触发。
        //aiContext.onMarkReleaseFun();

        //// 新增标记   标记施法者死亡后清除 clair add by 11.0 version
        //aiContext.onCharacterDeadIfMarkClearFun(&aiCharacter);

        // OP2优化
        AIContinuousMarkCaster::checkDoCharacterDead(aiContext, &aiCharacter);

        if (aiCharacter.isSummon()) {
            aiCharacter.addTag("summonkilled");
            AIContinuousSummonCaster::checkExcuteDeadRun(aiContext, &aiCharacter, 1);
            AIContinuousSummonCaster::checkExcuteDeadRun(aiContext, &aiCharacter, 3);
            AIContinuousSummonCaster::checkExcuteDeadRun(aiContext, &aiCharacter, 4);
        }
        else
        { // not summon
            auto pKiller = aiContext.getCharacterByID(aiCharacter.killerUID);
            if (pKiller && pKiller->myID != aiCharacter.myID && pKiller->teamSide != aiCharacter.teamSide)
            {
                int energy = pKiller->getEnergy();
                CharacterFunc::ApplyEnergyDelta(pKiller, AIDataHelper::getEnergyRecover()[1]);
                int energydelta = pKiller->getEnergy() - energy;

				// clair_wang  添加击杀奖励 文字显示
				AISpecialEffectLabel::excute(100040, energydelta, aiContext, pKiller);

            #if BATTLE_LOG_TEST
                OPBattleLog("gainenergy", pKiller->teamSide, pKiller->clusterNum, pKiller->attributes[0], (energy - pKiller->getEnergy()));
            #endif
            }
        } // END if isSummon

        aiContext.onEvent("herodead250", &aiCharacter);

        auto theFriends = aiCharacter.getCurrentFriendsList();
        for (auto aFriend : *theFriends)
        {
            aFriend->onFriendDead(&aiCharacter);
        }

        HeroActionController::didEnterAction(aiCharacter, aiContext);
    } // END Enter
    
    void HeroDeadAction::ForceEnter(CharacterData& aiCharacter, AIContext& aiContext)
    {
        luahelper_actioncontroller_force_enter_deadaction(aiContext.getLuaState(), &aiCharacter);
    }

    void HeroDeadAction::ForceEnterNative(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiCharacter.currentAction == ActionType::ATTACK_ACTION)
        {
            HeroActionController::setSkillActionCD(aiCharacter.actionAtt, aiCharacter.getAInterval());
        }

        if (HeroActionController::isDeadAction(aiCharacter.currentAction))
            return;

        HeroActionController::exitAction(aiCharacter, aiContext);
        HeroActionController::cleanup(aiCharacter, aiContext);
        aiCharacter.currentAction = DEAD_ACTION1;
        aiCharacter.actionCauserMyID = aiCharacter.killerUID;
        aiCharacter.updateHP(0);
        aiCharacter.startTick = aiCharacter.localTick + 1;
        aiCharacter.totalTick = 40;

        aiCharacter.exitDuel();

        if (!aiCharacter.isSummon())
        {
            AdjustTeamPos(aiCharacter, aiContext, false);
        }

        if (CharacterFunc::IsEnemyHero(aiCharacter) && !aiCharacter.hasTag("revived"))
        {
            ClaimMonsterDeadReward(aiCharacter, aiContext);
        }

        if (CharacterFunc::IsOurHero(aiCharacter) && !aiCharacter.hasTag("revived"))
        {
            DoHeroDead(aiCharacter, aiContext);
        }

        // 新增标记执行释放条件：达到层数，有人死亡触发。
        //aiContext.onMarkReleaseFun();
        AIContinuousMarkCaster::checkTriggerOnCharacterDead(aiContext, &aiCharacter);

        if (aiCharacter.isSummon())
        {
            aiCharacter.addTag("summonkilled");
        }

        aiContext.onEvent("herodead250", &aiCharacter);

        auto theFriends = aiCharacter.getCurrentFriendsList();
        for (auto aFriend : *theFriends)
        {
            aFriend->onFriendDead(&aiCharacter);
        }

        HeroActionController::didEnterAction(aiCharacter, aiContext);
    }

    void HeroDeadAction::AdjustTeamPos(CharacterData& aiCharacter, AIContext& aiContext, bool isRevive)
    {
        // 幻象应该排除在站位相关的计算, 防止影响大招释放或其他站位相关的逻辑
        int line = aiCharacter.staticData->confg.line;
        int* THE_NUM = CharacterFunc::IsEnemyHero(aiCharacter) ? aiContext.rival_num : aiContext.friend_num;
        if (!aiCharacter.isSummon())
        {
            if (isRevive)
            {
                THE_NUM[line]++;
            }
            else
            {
                THE_NUM[line]--;
                if (THE_NUM[line] < 0)
                    THE_NUM[line] = 0;
            }
        }
        //bool isEven = (THE_NUM[line] == ((int)(THE_NUM[line] / 2)) * 2);
        //int* postionArray = AIDataHelper::getPositionArray(isEven);
        //int middleY = (AIContext::getInstance()->deltaMinY + AIContext::getInstance()->deltaMaxY) / 2;

        //adjust Y position
        if (AIContext::getInstance()->isFighting && !aiCharacter.isSummon() && AIContext::getInstance()->businessType != BusinessType::CANNON_KING)
        {
            bool isEven = (THE_NUM[line] == ((int)(THE_NUM[line] / 2)) * 2);
            int* postionArray = AIDataHelper::getPositionArray(isEven);
            int middleY = (AIContext::getInstance()->deltaMinY + AIContext::getInstance()->deltaMaxY) / 2;
            int startIndex = 0;
            if (isEven) startIndex = (4 - THE_NUM[line]) / 2;
            else startIndex = (5 - THE_NUM[line]) / 2;
            if (startIndex >= 5)
                startIndex = 4;
            if (startIndex < 0)
                startIndex = 0;

            //for (std::list<CharacterData*>::iterator iter = aiCharacter.getCurrentFriendsList()->begin(); iter != aiCharacter.getCurrentFriendsList()->end(); iter++)
            //{
            //    if (!HeroActionController::isDeadAction((*iter)->currentAction))
            //    {
            //        if (line == (*iter)->staticData->confg.line)
            //        {
            //            (*iter)->needAdjusting = true;
            //            (*iter)->adjustingY = postionArray[startIndex++] + middleY;
            //            (*iter)->adjustingX = 100000000;
            //        }
            //    }
            //}

            // 幻象应该排除在站位相关的计算, 防止影响大招释放或其他站位相关的逻辑;
            // 被魅惑角色死亡时站位应该算到原来队伍的站位
            auto originCluster = aiContext.getClusterOrigin(aiCharacter.teamSide, aiCharacter.clusterNum);
            for (auto iter = originCluster->begin(); iter != originCluster->end(); ++iter)
            {
                auto hero = *iter;
                if (HeroActionController::isDeadAction(hero->currentAction) || hero->isSummon() || line != hero->staticData->confg.line)
                    continue;

                hero->needAdjusting = true;
                hero->adjustingY = postionArray[startIndex] + middleY;
                hero->adjustingX = 100000000;

                startIndex++;
                if (startIndex >= 5)
                    startIndex = 4;
            }
        }
    }

    void HeroDeadAction::ClaimMonsterDeadReward(CharacterData& aiCharacter, AIContext& aiContext)
    {
        char buff[10];
        std::vector<std::pair<int, int>>* rewardArray = nullptr;
        sprintf(buff, "%d-%d", aiCharacter.clusterNum, aiCharacter.myID);
        std::vector<RewardCandidateItem*>* rewards = nullptr;// reward_candidates.find(buff)->second;
        auto iter = aiContext.reward_candidates.find(buff);
        if (iter != aiContext.reward_candidates.end()) rewards = iter->second;
        if (rewards != nullptr) rewardArray = aiContext.getPveGuanqiaReward(rewards);
        if (aiContext.aiCallback)
            aiContext.aiCallback->monsterDeadReward(aiCharacter.myID, aiCharacter.getX(), aiCharacter.getY(), aiCharacter.getZ(), rewardArray, true, aiContext.getCharacterWaveNum(aiCharacter.myID));
        if (rewardArray != nullptr)
            delete rewardArray;
    }

    void HeroDeadAction::DoHeroDead(CharacterData& aiCharacter, AIContext& aiContext)
    {
        if (aiContext.aiCallback)
            aiContext.aiCallback->onHeroDead(aiCharacter.attributes[0], aiCharacter.getX(), aiCharacter.getY(), aiCharacter.getZ());
    }

} // namespace OnePiece

