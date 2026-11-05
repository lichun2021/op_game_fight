//
//  AIContinuousEffectCaster.cpp
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#include "AIContinuousEffectCaster.h"
#include <algorithm>
#include "AIContext.h"
#include "HeroHurtcalculate.h"
#include "projectile/ProjectileForCharacter.h"
#include "projectile/ProjectileForLanding.h"
#include "action_hero/HeroEffectAction.h"
#include "action_hero/HeroIdleAction.h"
#include "action_hero/HeroActionController.h"
#include "AISpecialEffectLabel.h"
#include "action_hero/HeroDeadAction.h"
#include "action_hero/HeroStruckHoverAction.h"
#include "action_hero/HeroAttackAction.h"
#include "action_hero/HeroSkillController.h"
#include "AIBattleStats.h"
#include "BattleLog.h"
#include "AIScriptBinder.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "AITargetHelper.h"

namespace OnePiece{

    //namespace {
    //    enum EffectHitLabel {
    //        MAX_HP_INCREASE = 105023,
    //        MAX_HP_DECREASE = 105024
    //    };
    //}
#define TAG_FOLLOW_SKILLTICK_EFFECT_CNT "foskeffcnt"
    
    static void initEffectCasterSearchContext(AITargetContext &ctx, CharacterData *s, CharacterData *t, CharacterData *sc, int skillId, int skillLevel, EffectExec *effectData, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter)
    {
        ctx.source = s;
        ctx.target = t;
        ctx.skill_caster = sc;
        ctx.projectile = nullptr;
        ctx.skill_id = skillId;
        ctx.skill_level = skillLevel;
        ctx.hurt_id = 0;
        ctx.exclude_character = excludeCharacter;
        ctx.should_hp_ratio_higher_than_exclude_character = shouldHpRatioHigherThanExcludeCharacter;
		if (shouldHpRatioHigherThanExcludeCharacter)
			ctx.lowerhplimit_character = excludeCharacter;
        
        ctx.search_type = effectData->target;
        ctx.range.x1 = effectData->range[0];
        ctx.range.x2 = effectData->range[1];
        ctx.range.y1 = effectData->range[2];
        ctx.range.y2 = effectData->range[3];
        ctx.search_params = &effectData->targetParams;
    }
    
    static void initEffectCasterSearchContext(AITargetContext &ctx, AIContinuousEffectCaster *caster)
    {
        ctx.source = caster->target;
        ctx.target = caster->target->target;
        ctx.skill_caster = caster->origin;
        ctx.projectile = nullptr;
        ctx.skill_id = caster->skillID;
        ctx.skill_level = caster->skillLevel;
        
        auto caster_data = caster->staticData;
        ctx.search_type = caster_data->target;
        ctx.range.x1 = caster_data->range[0];
        ctx.range.x2 = caster_data->range[1];
        ctx.range.y1 = caster_data->range[2];
        ctx.range.y2 = caster_data->range[3];
        ctx.search_params = &caster_data->targetParams;
    }
    
    //static void initEffectCasterSearchExcludeContext(AITargetContext &ctx, AIContinuousEffectCaster *caster)
    //{
    //    ctx.source = caster->target;
    //    ctx.target = caster->target->target;
    //    ctx.skill_caster = caster->origin;
    //    ctx.projectile = nullptr;
    //    ctx.skill_id = caster->skillID;
    //    ctx.skill_level = caster->skillLevel;
    //    
    //    auto caster_data = caster->staticData;
    //    ctx.search_type = caster_data->targetExclude;
    //    ctx.range.x1 = caster_data->excludeX1;
    //    ctx.range.x2 = caster_data->excludeX2;
    //    ctx.range.y1 = caster_data->excludeZ1;
    //    ctx.range.y2 = caster_data->excludeZ2;
    //    ctx.search_params = &caster_data->targetExcludeParams;
    //}

	AIContinuousEffectCaster::AIContinuousEffectCaster(AIContext& aiContext, int effID, int skID, int skLv, CharacterData* origin, AIProjectile* projectile, CharacterData* target, int theMarkID)
		: aiContext(aiContext), effectID(effID), skillID(skID), skillLevel(skLv), markID(theMarkID), AINode(target->myID, target->clusterNum)
	{
		myID = aiContext.local_incremential++;

		staticData = AIDataHelper::getEffectExec(effectID);
		firstRound = true;

		this->projectile = projectile;
		this->origin = origin;
		this->target = target;

		localTick = aiContext.currentTick;
		startTick = localTick;//from 0

        // 12.0
        if (staticData->followSkillTick == 1 && target->currentAction == ATTACK_ACTION)
        {
            totalTick = 3600;
        }
        else
        {
            totalTick = staticData->period + staticData->recurrenceGrowTimeValue * skillLevel;
            totalTick += EffectHelper::getExtendBuffTick(target, staticData->isBuff, totalTick);
            if (origin->attackSpeed != 100 && origin->attackSpeed > 0 && origin->myID == target->myID && AIScriptBinder::getInstance()->isCasterSyncAttackSpeed(aiContext.getLuaState(), effID))
            {
                totalTick = totalTick * origin->attackSpeed / 100;
            }
        }

        for (int i = 0; i < staticData->execs.size(); i++)
        {
            auto& innerExec = staticData->execs[i];
            if (innerExec.ID == EffectType::eEFFECT_HP_LOST && innerExec.params.size()) {
                totalTick += innerExec.params[0];
                break;
            }
        }
        
        if (staticData->followSkillTick == 1)
            target->setTagValue(TAG_FOLLOW_SKILLTICK_EFFECT_CNT, target->getTagValue(TAG_FOLLOW_SKILLTICK_EFFECT_CNT) + 1);
		
		stopFurtherEffect = false;
        isInvalidated = false;
        invalidateTick = 0;
        
        ignoreFreeze = target->isCaptain();

		target->effectCasts.push_back(this);
	}

	AIContinuousEffectCaster::~AIContinuousEffectCaster()
	{
        cleanupInnerEffect();

		for (std::list<AIContinuousEffectCaster*>::iterator iter = target->effectCasts.begin(); iter != target->effectCasts.end();)
		{
			if (*iter == this)
			{
				target->effectCasts.erase(iter);
				break;
			}
			iter++;
		}

        for (auto iter = allEffs.begin(); iter != allEffs.end(); ++iter)
        {
            delete *iter;
        }
        
        if (staticData->followSkillTick == 1)
        {
            target->setTagValue(TAG_FOLLOW_SKILLTICK_EFFECT_CNT, target->getTagValue(TAG_FOLLOW_SKILLTICK_EFFECT_CNT) - 1);
        }
	}

    void AIContinuousEffectCaster::removeInnerEffRecord(int characterMyID, int innerExecID)
    {
        auto iter = innerEffs.find(innerExecID);
        if (iter == innerEffs.end() || iter->second.count(characterMyID) <= 0)
            return;
        iter->second.erase(characterMyID);
        if (iter->second.size() == 0)
            innerEffs.erase(iter);
    }

    //void AIContinuousEffectCaster::scheduleInnerEff(int scheduleTick, AIEffectExecResultWeakPtr innerEff)
    //{
    //    auto pEff = innerEff.lock();
    //    if (!pEff)
    //        return;
    //    
    //    _scheduleMap[scheduleTick][pEff->getUID()] = innerEff;
    //}

    void AIContinuousEffectCaster::scheduleInnerEff(int scheduleTick, AIInnerEffect *pInnerEff)
    {
        _scheduleMap[scheduleTick][pInnerEff->getUID()] = pInnerEff;
    }

    void AIContinuousEffectCaster::unscheduleInnerEff(int innerExecUID)
    {
        for (auto iter = _scheduleMap.begin(); iter != _scheduleMap.end(); ++iter)
        {
            auto &&effs = iter->second;
            auto theIter = effs.find(innerExecUID);
            if (theIter != effs.end())
                theIter->second = nullptr;
        }

        for (auto iter = _appendScheduleEff.begin(); iter != _appendScheduleEff.end(); ++iter)
        {
            auto &&tup = *iter;
            if (std::get<1>(tup) == innerExecUID) {
                std::get<1>(tup) = 0;
                std::get<2>(tup) = nullptr;
            }
        }
    }

    void AIContinuousEffectCaster::appendScheduleInnerEff(int scheduleTick, AIInnerEffect *pInnerEff)
    {
        _appendScheduleEff.push_back(std::make_tuple(scheduleTick, pInnerEff->getUID(), pInnerEff));
    }

    bool AIContinuousEffectCaster::hasInnerEffect(int execID)
    {
        return (innerEffs.find(execID) != innerEffs.end());
    }

    void AIContinuousEffectCaster::extendInnerEffectUITick()
    {
        for (auto iter = innerEffs.begin(); iter != innerEffs.end(); ++iter)
        {
            for (auto iiter = iter->second.begin(); iiter != iter->second.end(); ++iiter)
            {
                auto pCharacter = aiContext.getCharacterByID(*iiter);
                if (!pCharacter)
                    continue;
                auto iiiter = pCharacter->m_aiEffectMap.find(iter->first);
                if (iiiter != pCharacter->m_aiEffectMap.end())
                {
                    for (auto &&pEff : iiiter->second)
                    {
                        if (pEff && pEff->parentCaster->myID == myID)
                            pEff->castUIEffect(totalTick - localTick + startTick);
                    }
                }
            }
        }
    }

    void AIContinuousEffectCaster::castInnerEffectByIdx(int idx, CharacterData *c)
    {
        if (idx < staticData->execs.size() && c)
            castInnerExec(staticData->execs[idx], c);
    }

    int AIContinuousEffectCaster::getStateAnimationResult(CharacterData *c)
    {
        auto iter = stateAniAffectedResult.find(c->myID);
        if (iter != stateAniAffectedResult.end())
            return iter->second;
        return 0;
    }

    int AIContinuousEffectCaster::getEffectValue(CharacterData* aiCharacter, int execID, int param)
    {
        int changeValue = 0;
        auto iter = aiCharacter->m_aiEffectMap.find(execID);
        if (iter != aiCharacter->m_aiEffectMap.end())
        {
            for (auto ite = iter->second.begin(); ite != iter->second.end(); ++ite)
            {
                auto &&innerEff = *ite;
//                if (execID == eEFFECT_CRIT_IF_HIT && param != innerEff->innerExec->targetAttribute)
//                    continue;
                if (innerEff && !innerEff->isTempInvalid())
                    changeValue += innerEff->getChangeValue();
            }
        }

        return changeValue;
    }

    int AIContinuousEffectCaster::getEffectValueWithParam(CharacterData* tmpTarget, int execID, int param)
    {
        auto iter = tmpTarget->m_aiEffectMap.find(execID);
        if (iter == tmpTarget->m_aiEffectMap.end())
            return 0;

        int changeValue = 0;
        for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
        {
            auto &&pEff = *iter2;
            if (!pEff || pEff->isTempInvalid()) continue;
            if (pEff->innerExec->changeType == 1)
            {
                changeValue += pEff->getChangeValue() * param / 100;
            }
            else
            {
                changeValue += pEff->getChangeValue();
            }
        }

        // HQ: 8.0起,这几个效果如果最终数值是减伤,则约定叠加后的效果值最多减伤80%
        switch (execID)
        {
        case eEFFECT_PHY_HARM_DOWN:
        case eEFFECT_MAG_HARM_DOWN:
        case eEFFECT_PHY_HARM_FACE_DOWN:
        case eEFFECT_MAG_HARM_FACE_DOWN:
        case eEFFECT_ALL_HARM_FACE_DOWN:
		case eEFFECT_HURT_ADD: // HQ: 25.0 add
		case eEFFECT_HURT_ADD_EXTRA:
		case eEFFECT_HURT_SUB_EXTRA:
            if (changeValue < 0 && param > 0)
            {
                int limit = param * 8 / 10;
                if (-changeValue > limit)
                    changeValue = -limit;
            }
            break;
        //case eEFFECT_HURT_CHANGE_BY_EFFECT:  //510 cui 2023_2_20
        //{d
        //    RETURN_SMART_STATU
        //    break;
        //}
        default:break;
        }

        // qing.huang: 15.0,该效果不会倒扣能量
        if (execID == eEFFECT_HIT_GAIN_ENERGY_ADD)
        {
            if (!AIScriptBinder::getInstance()->isEffect175DisableLimit(AIContext::getInstance()->getLuaState()))
            {
                if (changeValue < 0 && param > 0 && param + changeValue < 0)
                {
                    changeValue = -param;
                }
            }
        }

        return changeValue;
    }

    void AIContinuousEffectCaster::stopExecute()
    {
        stopFurtherEffect = true;
    }

    void AIContinuousEffectCaster::removeAfterTick(int tick)
    {
        isInvalidated = true;
        invalidateTick = (localTick - startTick - 1) + tick;
    }

    void AIContinuousEffectCaster::castOnTarget(CharacterData* tmpTarget)
    {
        // HQ:8.0新复活可直接作用于已死亡目标
        //if (HeroActionController::isDeadAction(tmpTarget->currentAction))
        //    return;

        if (staticData->stateAnimation > 0 && staticData->canStateAnimationBeDebuffed)
        {
            if (EffectHelper::isDebuffedByType(tmpTarget, staticData->isBuff))
                return;
        }

        if (staticData->stateAnimation > 0 && stateAniAffectedChars.find(tmpTarget->myID) == stateAniAffectedChars.end())
        {
            stateAniAffectedChars[tmpTarget->myID] = true;
            stateAniAffectedResult[tmpTarget->myID] = 0;

            if (HeroActionController::isDeadAction(tmpTarget->currentAction))
                return;

            //若当前拥有免疫buff 现在放一个眩晕或者其他限制效果的技能则无效,但是伤害还是得算
            if (staticData->canStateAnimationBeDebuffed)
            {
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *tmpTarget, EffectType::eEFFECT_DEBUF_REPEL, 0))
                    return;
            }

            // //击飞击倒等情况下,改变目标状态的效果不执行(眩晕石化等),其他的都要执行
            // switch (tmpTarget->currentAction)
            // {
            // case STRUCK_FLY_AWAY_ACTION:
            // case STRUCK_BACKWORD_ACTION:
            // case STRUCK_HOVER_ACTION:
            // case FALL_DOWN_ACTION:
            //     return;
            //     break;
            // default:
            //     break;
            // }
            if (luahelper_actioncontroller_is_escape_effect_stateanim(aiContext.getLuaState(), tmpTarget)) return;

            int stateTick = staticData->period + staticData->recurrenceGrowTimeValue * skillLevel;
            stateTick += EffectHelper::getExtendBuffTick(tmpTarget, staticData->isBuff, stateTick);

            int stateAniResult = 0; //0-不成功 1-切换到EffectState成功 2-延长EffectState成功

            if (tmpTarget->currentAction == EFFECT_ACTION)
            {
                auto currentEffectID = HeroActionController::getEffectID(tmpTarget->actionAtt);
                auto currentEffect = AIDataHelper::getEffectExec(currentEffectID);

                if (!currentEffect)
                    return;

                if (EffectHelper::IsEffectStateAnimationSupportMix(currentEffect->stateAnimation) ||
                    EffectHelper::IsEffectStateAnimationSupportMix(staticData->stateAnimation))
                {
                    int currentRemainTick = tmpTarget->totalTick - (tmpTarget->localTick - tmpTarget->startTick);
                    int extendTick = stateTick - currentRemainTick;
                    if (extendTick > 0)
                    {
                        bool isSuccess = HeroEffectAction::ExtendDuration(*tmpTarget, effectID, extendTick, staticData->canStateAnimationBeDebuffed, *origin, aiContext);
                        stateAniResult = isSuccess ? 2 : 0;
                    }
                    else
                        return;
                }
                else
                {
                    return;
                }
            }
            else
            {
                bool isSuccess = HeroEffectAction::Enter(*tmpTarget, aiContext, staticData->ID, stateTick, tmpTarget->currentAction, tmpTarget->totalTick, tmpTarget->localTick - tmpTarget->startTick, staticData->canStateAnimationBeDebuffed, *origin);
                stateAniResult = isSuccess ? 1 : 0;
            }

            stateAniAffectedResult[tmpTarget->myID] = stateAniResult;
        }

        if (firstRound)
        {
            if (staticData->delExecs.size() > 0)
            {
                bool isUseNewDispel = AIScriptBinder::getInstance()->getCommonConfigBoolean(aiContext.getLuaState(), "is_use_newdispel");
                for (auto delIterator = staticData->delExecs.begin(); delIterator != staticData->delExecs.end(); ++delIterator)
                {
                    if (isUseNewDispel)
                        dispelInnerEffect(aiContext, tmpTarget, 0, *delIterator);
                    else
                        zero_Special_Effect(aiContext, *tmpTarget, EffectType(*delIterator));
                }
            }
        }

        bool bHaveDebufEffect = false;
        int deltaTick = localTick - startTick - 1;
		int execSuccessCnt = 0;

        for (int i = 0; i < staticData->execs.size(); i++)
        {
            auto &innerExec = staticData->execs[i];
            if (innerExec.ID <= 0)
                continue;

            if (!bHaveDebufEffect && innerExec.ID == EffectType::eEFFECT_DEBUF_REPEL && (!tmpTarget->hasEffect(292) || !luahelper_innereffect_is_delayed(aiContext.getLuaState(), this, tmpTarget, i) ))
                bHaveDebufEffect = true;

            if (firstRound || (innerExec.interval>0 && deltaTick % innerExec.interval == 0))
            {
                if (innerExec.interval == 0 && deltaTick>0)
                    continue;

                if (innerExec.interval > 0 && deltaTick % innerExec.interval != 0)
                    continue;

                OutPutFileLine;
                if (aiContext.localRandom->nextInt(0, 100) < innerExec.rate)
                {
                    bool isContinueHurt = false;
                    if (isHurtEffect(innerExec.ID) && staticData->period > 0 && innerExec.interval > 0)
                        isContinueHurt = staticData->period / innerExec.interval >1;

                    // Huang Qing:这里的逻辑比较奇怪,但线上一直是这样的,经策划确认先保持不变,原始修改commit:e61ed08e
                    if (isContinueHurt && check_exert_Special_Effect(aiContext, *tmpTarget, eEFFECT_ONLY_HURT_BY_CONTINUE, 0) > 0)
                        continue;

					if (!tmpTarget->hasEffect(292) || !luahelper_innereffect_check_delayed(aiContext.getLuaState(), this, tmpTarget, i))
					{
						if (castInnerExec(innerExec, tmpTarget))
						{
							execSuccessCnt++;
						}
					}
                }
            }
        }

        //若现在释放的的是免疫技能,可以删除当前所中的控制buff
        if (bHaveDebufEffect)
        {
            if (tmpTarget->currentAction == EFFECT_ACTION && !tmpTarget->hasTag("effnodebuff"))
            {
                tmpTarget->totalTick = 0;
            }

            for (auto pEff : tmpTarget->effectCasts)
            {
                if (pEff && pEff->myID != this->myID && pEff->staticData->stateAnimation > 0 && pEff->staticData->canStateAnimationBeDebuffed)
                    pEff->removeAfterTick(0);
            }
        }

		if (firstRound && staticData->autoClear == 1 && execSuccessCnt <= 0)
		{
			this->removeAfterTick(0);
		}
    }

	void AIContinuousEffectCaster::Excute(AIContext& aiContext)
	{
		AINode::update(aiContext);
//		if (isFrozen) return;
        if (isFrozen && hasInnerEffect(eEFFECT_LAZY_ENTER_BATTLE))
            localTick++;

        int deltaTick = localTick - startTick - 1;
		if ((totalTick == 0 && deltaTick > totalTick)
			|| (totalTick != 0 && deltaTick >= totalTick)
			|| (isInvalidated && deltaTick >= invalidateTick)
            || shouldDeleteSelf)
		{
			delete this;
			return;
		}

        if (staticData->deadClear && HeroActionController::isDeadAction(origin->currentAction))
        {
            delete this;
            return;
        }

		if (stopFurtherEffect) return;

        if (_appendScheduleEff.size()) {
            for (int i = 0; i < _appendScheduleEff.size(); i++) {
                auto &&tup = _appendScheduleEff[i];
                if (std::get<2>(tup))
                    scheduleInnerEff(std::get<0>(tup), std::get<2>(tup));
            }
            _appendScheduleEff.clear();
        }

        if (_scheduleMap.find(localTick) != _scheduleMap.end()) {
            auto &&scheduleEffs = _scheduleMap[localTick];
            for (auto &&iter : scheduleEffs) {
                auto pEff = iter.second;
                if (!pEff)
                    continue;
                pEff->onScheduleTick();
            }
            _scheduleMap.erase(localTick);
        }

        /// 12.0
        if (_schduleUIEffectMap.find(localTick) != _schduleUIEffectMap.end()) {
            auto &&scheduleEffs = _schduleUIEffectMap[localTick];
            int remainTick = totalTick - localTick + startTick;
            for (auto iter = scheduleEffs.begin(); iter != scheduleEffs.end(); ++iter) {
                auto pEff = iter->second;
                if (pEff)
                    pEff->castUIEffect(remainTick);
            }
            _schduleUIEffectMap.erase(localTick);
        }
        ///

        if (AITargetHelper::isUseNewHelper()) {
            std::list<CharacterData *> currentTargets;
            AITargetContext ctx;
            initEffectCasterSearchContext(ctx, this);
            AITargetHelper::searchTarget(&ctx, &currentTargets);
            
            //if (staticData->targetExclude >= 0) {
            //    std::list<CharacterData *> excludeTargets;
            //    AITargetContext ctxExclude;
            //    initEffectCasterSearchExcludeContext(ctxExclude, this);
            //    AITargetHelper::searchTarget(&ctxExclude, &excludeTargets);
            //    for (auto c : excludeTargets) {
            //        currentTargets.remove(c);
            //    }
            //}
            
            for (auto c : currentTargets)
                castOnTarget(c);
        } else {
            auto currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->target, staticData->range[0], staticData->range[1], staticData->range[2], staticData->range[3], staticData->range[2], staticData->range[3], &staticData->targetParams, *this->origin, false, skillID);
            
            //if (staticData->targetExclude >= 0) {
            //    auto excludeTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, staticData->targetExclude, staticData->excludeX1, staticData->excludeX2, staticData->excludeZ1, staticData->excludeZ2, staticData->excludeZ1, staticData->excludeZ2, &staticData->targetExcludeParams, *this->origin, false, skillID);
            //    for (auto iter = excludeTargets->begin(); iter != excludeTargets->end(); ++iter) {
            //        currentTargets->remove(*iter);
            //    }
            //}
            
            for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
            {
                castOnTarget(*iter);
            }
        }

		firstRound = false;
	}

    void AIContinuousEffectCaster::zero_Special_Effect(AIContext& aiContext, CharacterData& aiCharacter, int etype)
    {
        //auto effectResult = aiCharacter.m_aiEffectMap.find(etype);
        //if (effectResult != aiCharacter.m_aiEffectMap.end())
        //{
        //    for (auto eIter = effectResult->second.begin(); eIter != effectResult->second.end(); ++eIter)
        //    {
        //        if (aiContext.seffProxy != nullptr)
        //            aiContext.seffProxy->notify(aiContext, (*eIter)->getUID());

        //        if (aiContext.bodyEffectProxy)
        //            aiContext.bodyEffectProxy->notify(aiContext, (*eIter)->getUID());
        //    }
        //    effectResult->second.clear();
        //}
        
        auto iter = aiCharacter.m_aiEffectMap.find(etype);
        if (iter == aiCharacter.m_aiEffectMap.end())
            return;
        for (auto &&pEff : iter->second)
        {
            if (!pEff)
                continue;
            pEff->parentCaster->removeInnerEffRecord(aiCharacter.myID, etype);
        }
        while (iter->second.size() > 0)
        {
            auto pEff = *iter->second.rbegin();
            //pEff->onParentCleanup();
            pEff->onEraseEffect();
            iter->second.pop_back();
        }
        aiCharacter.m_aiEffectMap.erase(iter);
    }

    void AIContinuousEffectCaster::dispelInnerEffect(AIContext& aiContext, CharacterData *c, int parentEffectCasterId, int innerEffectId)
    {
        auto iter = c->m_aiEffectMap.find(innerEffectId);
        if (iter == c->m_aiEffectMap.end())
            return;

        int theProtectCnt = CharacterFunc::GetInnerEffectProtectCount(c, parentEffectCasterId, innerEffectId);
        if (parentEffectCasterId)
            theProtectCnt += CharacterFunc::GetInnerEffectProtectCount(c, 0, innerEffectId);

        if (theProtectCnt > 0)
            return;

        std::vector<AIInnerEffect *> &theEffects = iter->second;
        for (int i = theEffects.size() - 1; i >= 0; i--)
        {
            AIInnerEffect *anEff = theEffects[i];
            if (!anEff)
                continue;
			if (parentEffectCasterId == 0)
			{
				if (CharacterFunc::GetInnerEffectProtectCount(c, anEff->parentCaster->effectID, innerEffectId) > 0)
				{
					continue;
				}
			}
			else if (parentEffectCasterId != anEff->parentCaster->effectID)
			{
				continue;
			}
            //if (parentEffectCasterId == 0 || parentEffectCasterId == anEff->parentCaster->effectID)
            {
                anEff->parentCaster->removeInnerEffRecord(c->myID, innerEffectId);
                anEff->onEraseEffect();
                theEffects.erase(theEffects.begin() + i);
            }
        }

        if (theEffects.size() <= 0)
            c->m_aiEffectMap.erase(iter);
    }

    void AIContinuousEffectCaster::cover_Special_Effect(AIContext& aiContext, CharacterData& tmpTarget, int etype)
    {
        auto iter = tmpTarget.m_aiEffectMap.find(etype);
        if (iter == tmpTarget.m_aiEffectMap.end())
            return;

        for (auto it = iter->second.begin(); it != iter->second.end();)
        {
            auto pEff = *it;
            if (pEff->parentCaster->staticData->coverBySameType == 1)
            {
                ++it;
            }
            else
            {
                pEff->parentCaster->removeInnerEffRecord(tmpTarget.myID, etype);
                pEff->onEraseEffect();
                it = iter->second.erase(it);
            }
        }

        if (iter->second.size() == 0)
            tmpTarget.m_aiEffectMap.erase(iter);
    }

	void AIContinuousEffectCaster::changeAttrWithLoseHp(AIContext& aiContext, CharacterData* tmpTarget)
	{
        if (tmpTarget->hasEffect(eEFFECT_ATTR_WITH_LOSEHP_ADD) && tmpTarget->getEffectRemainCD(eEFFECT_ATTR_WITH_LOSEHP_ADD) <= 0)
		{
            /*
			int percent = (float)(tmpTarget->getHPTop() - tmpTarget->getHP()) / tmpTarget->getHPTop() * 100;
            std::unordered_map<int, int*>::iterator addValIte = tmpTarget->_loseHpAddAttrMap.begin();
			while (addValIte != tmpTarget->_loseHpAddAttrMap.end()) {
				//CCLOG("old %d value = %d",addValIte->first,aiCharacter.attributes[addValIte->first]);
				tmpTarget->attributes[addValIte->first] -= addValIte->second[1];
				//CCLOG("real %d value = %d",addValIte->first,aiCharacter.attributes[addValIte->first]);
				if (addValIte->second[4] == 1) {
					addValIte->second[1] = percent * addValIte->second[0] / 10000;
				}
				else{
					addValIte->second[1] = percent * addValIte->second[0] / 100;
				}

				//addValIte->second[1] =  MAX(addValIte->second[2], MIN(addValIte->second[3],addValIte->second[1])) ;

				if ((tmpTarget->attributes[addValIte->first] + addValIte->second[1]) < 0) {
					tmpTarget->attributes[addValIte->first] = 0;
					addValIte->second[1] = -tmpTarget->attributes[addValIte->first];
				}
				else
				{
					tmpTarget->attributes[addValIte->first] += addValIte->second[1];
				}

				//CCLOG("new %d value = %d",addValIte->first,aiCharacter.attributes[addValIte->first]);
				++addValIte;
			}
            */
            int percent = (float)(tmpTarget->getHPTop() - tmpTarget->getHP()) / (float)tmpTarget->getHPTop() * 100.0f;
            for (auto iter = tmpTarget->loseHpAddAttrMap.begin(); iter != tmpTarget->loseHpAddAttrMap.end(); ++iter)
            {
                int attrIdx = iter->first;
                auto &&info = iter->second;

                tmpTarget->attributes[attrIdx] -= info.haveAddVal; // restore to origin value
                if (info.changeType == 1)
                    info.haveAddVal = percent * info.baseAddVal / 10000;
                else
                    info.haveAddVal = percent * info.baseAddVal / 100;

                if (tmpTarget->attributes[attrIdx] + info.haveAddVal < 0)
                {
                    tmpTarget->attributes[attrIdx] = 0;
                    info.haveAddVal = -tmpTarget->attributes[attrIdx];
                }
                else
                {
                    tmpTarget->attributes[attrIdx] += info.haveAddVal;
                }

				EffectHelper::limitAttr(&aiContext, tmpTarget, attrIdx);
            }
		}
	}

    void AIContinuousEffectCaster::change_Special_Effect(AIContext& aiContext, CharacterData& aiCharacter, int etype, int param)
    {
        auto effectsIte = aiCharacter.m_aiEffectMap.find(etype);

        if (effectsIte != aiCharacter.m_aiEffectMap.end())
        {
            if (etype == eEFFECT_ATTR_WITH_LOSEHP_ADD)
            {
                AIContinuousEffectCaster::changeAttrWithLoseHp(aiContext, &aiCharacter);
            }
            else
            {
                for (auto iter = effectsIte->second.begin(); iter != effectsIte->second.end();) {
                    auto theEff = *iter;
                    param -= theEff->getChangeValue();
                    if (param >= 0){
                        if (aiContext.seffProxy != nullptr)
                            aiContext.seffProxy->notify(aiContext, theEff->getUID());
                        if (aiContext.bodyEffectProxy)
                            aiContext.bodyEffectProxy->notify(aiContext, theEff->getUID());

                        theEff->setChangeValue(0);

                        //(*iter)->onParentCleanup();
                        theEff->onEraseEffect();
                        
                        iter = effectsIte->second.erase(iter);
                    }
                    else {
                        theEff->setChangeValue(-param);
                        break;
                    }

                }
                if (effectsIte->second.size() == 0)
                {
                    aiCharacter.m_aiEffectMap.erase(effectsIte);
                }
            }
        }
    }

    void AIContinuousEffectCaster::excertOnEscapeEffects(AIContext& aiContext, CharacterData& theTarget)
    {
        if (!theTarget.hasEffect(eEFFECT_GAIN_ENERGY_ON_ESCAPE) || theTarget.getEffectRemainCD(eEFFECT_GAIN_ENERGY_ON_ESCAPE) > 0)
            return;

        bool isInEnergySkill = theTarget.hasTag("energyskill");

        auto effects = theTarget.m_aiEffectMap[eEFFECT_GAIN_ENERGY_ON_ESCAPE];
        for (auto& pEff : effects)
        {
            if (!pEff || pEff->isTempInvalid())
                continue;
            if (pEff->getChangeValue() > 0 && isInEnergySkill)
                continue;

            // 151效果，数值计算基准作为触发效果后回能量的几率
            if (pEff->innerExec->valueBase <= 0 || aiContext.localRandom->nextInt(0, 100) >= pEff->innerExec->valueBase)
                continue;

            int energy = theTarget.getEnergy();
            CharacterFunc::ApplyEnergyDelta(&theTarget, pEff->getChangeValue());
            int endelta = theTarget.getEnergy() - energy;
            if (endelta)
            {
                AISpecialEffectLabel::excute(ENERGY_CHANGE_LABEL, endelta, aiContext, &theTarget);
            }

            AIContinuousEffectCaster::startEffectCD(theTarget, pEff->innerExec);
        }
    }

    void AIContinuousEffectCaster::executeOnHealTriggerEffect(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget)
    {
        if (!tmpTarget || !tmpTarget->hasEffect(eEFFECT_TRIGGER_ON_HEAL) || tmpTarget->getEffectRemainCD(eEFFECT_TRIGGER_ON_HEAL) > 0)
            return;
        auto effectsItr = tmpTarget->m_aiEffectMap.find(eEFFECT_TRIGGER_ON_HEAL);
        auto& effects = effectsItr->second;
        for (auto iter = effects.begin(); iter != effects.end();)
        {
            auto pEff = *iter;
            if (aiContext.seffProxy != nullptr)
                aiContext.seffProxy->notify(aiContext, pEff->getUID());
            if (aiContext.bodyEffectProxy)
                aiContext.bodyEffectProxy->notify(aiContext, pEff->getUID());

            // 数值计算基准作为触发的执行ID
            AIActionController::executeExec(aiContext, pEff->innerExec->valueBase, pEff->skillID, pEff->skillLevel, tmpSource, nullptr, tmpTarget, 0);
            //(*iter)->onParentCleanup();
            iter = effects.erase(iter);

            AIContinuousEffectCaster::startEffectCD(*tmpTarget, pEff->innerExec);
        }

        if (effects.size() == 0)
            tmpTarget->m_aiEffectMap.erase(effectsItr);
    }
    //治疗量溢出 触发执行2023_1_11
    void AIContinuousEffectCaster::checkHealTriggerEffect(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget, int addValue)
    {
        if (addValue <= 0 || !tmpTarget->hasEffect(eEFFECT_HEAL_EXT)) {
            return;
        }

        //检查是否执行治疗溢出相关效果
        auto effectsItr = tmpTarget->m_aiEffectMap.find(eEFFECT_HEAL_EXT);
        auto& effects = effectsItr->second;


        for (auto iter = effects.begin(); iter != effects.end();)
        {
            auto pEff = static_cast<AIEffect508*>(*iter);
            pEff->checkExecuteHeal(aiContext, tmpSource, tmpTarget,  addValue);
            if (pEff->_leftExecuteNum <= 0) {
                iter = effects.erase(iter);
                if (iter == effects.end()) {
                    AIContinuousEffectCaster::zero_Special_Effect(aiContext, *tmpTarget, pEff->innerExec->ID);
                    break;
                }
            }
            else
            {
                iter++;
            }



            
        }
    }

    //检测 治疗溢出（以某一个值为基础）护盾 是否产生，以及此次增加的护盾值
    void AIContinuousEffectCaster::checkShieldEffectAdd(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget, int addValue)
    {
        if (addValue <= 0 || !tmpTarget->hasEffect(eEFFECT_HEAL_SHIELD)) {
            return;
        }

        //检查是否执行治疗溢出相关效果
        auto effectsItr = tmpTarget->m_aiEffectMap.find(eEFFECT_HEAL_SHIELD);
        auto& effects = effectsItr->second;
        for (auto iter = effects.begin(); iter != effects.end(); iter++)
        {
            auto pEff = static_cast<AIEffect509*>(*iter);
            pEff->checkShieldAdd(addValue);
        }
    }
    
    void AIContinuousEffectCaster::killFollowSkillEffect(AIContext &aiContext, CharacterData *t)
    {
        if (t->getTagValue(TAG_FOLLOW_SKILLTICK_EFFECT_CNT) <= 0)
            return;
        for (auto iter = t->effectCasts.begin(); iter != t->effectCasts.end(); )
        {
            auto e = *iter++;
            if (e->isFollowSkillTick())
                delete e;
        }
    }

    bool AIContinuousEffectCaster::isCasterGroupCovered(CharacterData *t, int execId)
    {
        EffectExec *the_caster_data = AIDataHelper::getEffectExec(execId);
        if (the_caster_data->groupID <= 0)
            return false;

        for (AIContinuousEffectCaster *a_caster : t->effectCasts) {
            if (a_caster && a_caster->staticData && a_caster->staticData->groupID == the_caster_data->groupID) {
                if (a_caster->staticData->groupWeight > the_caster_data->groupWeight)
                    return true;
                else if (a_caster->staticData->groupWeight == the_caster_data->groupWeight) {
                    if (the_caster_data->groupCoverRule == 0)
                        return true;
                }
            }
        }
        return false;
    }

    int AIContinuousEffectCaster::checkCoverGroupCasters(AIContext &aiContext, CharacterData *t, int execId, bool isCleanInnerEffect)
    {
        EffectExec *the_caster_data = AIDataHelper::getEffectExec(execId);
        if (the_caster_data->groupID <= 0)
            return 0;

        int covered_caster_cnt = 0;
        for (AIContinuousEffectCaster *a_caster : t->effectCasts) {
            if (a_caster && a_caster->staticData && a_caster->staticData->groupID == the_caster_data->groupID) {
                bool is_cover = false;
                if (a_caster->staticData->groupWeight < the_caster_data->groupWeight)
                    is_cover = true;
                else if (a_caster->staticData->groupWeight == the_caster_data->groupWeight) {
                    if (the_caster_data->groupCoverRule == 1)
                        is_cover = true;
                }

                if (is_cover && !aiContext.isToDeleteCasterInfoExisted(a_caster->myID)) {
                    if (isCleanInnerEffect)
                        a_caster->cleanupInnerEffect();
                    aiContext.addToDeleteCasterInfo(t->myID, eEffectExec, a_caster->myID);
                    covered_caster_cnt++;
                }
            }
        }

        return covered_caster_cnt;
    }

    void AIContinuousEffectCaster::startEffectCD(CharacterData& target, InnerEffectExec* pExec)
    {
        if (!pExec || pExec->triggerCD <= 0)
            return;
        target.innerEffectIntervals[pExec->ID] = pExec->triggerCD;
    }

	bool AIContinuousEffectCaster::isHurtEffect(int type)
	{
        if (type == eEFFECT_CAUSE_HURT_BY_CUR_HP || type == eEFFECT_CAUSE_HURT_BY_LOSE_HP || type == eEFFECT_CAUSE_HURT)
			return true;
		return false;
	}

    void AIContinuousEffectCaster::executeTriggerEffect(CharacterData* target, int triggerEffectID)
    {
        auto triggerFunc = [target, triggerEffectID](EffectType eff, AIContext &aiContext, CharacterData *triggerOrigin, AIProjectile *triggerProjectile, CharacterData *triggerTarget, bool isOneUseEff){
            if (!target || !target->hasEffect(eff) || target->getEffectRemainCD(eff) > 0)
                return;
            auto effectsItr = target->m_aiEffectMap.find(eff);
            auto& effects = effectsItr->second;
            for (auto iter = effects.begin(); iter != effects.end();)
            {
                auto pEff = *iter;
                if (pEff && !pEff->isTempInvalid() && pEff->innerExec->targetAttribute == triggerEffectID) // 目标属性字段作为触发效果ID判断
                {
                    // 数值计算基准作为触发的执行ID
                    AIActionController::executeExec(aiContext, pEff->innerExec->valueBase, pEff->skillID, pEff->skillLevel, triggerOrigin, triggerProjectile, triggerTarget, 0);
                    if (isOneUseEff) {
                        //(*iter)->onParentCleanup();
                        iter = effects.erase(iter);
                        break;
                    }
                    else {
                        ++iter;
                    }
                }
                else
                    ++iter;
            }

            if (effects.size() == 0)
                target->m_aiEffectMap.erase(effectsItr);
        };
        triggerFunc(EffectType::eEFFECT_TRIGGER, aiContext, origin, projectile, target, true);
        triggerFunc(EffectType::eEFFECT_TRIGGER_ANOTHER, aiContext, target, projectile, target->target, false);
    }

    bool AIContinuousEffectCaster::isCharacterAffectedUIEff(int characterMyID)
    {
        auto iter = action_uieffect_affectedCharacters.find(characterMyID);
        return (iter != action_uieffect_affectedCharacters.end() && iter->second != localTick);
    }

    void AIContinuousEffectCaster::addUIEffAffectedCharacter(int characterMyID)
    {
        action_uieffect_affectedCharacters[characterMyID] = localTick;
    }

    bool AIContinuousEffectCaster::castInnerExec(InnerEffectExec& innerExec, CharacterData* target)
    {
        if (target->getEffectRemainCD(innerExec.ID) > 0)
            return false;

        if (HeroActionController::isDeadAction(target->currentAction) && innerExec.ID != eEFFECT_REVIVE)
        {
            return false;
        }

        AIInnerEffect *pInnerEff = AIInnerEffect::create(this, target, &innerExec);
        if (!pInnerEff)
            return false;
        allEffs.push_back(pInnerEff);
        bool isInnerEffSuccess = pInnerEff->onEnter();
        if (isInnerEffSuccess)
        {
            pInnerEff->isValid = true;
            executeTriggerEffect(target, innerExec.ID);

            bool isFirstInnerEff = target->m_aiEffectMap[innerExec.ID].empty();
            target->m_aiEffectMap[innerExec.ID].push_back(pInnerEff);
            //this->m_affectedCharactersMap[target->myID].push_back(pInnerEff);
            innerEffs[innerExec.ID].insert(target->myID);
            
            if (innerExec.canBeAttack == 1)
            {
                target->increaseCannotBeAttackedBuffCnt();
                target->increaseAllCannotBeFoundCnt();
            }

            if (!isCharacterAffectedUIEff(target->myID))
            {
                auto casterRemainTick = totalTick - localTick + startTick;
                /// 12.0
                bool isStartEffRun = false;
                if (aiContext.seffProxy && isFirstInnerEff && innerExec.uiStartEffect > 0 && IsSpecialUIEffectExec(innerExec.uiStartEffect))
                {
                    auto uiEffData = AIDataHelper::getSpecialUIEffectExec(innerExec.uiStartEffect);
                    if (uiEffData && uiEffData->duration < casterRemainTick)
                    {
                        isStartEffRun = true;
                        // 同类效果开始/结束特效注册uid时使用角色uid，以便单独清除
                        if (aiContext.seffProxy && IsSpecialUIEffectExec(innerExec.uiEndEffect))
                            aiContext.seffProxy->notify(aiContext, target->myID);
                        AIActionController::executeSpecialEffect(aiContext, target->myID, innerExec.uiStartEffect, skillLevel, origin, projectile, target, uiEffData->duration);

                        _schduleUIEffectMap[localTick + uiEffData->duration][pInnerEff->getUID()] = pInnerEff;
                    }
                }

                if (!isStartEffRun)
                    pInnerEff->castUIEffect(casterRemainTick);
                ///
                pInnerEff->castBodyEffect(casterRemainTick);
            }

            if (innerExec.ID != eEFFECT_CAST_BY_SPEC_EFFECT)
                AICastBySpecEffect::checkExecute(aiContext, target, origin, innerExec.ID);
        }

        // TODO: 优化
        // 击中文字,可以考虑移到内部效果自行处理
        if (innerExec.label > 0 && !pInnerEff->isIgnoreHitLabel())
        {
            int displayValue = pInnerEff->getHitLabelDisplayValue();
            if (EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD == innerExec.ID && 1 == innerExec.changeType)
                displayValue /= 100;
            if (displayValue)
                AISpecialEffectLabel::excute(innerExec.label, displayValue, aiContext, target);
        }

        return isInnerEffSuccess;
    }

	int AIContinuousEffectCaster::check_exert_Special_Effect(AIContext& aiContext, CharacterData& aiCharacter, int etype, int param)
	{
		auto iter = aiCharacter.m_aiEffectMap.find(etype);

		if (iter != aiCharacter.m_aiEffectMap.end())
		{
            if (iter->second.size() > 0 && aiCharacter.getEffectRemainCD(etype) <= 0)
			{
                //AIContinuousEffectCaster::startEffectCD(aiCharacter, iter->second[0]->innerExec);

				if (etype == eEFFECT_ATTR_WITH_LOSEHP_ADD)
					return 1;

				//int changeType = iter->second[0]->innerExec->changeType;
				//int changeValue = getEffectValue(&aiCharacter, etype);
				//if (changeValue > 0)
				switch (etype) {
                case eEFFECT_NO_DIE: //103			不死
                {
                    int changeValue = getEffectValue(&aiCharacter, etype);
                    aiCharacter.updateHP(changeValue);
                    //HeroIdleAction::Enter(aiCharacter, aiContext, 1, IDLE_ACTION); //HQ:6.0开始不会强制进入待机
                    return changeValue;
                }
					break;
                case eEFFECT_RESTORE: //101			复活
				case eEFFECT_RESTORE_TMP: //102		回光返照  根据伤害回复血量
				case eEFFECT_REBOUND_HP: //113		反弹攻击方血量  根据对方血量伤害对方
				case eEFFECT_BAHIT_ADD: //106		物理命中率降低
                case eEFFECT_MAHIT_ADD: //107		法术命中率降低
				case eEFFECT_CAUSE_HURT_BY_CUR_HP://122 根据当前血量造成伤害
				case eEFFECT_CAUSE_HURT_BY_LOSE_HP://123 根据当前损失血量造成伤害
				case eEFFECT_CAUSE_HURT://125 直接造成伤害
                case eEFFECT_SHIELD_BYCOUNT_PHYSICS:
                case eEFFECT_SHIELD_BYCOUNT_MAGIC:
                case eEFFECT_SHIELD_BYCOUNT_BOTHPHMA:
                case eEFFECT_SHIELD_BYCOUNT_ALL_HURT:
                case eEFFECT_KNOCK_OUT:
                case eEFFECT_FORCE_ATTACK_SELF:
                case eEFFECT_CHANGE_ATTACK_SPEED:
                //case eEFFECT_CRIT_IF_HIT:
                    return getEffectValue(&aiCharacter, etype, param);
					break;
                case eEFFECT_BACRIT_ADD: //104		物理暴击率提升
                case eEFFECT_MACRIT_ADD: //105		法术暴击率提升
				case eEFFECT_BAAVOID_ADD: //108		闪避几率
				case eEFFECT_MAAVOID_ADD: //109		抵抗几率
				case eEFFECT_BAREPEL_ADD: //110		法术免疫
				case eEFFECT_MAREPEL_ADD: //111		物理免疫
				case eEFFECT_ALL_REPEL: //112		免疫所有攻击
				case eEFFECT_CHARACTER_HARD: //116	硬度
				case eEFFECT_REBOUND_BA_HURT://135
				case eEFFECT_REBOUND_MA_HURT://136
                case eEFFECT_BAREDUCE_ADD://138
                case eEFFECT_MAREDUCE_ADD://139
                case eEFFECT_ONLY_HURT_ABOVE_CONDITION:
                case eEFFECT_HEAL_ADD:
                case eEFFECT_HURT_ADD:
                case eEFFECT_HURT_LIMIT:
                case eEFFECT_NEW_HURT_LIMIT:
                case eEFFECT_PHYCRIT_MUL_REDUCE_ADD:
                case eEFFECT_MAGCRIT_MUL_REDUCE_ADD:
                case eEFFECT_PHYCRIT_REDUCE_ADD:
                case eEFFECT_MAGCRIT_REDUCE_ADD:
                case eEFFECT_PHYCRIT_PROB_REDUCE_ADD:
                case eEFFECT_MAGCRIT_PROB_REDUCE_ADD:
                case eEFFECT_SUMMON_HARM_ADD:
                case eEFFECT_CAUSE_ADD_PROPERTY_HURT:
                case eEFFECT_HIT_GAIN_ENERGY_ADD:
                case eEFFECT_PHY_HARM_DOWN:
                case eEFFECT_MAG_HARM_DOWN:
                case eEFFECT_HEALING_DOWN:
                case eEFFECT_PHY_HARM_FACE_DOWN:
                case eEFFECT_MAG_HARM_FACE_DOWN:
                case eEFFECT_ALL_HARM_FACE_DOWN:
                case eEFFECT_ATTACK_GAIN_ENERGY_ADD:
                case eEFFECT_BEHIT_GAIN_ENERGY_ADD:
                case eEFFECT_RESIST_N_REBOUND:
                case eEFFECT_ADD_HURT:
				case eEFFECT_REVIVE:
				case eEFFECT_ENDURE_HURT_BY_CONDITION:
				case eEFFECT_IGNORE_TARGET_BAREPEL:
				case eEFFECT_IGNORE_TARGET_MAREPEL:
				case eEFFECT_HURT_ADD_EXTRA:
				case eEFFECT_HURT_SUB_EXTRA:
                case eEFFECT_HURT_CHANGE_BY_EFFECT:  //510 cui 2023_2_20
                    return getEffectValueWithParam(&aiCharacter, etype, param);
					break;
				case eEFFECT_NO_SKILL_MARK: //114	不能释放技能 计数
				case eEFFECT_NO_ATTACK_MARK: //115	不能普通攻击 计数
				case eEFFECT_DEBUF_REPEL://119 免疫控制技能
				case eEFFECT_NO_HIT_DOWN_FLY_BACK://120 免疫被击飞击倒击退
				case eEFFECT_ATTR_WITH_LOSEHP_ADD://121 根据损失血量增加属性
				case eEFFECT_ONLY_FINAL_SKILL://124 只能放大招(强高虚无)
				case eEFFECT_OTHER_SKLIST://126 使用备用的技能列表
				case eEFFECT_CHARM://127魅惑
				case eEFFECT_WUDI_NOSKILL_DISPEAR://129 不能攻击别人也不能被攻击
				case eEFFECT_ONLY_HURT_BY_CONTINUE://130 只能被持续性伤害所伤害
				case eEFFECT_EXCHANGE_MYLOWER_HP://132己方血量最低单位的血量与随机敌方的血量调换
				case eEFFECT_BE_TAUNTED://133 中了嘲讽技能
				case eEFFECT_WUDI_NOSKILL_APPEAR://134取消129状态
				case eEFFECT_NO_UltraSkill://不能释放大招的状态
                case eEFFECT_STRUCK_HOVER:
                case eEFFECT_HURT_PROPERTY_REPL_LEI:
                case eEFFECT_HURT_PROPERTY_REPL_GUANG:
                case eEFFECT_HURT_PROPERTY_REPL_HUO:
                case eEFFECT_HURT_PROPERTY_REPL_SHUI:
                case eEFFECT_HURT_PROPERTY_REPL_AN:
                case eEFFECT_HURT_PROPERTY_REPL_DU:
                case eEFFECT_INVALIDATE_HURT_PROPERTY:
                case eEFFECT_TRIGGER:
                case eEFFECT_TRIGGER_ON_HEAL:
                case eEFFECT_CHARM_HALF:
                case eEFFECT_CLEAR_BUFF:
                case eEFFECT_FATAL_HURT_TRANS:
                case eEFFECT_MAG_BLOOD_SUCK:
                case eEFFECT_DEBUF_MOVECASTER:
                case eEFFECT_DEBUF_MOVE_EX:
                case eEFFECT_NO_TURN_ACTION:
                case eEFFECT_PICK_RANGE_SKILL:
                case eEFFECT_LIMIT_MOVE_RANGE:
                case eEFFECT_LAZY_ENTER_BATTLE:
                case eEFFECT_THRESHOLD_EXEC:
                case eEFFECT_COUNT_MARK_LEVEL:
                case eEFFECT_COUNT_MARK_LEVEL_PLUS:
                case eEFFECT_HEAL_BY_EFFECT_COUNT:
                case eEFFECT_HEAL_BY_EFFECT_COUNT_PLUS:
                case eEFFECT_EXEC_BY_JUEXINGLV:
                case eEFFECT_INSERT_ATTACK_ACTION:
                case eEFFECT_INSERT_ATTACK_ACTION_PLUS:
                case eEFFECT_EXTEND_BUFF_TICK:
                case eEFFECT_CRITHIT_NOFAIL:
				case eEFFECT_GOBACK_SAVEPOSITION:
                case eEFFECT_BYPASS_AVOID:
                case eEFFECT_EXCUTE_EXEC:
				case eEFFECT_PROMOTE_SKILL_LEVEL:
                case eEFFECT_IGNORE_NO_DIE:
				case eEFFECT_ATTACKLIST_ADD_ONE:
                case eEFFECT_CREATE_HURT_ADD_VALUE:
                case eEFFECT_IGNORE_SILENCE:
                case eEFFECT_ADD_HURT_ON_DIST:
                case eEFFECT_NO_HIT_FORCE_DOWN:
				case eEFFECT_SPECIFY_SEX_CONDITION:
				case eEFFECT_CHANGE_SEX_EFFECT:
				case eEFFECT_ACCORDANCE_SEX_EXEC:
				case eEFFECT_PRIMARYATTRIBUTE_CONDITION:
				case eEFFECT_JUDGE_SUMMON_COUNT:
				case eEFFECT_CURATT_MAXATT_CONDITION:
                case eEFFECT_SLIP_NCAST:
                case eEFFECT_SKIP_SKILLCD_NCAST:
                case eEFFECT_SACRIFICE_HURT_RECV:
                case eEFFECT_SACRIFICE_HURT_SEND:
                case eEFFECT_HURT_BY_DEADFRIEND:
                case eEFFECT_RESIST_FATAL_HURT_NCAST:
                case eEFFECT_CAST_BY_TEAM_HP_RATIO:
                case eEFFECT_CAST_BY_TEAM_DEAD_CNT:
                case eEFFECT_TAUNT_EXTRA:
				case eEFFECT_DUR_HIT_CRIT_EFFECT:
                case eEFFECT_ALLOW_NEGTIVE_HEAL:
                case eEFFECT_CAST_WHEN_FRIEND_SUMMON:
                case eEFFECT_CAST_WHEN_FRIEND_DIE:
                case eEFFECT_CAST_BY_SPEC_LABEL_HURT:
                case eEFFECT_CANNOT_FIND_TARGET:
                case eEFFECT_CANNOT_BE_FOUND_BY_SIDE:
                case eEFFECT_CAST_BY_SPEC_EFFECT:
                case eEFFECT_CAST_BY_SPEC_STATE:
                case eEFFECT_SHIELD_TRANSFER_HURT:
                case eEFFECT_STATE_VERTIGO:
				case eEFFECT_CREATE_HURT_ADD_VALUE_NEW:
				case eEFFECT_CONSUME_HP_ADD_CRUITHURT:
				case eEFFECT_ABSORB_ALLTYPE_HURT_SHIELD:
				case eEFFECT_IGNORE_FIGHT_PARAM_LIMIT:
				case eEFFECT_FATAL_HURT_DO_EFFECT:
                case eEFFECT_FOCUS_TARGET:
                case eEFFECT_FREEZE_IN_EFFECT_ACTION:
                    if (aiCharacter.hasEffect(etype)) return 1;
                    break;
				case eEFFECT_BAMA_CRIT_NCAST:
				{
					for (auto &&pEff : iter->second)
					{
						auto theEff = (AIBAMACritCastEffect *)(pEff);

						if (theEff && !theEff->isTempInvalid() && theEff->isMatchCondition(param))
						{
							return 1;
						}
					}
				}
					break;
				default:
					break;
				}
			}
		}
		return 0;
	}

    void AIContinuousEffectCaster::cleanupInnerEffect()
	{
        for (auto &&iter : innerEffs) {
            auto innerExecID = iter.first;
            auto &&targetMyIDs = iter.second;
            for (auto targetMyID : targetMyIDs) {
                auto  target = aiContext.getCharacterByID(targetMyID);
                if (!target)
                    continue;

                auto innerEffsIter = target->m_aiEffectMap.find(innerExecID);
                if (innerEffsIter == target->m_aiEffectMap.end())
                    continue;

                auto &&the_innerEffs = innerEffsIter->second;
                for (auto innerIter = the_innerEffs.begin(); innerIter != the_innerEffs.end();)
                {
                    auto pEff = *innerIter;
                    if (pEff && pEff->parentCaster->myID == this->myID)
                    {
                        //pEff->onParentCleanup();
                        pEff->onEraseEffect();
                        innerIter = the_innerEffs.erase(innerIter);
                    }
                    else
                    {
                        ++innerIter;
                    }
                }

                if (the_innerEffs.size() == 0)
                    target->m_aiEffectMap.erase(innerEffsIter);
            }
        }

        innerEffs.clear();

		////在特效到时间后 删除特效 或者 进入下一关卡后 清除掉角色身上的效果HeroActionController::cleanup会清除
		////所有清理都是角色中的容器来清理，AIContinuousEffect不负责delete自己拥有的m_affectedCharactersMap
  //      for (auto iter = m_affectedCharactersMap.begin(); iter != m_affectedCharactersMap.end(); ++iter)
		//{
		//	//1.先查询到角色是否存在
		//	CharacterData* pCharacterData = nullptr;
		//	if (aiContext.allCharacters.find(iter->first) != aiContext.allCharacters.end())
		//		pCharacterData = aiContext.allCharacters[iter->first];

  //          if (!pCharacterData || HeroActionController::isDeadAction(pCharacterData->currentAction))
  //          {
  //              iter->second.clear();
  //              continue;
  //          }

  //          for (int i = 0; i < iter->second.size(); ++i)
  //          {
  //              //然后找到角色身上这个buff
  //              auto pAIInnerEffect = iter->second[i].lock();
  //              if (!pAIInnerEffect)
  //                  continue;

  //              auto innerEffectResultsIter = pCharacterData->m_aiEffectMap.find(pAIInnerEffect->innerExec->ID);
  //              if (innerEffectResultsIter == pCharacterData->m_aiEffectMap.end())
  //                  continue;

  //              for (auto characterHaveEffectIter = innerEffectResultsIter->second.begin(); characterHaveEffectIter != innerEffectResultsIter->second.end();)
  //              {
  //                  if ((*characterHaveEffectIter)->parentCaster->myID == myID)
  //                  {
  //                      if (aiContext.seffProxy)
  //                          aiContext.seffProxy->notify(aiContext, (*characterHaveEffectIter)->getUID());
  //                      if (aiContext.bodyEffectProxy)
  //                          aiContext.bodyEffectProxy->notify(aiContext, (*characterHaveEffectIter)->getUID());

  //                      characterHaveEffectIter = innerEffectResultsIter->second.erase(characterHaveEffectIter);
  //                      continue;
  //                  }
  //                  ++characterHaveEffectIter;
  //              }

  //              if (innerEffectResultsIter->second.size() == 0)
  //                  pCharacterData->m_aiEffectMap.erase(innerEffectResultsIter);
  //          }

		//	iter->second.clear();
		//}

  //      m_affectedCharactersMap.clear();
	}

    bool AIContinuousEffectCaster::isFollowSkillTick()
    {
        return staticData->followSkillTick == 1;
    }
	
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIInnerEffect *AIInnerEffect::create(AIContinuousEffectCaster* parent, CharacterData* target, InnerEffectExec* exec)
    {
        AIInnerEffect *pEff = nullptr;
        if (AIScriptBinder::getInstance()->isInnerEffectByLua(parent->aiContext.getLuaState(), exec->ID))
        {
            pEff = new AIEffectScript(parent, target, exec);
            return pEff;
        }
        
        switch (exec->ID) {
        case eEFFECT_KO_BY_COUNT:
            pEff = new AIKOByCountEffect(parent, target, exec);
            break;
        case eEFFECT_SKIP_SKILL_SEQ:
            pEff = new AISkipSkillSeqEffect(parent, target, exec);
            break;
        case eEFFECT_SACRIFICE_HURT_RECV:
            pEff = new AISacrificeHurtRecv(parent, target, exec);
            break;
        case eEFFECT_SACRIFICE_HURT_SEND:
            pEff = new AISacrificeHurtSend(parent, target, exec);
            break;
        case eEFFECT_RESIST_FATAL_HURT_NCAST:
            pEff = new AIResistFatalHurtNCastEffect(parent, target, exec);
            break;
        case eEFFECT_HURT_BY_DEADFRIEND:
            pEff = new AIHurtByDeadFriendEffect(parent, target, exec);
            break;
        case eEFFECT_CAST_BY_TEAM_DEAD_CNT:
            pEff = new AICastByTeamDeadCntEffect(parent, target, exec);
            break;
        case eEFFECT_CAST_BY_TEAM_HP_RATIO:
            pEff = new AICastByTeamHPRatioEffect(parent, target, exec);
            break;
        case eEFFECT_FORCE_ATTACK_SELF:
            pEff = new AIForceAttackSelfEffect(parent, target, exec);
            break;
        case eEFFECT_SKIP_SKILLCD_NCAST:
            pEff = new AISkipCDCastEffect(parent, target, exec);
            break;
        case eEFFECT_SLIP_NCAST:
            pEff = new AISlipCastEffect(parent, target, exec);
            break;
        case eEFFECT_BAMA_CRIT_NCAST:
            pEff = new AIBAMACritCastEffect(parent, target, exec);
            break;
        case eEFFECT_CAST_ON_HURT_RESULT:
            pEff = new AIExcuteOnHurtResultEffect(parent, target, exec);
            break;
        case eEFFECT_PHYCRIT_MUL_REDUCE_ADD:
        case eEFFECT_MAGCRIT_MUL_REDUCE_ADD:
        case eEFFECT_PHYCRIT_PROB_REDUCE_ADD:
        case eEFFECT_MAGCRIT_PROB_REDUCE_ADD:
            pEff = new AIFloatStateEffect(parent, target, exec);
            break;
        case eEFFECT_ATTR_WITH_LOSEHP_ADD:
            pEff = new AILoseHPAddEffect(parent, target, exec);
            break;
        case eEFFECT_EXCHANGE_BA_MA_POWER:
            pEff = new AIExchangePowerEffect(parent, target, exec);
            break;
        case eEFFECT_DEBUF_EFFECT:
        case eEFFECT_GAIN_ENERGY_ON_ESCAPE:
        case eEFFECT_TRIGGER:
        case eEFFECT_EXEC_BY_JUEXINGLV:
        case eEFFECT_INSERT_ATTACK_ACTION:
        case eEFFECT_INSERT_ATTACK_ACTION_PLUS:
        case eEFFECT_EXTEND_BUFF_TICK:
        case eEFFECT_DEBUF_MOVECASTER:
        case eEFFECT_DEBUF_MOVE_EX:
        case eEFFECT_PROP_HURT_ADD:
        case eEFFECT_EXCUTE_EXEC:
        case eEFFECT_RESIST_N_REBOUND:
        case eEFFECT_ADD_HURT_ON_DIST:
        case eEFFECT_DUEL:
        case eEFFECT_SHIELD_BYCOUNT_ALL_HURT:
        case eEFFECT_EXTEND_HIT_FLYNDOWN:
        case eEFFECT_CAST_IFHAS_SKILL:
        case eEFFECT_NO_HIT_FORCE_DOWN:
        case eEFFECT_ADD_HURT:
        case eEFFECT_REVIVE:
        case eEFFECT_CHANGE_ATTACK_SPEED:
        case eEFFECT_CAUSE_ADD_PROPERTY_HURT:
        case eEFFECT_TAUNT_EXTRA:
        case eEFFECT_ALLOW_NEGTIVE_HEAL:
        //case eEFFECT_CRIT_IF_HIT:
        case eEFFECT_CANNOT_BE_FOUND_BY_SIDE:
        case eEFFECT_CANNOT_FIND_TARGET:
        case eEFFECT_EXTEND_EXIST_BUFF:
        //case eEFFECT_ALL_REPEL:
            pEff = new AIStateEffect(parent, target, exec);
            break;
        case eEFFECT_CAST_WHEN_FRIEND_SUMMON:
            pEff = new AICastWhenFriendSummonEffect(parent, target, exec);
            break;
        case eEFFECT_KNOCK_OUT:
            pEff = new AIKnockoutEffect(parent, target, exec);
            break;
        case eEFFECT_FATAL_HURT_TRANS:
            pEff = new AIFatalHurtTransferEffect(parent, target, exec);
            break;
        case eEFFECT_LINKEN_SPHERE:
            pEff = new AILinkenSphereEffect(parent, target, exec);
            break;
        case eEFFECT_FLIMSY_BURYING:
            pEff = new AIFlimsyBuryingEffect(parent, target, exec);
            break;
        case eEFFECT_SHARE_HURT:
            pEff = new AIShareHurtEffect(parent, target, exec);
            break;
        case eEFFECT_CRITHIT_NOFAIL:
            pEff = new AICritHitNoFailEffect(parent, target, exec);
            break;
        case eEFFECT_PICK_RANGE_SKILL:
            pEff = new AIPickRangeSkillEffect(parent, target, exec);
            break;
        case eEFFECT_LIMIT_MOVE_RANGE:
            pEff = new AILimitMoveRangeEffect(parent, target, exec);
            break;
        case eEFFECT_LAZY_ENTER_BATTLE:
            pEff = new AILazyEnterBattleEffect(parent, target, exec);
            break;
        case eEFFECT_THRESHOLD_EXEC:
            pEff = new AIThresholdExecEffect(parent, target, exec);
            break;
        case eEFFECT_COUNT_MARK_LEVEL:
        case eEFFECT_COUNT_MARK_LEVEL_PLUS:
            pEff = new AICountMarkLevelEffect(parent, target, exec);
            break;
        case eEFFECT_HEAL_BY_EFFECT_COUNT:
        case eEFFECT_HEAL_BY_EFFECT_COUNT_PLUS:
            pEff = new AIHealByEffCountEffect(parent, target, exec);
            break;

		case eEFFECT_GOBACK_SAVEPOSITION:
			pEff = new AISavePositionAndResetEffect(parent, target, exec);
			break;

        case eEFFECT_MISS_EXCUTE:
        case eEFFECT_ESCAPE_PHY_EXCUTE:
        case eEFFECT_ESCAPE_MAG_EXCUTE:
            pEff = new AIEscMissExcuteEffect(parent, target, exec);
            break;

		case eEFFECT_PROMOTE_SKILL_LEVEL:
			pEff = new AIPromoteAppointSkillLevelEffect(parent, target, exec);
			break;
        case eEFFECT_IGNORE_NO_DIE:
            pEff = new AIAttackIgnoreNoDieEffect(parent, target, exec);
			break;
		case eEFFECT_ATTACKLIST_ADD_ONE:
			pEff = new AIAttackListAddSomeOneEffect(parent, target, exec);
			break;
        case eEFFECT_CREATE_HURT_ADD_VALUE:
			pEff = new AICreateHurtAddPureValueEffect(parent, target, exec);
			break;
        case eEFFECT_ADD_HURT_ON_SAME_TARGET:
            pEff = new AIAddHurtOnSameTargetEffect(parent, target, exec);
            break;
        case eEFFECT_EXCUTE_ON_ATTR:
            pEff = new AIExcuteOnAttrChangeEffect(parent, target, exec);
            break;
        case eEFFECT_CRIT_ON_ATTR:
            pEff = new AICritOnAttrEffect(parent, target, exec);
            break;
        case eEFFECT_ATTR_CONVERT_ADD:
        case eEFFECT_ADD_ATTR_ON_ATTR:
        case eEFFECT_BALANCE_ATTR:
            pEff = new AIAttributeEffect(parent, target, exec);
            break;
        case eEFFECT_DEBUF_CLEAR_BODY_EXEC:
            pEff = new AIDebufClearBodyExecEffect(parent, target, exec);
            break;
		case eEFFECT_SPECIFY_SEX_CONDITION:
			pEff = new AISpecifySexConditionResultEffect(parent, target, exec);
			break;
		case eEFFECT_CHANGE_SEX_EFFECT:
			pEff = new AIChangeSexEffect(parent, target, exec);
			break;
		case eEFFECT_ACCORDANCE_SEX_EXEC:
			pEff = new AIAccordanceSexExectueEffect(parent, target, exec);
			break;
		case eEFFECT_PRIMARYATTRIBUTE_CONDITION:
			pEff = new AIPrimayAttributeConditionExectueEffect(parent, target, exec);
			break;
		case eEFFECT_JUDGE_SUMMON_COUNT:
			pEff = new AIExecuteJudgeSummonCountEffect(parent, target, exec);
			break;
		case eEFFECT_CURATT_MAXATT_CONDITION:
			pEff = new AIReleaseCurAttrAndMaxAttrContionEffect(parent, target, exec);
			break;
		case eEFFECT_DUR_HIT_CRIT_EFFECT:
			pEff = new AIDurHitCritCastEffect(parent, target, exec);
			break;
        case eEFFECT_CAST_WHEN_FRIEND_DIE:
            pEff = new AICastWhenFriendDownEffect(parent, target, exec);
            break;
        case eEFFECT_CAST_BY_SPEC_LABEL_HURT:
            pEff = new AIEffect258(parent, target, exec);
            break;
        case eEFFECT_HURT_DOWN_FROM_MARK:
            pEff = new AIEffect259(parent, target, exec);
            break;
        case eEFFECT_CAST_BY_SPEC_EFFECT:
            pEff = new AIEffect260(parent, target, exec);
            break;
        case eEFFECT_CRIT_IF_HIT:
            pEff = new AIEffect262(parent, target, exec);
            break;
        case eEFFECT_CAST_BY_SPEC_STATE:
            pEff = new AIEffect264(parent, target, exec);
            break;
        case eEFFECT_SHIELD_TRANSFER_HURT:
            pEff = new AIEffect265(parent, target, exec);
            break;
        case eEFFECT_STATE_VERTIGO:
            pEff = new AIEffect266(parent, target, exec);
            break;

		case eEFFECT_CREATE_HURT_ADD_VALUE_NEW:
			pEff = new AIEffect267(parent, target, exec);
			break;

        case eEFFECT_BAREPEL_ADD:
        case eEFFECT_MAREPEL_ADD:
        case eEFFECT_ALL_REPEL:
            pEff = new AIRepelEffect(parent, target, exec);
            break;
		case eEFFECT_CONSUME_HP_ADD_CRUITHURT:
		{
			pEff = new AIEffect270(parent, target, exec);
		}
			break;
        case eEFFECT_ENDURE_HURT_BY_CONDITION:
            pEff = new AIEffect271(parent, target, exec);
            break;
		case eEFFECT_FATAL_HURT_DO_EFFECT:
		{
			pEff = new AIEffect272(parent, target, exec);
		}
			break;
		case eEFFECT_ABSORB_ALLTYPE_HURT_SHIELD:
		{
			pEff = new AIEffect273(parent, target, exec);
		}
			break;

        case eEFFECT_DIRECT_KO:
            pEff = new AIDirectKoEffect(parent, target, exec);
            break;

        case eEFFECT_CAUSE_HURT_BY_CURR_HP:
            pEff = new AICauseExtraHurtByHp(parent, target, exec);
            break;

        case eEFFECT_COPY_CREATURE_ANIMATION:
            pEff = new AICopyCreatureAnimation(parent, target, exec);
            break;

        case eEFFECT_IGNORE_FIGHT_PARAM_LIMIT:
            pEff = new AIStateEffect(parent, target, exec);
            break;

        case eEFFECT_CAST_BY_TEAM_LOSE_HP:
            pEff = new AICastByTeamLoseHpEffect(parent, target, exec);
            break;

        case eEFFECT_CHANGE_ACCU_MAX_TICK:
            pEff = new AIChangeAccuMaxTickEffect(parent, target, exec);
            break;

        case eEFFECT_CHANGE_HPADD1_BY_MARK:
            pEff = new AIChangeHPAdd1ByMarkEffect(parent, target, exec);
            break;

        case eEFFECT_CAST_BY_STATE:
            pEff = new AICastByStateEffect(parent, target, exec);
            break;

        case eEFFECT_FOCUS_TARGET:
            pEff = new AIStateEffect(parent, target, exec);
            break;

        case eEFFECT_FREEZE_IN_EFFECT_ACTION:
            pEff = new AIEffect281(parent, target, exec);
            break;

        case eEFFECT_HP_LOST:
            pEff = new AIHpLostEffect(parent, target, exec);
            break;
        case eEFFECT_HEAL_EXT:
            pEff = new AIEffect508(parent, target, exec);
            break;
        case eEFFECT_HEAL_SHIELD:
            pEff = new AIEffect509(parent, target, exec);
            break;
        case eEFFECT_SPURTING_HURT:
            pEff = new AIEffect511(parent, target, exec);
            break;

        default:
            break;
        }

        if (!pEff) {
            if (exec->targetAttribute > 0 && exec->targetAttribute < ATT_NUM) {
                pEff = new AIAttributeEffect(parent, target, exec);
            }
            else {
                pEff = new AIStateEffect(parent, target, exec);
            }
        }

        return pEff;
    }

    AIInnerEffect::AIInnerEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : aiContext(pParent->aiContext),
          effectExec(pParent->staticData),
          origin(pParent->origin),
          projectile(pParent->projectile),
          target(pTarget),
          innerExec(pExec),
          skillID(pParent->skillID),
          skillLevel(pParent->skillLevel),
          parentCaster(pParent)
    {
        _originMyID = origin->myID;
        _targetMyID = target->myID;
        _uid = aiContext.local_incremential++;
    }

    AIInnerEffect::~AIInnerEffect()
	{
        //parentCaster->unscheduleInnerEff(getUID());
		//_changeValue = 0;
        //removeUIBodyEffects();
    }

    void AIInnerEffect::onEraseEffect()
    {
        if (_isErased) { return; }

        _isErased = true;
        onExit();
        parentCaster->unscheduleInnerEff(getUID());
        _changeValue = 0;
        removeUIBodyEffects();
        if (innerExec->canBeAttack == 1 && getTargetSafe()) {
            target->decreaseCannotBeAttackedBuffCnt();
            target->decreaseAllCannotBeFoundCnt();
        }

        if (aiContext.seffProxy && getOriginSafe() && getTargetSafe()) {
            if (target->m_aiEffectMap[innerExec->ID].size() <= 1) {
                if (innerExec->uiEndEffect > 0 && IsSpecialUIEffectExec(innerExec->uiEndEffect)) {
                    auto uiEffData = AIDataHelper::getSpecialUIEffectExec(innerExec->uiEndEffect);
                    if (uiEffData) {
                        if (aiContext.seffProxy && IsSpecialUIEffectExec(innerExec->uiStartEffect))
                            aiContext.seffProxy->notify(aiContext, target->myID);
                        AIActionController::executeSpecialEffect(aiContext, target->myID, innerExec->uiEndEffect, skillLevel, origin, projectile, target, uiEffData->duration);
                    }
                }
            }
        }
    }

    CharacterData* AIInnerEffect::getTargetSafe()
    {
        return aiContext.getCharacterByID(_targetMyID);
    }

    CharacterData* AIInnerEffect::getOriginSafe()
    {
        return aiContext.getCharacterByID(_originMyID);
    }

    int AIInnerEffect::caculateChangeValue(bool ignoreValuebase)
    {
        /*float changeValue = (float)(innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;
        _originChangeValue = changeValue;
        if (innerExec->changeType == 1 && !ignoreValuebase)//percentage
        {
            int valueBase = innerExec->valueBase;
            if (valueBase>0 && valueBase<ATT_NUM)
            {
                changeValue = target->attributes[valueBase] * changeValue / 100;
            }
        }
        return changeValue;*/
        return this->caculatePassChangeValue(innerExec->changeValue, ignoreValuebase);
    }
    //计算优化修改 28.7 cui_to_do 
    int AIInnerEffect::caculatePassChangeValue(int realChangeValue,bool ignoreValuebase)
    {
        float changeValue = innerExec->changeValue; 
        if (realChangeValue != 0) {
            changeValue = realChangeValue;
        }
        changeValue = (float)(changeValue + innerExec->levelup * skillLevel) / 100.0f;
        _originChangeValue = changeValue;
        if (innerExec->changeType == 1 && !ignoreValuebase)//percentage
        {
            int valueBase = innerExec->valueBase;
            if (valueBase > 0 && valueBase < ATT_NUM)
            {
                changeValue = target->attributes[valueBase] * changeValue / 100;
            }
        }
        return changeValue;
    }


    int AIInnerEffect::getBoundedChangeValue(int changeValue, InnerEffectExec* pExec, int skillLevel)
    {
        if (!pExec->limitOnOff)
            return changeValue;

        int limitUpper = pExec->limitValUpperBase + pExec->limitValUpperGrowth * skillLevel;
        int limitLower = pExec->limitValLowerBase + pExec->limitValLowerGrowth * skillLevel;

        limitUpper /= 100;
        limitLower /= 100;

        changeValue = std::min(changeValue, limitUpper);
        changeValue = std::max(changeValue, limitLower);

        return changeValue;
    }

    int AIInnerEffect::calculateUndividedChangeValue()
    {
        return innerExec->changeValue + innerExec->levelup * skillLevel;
    }

    int AIInnerEffect::getBoundedUndividedChangeValue(int changeValue)
    {
        if (!innerExec->limitOnOff)
            return changeValue;
        int limitUpper = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
        int limitLower = innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel;

        changeValue = std::min(changeValue, limitUpper);
        changeValue = std::max(changeValue, limitLower);

        return changeValue;
    }

    bool AIInnerEffect::isDebuffed()
    {
        int protectCnt = CharacterFunc::GetInnerEffectProtectCount(target, parentCaster->effectID, innerExec->ID) + CharacterFunc::GetInnerEffectProtectCount(target, 0, innerExec->ID);
        if (protectCnt > 0)
            return false;

        if (target->hasEffect(eEFFECT_DEBUF_EFFECT))
        {
            std::vector<AIInnerEffect *> &debufEffects = target->m_aiEffectMap[eEFFECT_DEBUF_EFFECT];
            for (AIInnerEffect *anEffect : debufEffects)
            {
                if (anEffect && innerExec->ID == anEffect->innerExec->targetAttribute)
                {
                    /// 13.0
                    //增加免疫指定效果的标记位：数值计算基准
                    //0-增加或减少都免疫（和现在一样）1-免疫减少的效果，增加的效果不免疫 2-免疫增加的效果，减少的效果不免疫
                    if (anEffect->innerExec->valueBase == 0)
                    {
                        return true;
                    }
                    else if (anEffect->innerExec->valueBase == 1)
                    {
                        if (this->innerExec->changeValue < 0)
                        {
                            return true;
                        }
                    }
                    else if (anEffect->innerExec->valueBase == 2)
                    {
                        if (this->innerExec->changeValue > 0)
                        {
                            return true;
                        }
                    }
                    else
                    {
                        return true;
                    }
                    ///
                }
            }
        }

        int debuffCnt = CharacterFunc::GetInnerEffectDebuffCount(target, parentCaster->effectID, innerExec->ID) + CharacterFunc::GetInnerEffectDebuffCount(target, 0, innerExec->ID);
        if (debuffCnt > 0)
            return true;

        return false;
    }

    void AIInnerEffect::checkCleanSameTypeExec()
    {
        if (innerExec->onlySameType != 1)
            return;

        //AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, innerExec->ID);
        AIContinuousEffectCaster::cover_Special_Effect(aiContext, *target, innerExec->ID);
    }

    void AIInnerEffect::castUIEffect(int ticks)
    {
        if (AIDataHelper::getAiUiEffectMode() == AIUIEFFECTMODE_VERY_LONG)
            ticks = 3600;
        for (auto execID : innerExec->uiSpecialEffects)
        {
            if (execID <= 0)
                continue;
            AIActionController::executeSpecialEffect(aiContext, _uid, execID, skillLevel, origin, projectile, target, ticks);
            parentCaster->addUIEffAffectedCharacter(target->myID);
        }
    }

    void AIInnerEffect::castBodyEffect(int ticks)
    {
        for (auto bodyEffectID : innerExec->bodyEffects)
        {
            if (bodyEffectID <= 0)
                continue;
            AIActionController::executeBodyEffect(aiContext, _uid, bodyEffectID, skillLevel, origin, projectile, target, ticks);
            parentCaster->addUIEffAffectedCharacter(target->myID);
        }
    }

    void AIInnerEffect::removeUIBodyEffects()
    {
        // 致命伤害转移的特效,在播放完成后自动删除
        if (aiContext.seffProxy && innerExec->ID != eEFFECT_FATAL_HURT_TRANS)
            aiContext.seffProxy->notify(aiContext, _uid);
        if (aiContext.bodyEffectProxy)
            aiContext.bodyEffectProxy->notify(aiContext, _uid);
    }

    void AIInnerEffect::removeUIEffect()
    {
        if (aiContext.seffProxy && innerExec->ID != eEFFECT_FATAL_HURT_TRANS)
            aiContext.seffProxy->notify(aiContext, _uid);
    }

    void AIInnerEffect::removeBodyEffect()
    {
        if (aiContext.bodyEffectProxy)
            aiContext.bodyEffectProxy->notify(aiContext, _uid);
    }

    void AIInnerEffect::setInnerData(const std::string &datakey, int dataval)
    {
        _innerData[datakey] = dataval;
    }

    bool AIInnerEffect::hasInnerData(const std::string &datakey)
    {
        return _innerData.find(datakey) != _innerData.end();
    }

    int AIInnerEffect::getInnerData(const std::string &datakey)
    {
        if (hasInnerData(datakey))
            return _innerData[datakey];
        return 0;
    }

    void AIInnerEffect::setTempInvalid(bool tempInvalid)
    {
        _isTempInvalid = tempInvalid;
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIAttributeEffectResult
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    AIAttributeEffectResult::AIAttributeEffectResult(int theInnerEffectUid, int theAttrIndex) : innerEffectUid(theInnerEffectUid), attrIndex(theAttrIndex)
    {
        isValid = (attrIndex > ECharacterAttr::ID && attrIndex < ECharacterAttr::ATT_LEN);
    }
    
    void AIAttributeEffectResult::applySwapResult(CharacterData *theTarget, int theSwapperAttrValue)
    {
        if (!isValid || !theTarget)
        {
            return;
        }
        
        changeType = ATTR_SWAP;
        target = theTarget;
        
        attrValueOld = target->attributes[attrIndex];
        attrValueNew = theSwapperAttrValue;
        
        target->attributes[attrIndex] = attrValueNew;

		EffectHelper::limitAttr(AIContext::getInstance(), target, attrIndex);
    }
    
    void AIAttributeEffectResult::deapply()
    {
        if (!isValid || !target)
        {
            return;
        }
        
        if (changeType == ATTR_SWAP)
        {
            target->attributes[attrIndex] = attrValueOld;
        }
        else if (changeType == ATTR_CHANGE_RAW)
        {
            target->attributes[attrIndex] = attrValueOld;
        }
        else if (changeType == ATTR_CHANGE_PERCENT)
        {
            target->attributes[attrIndex] = attrValueOld;
        }

		EffectHelper::limitAttr(AIContext::getInstance(), target, attrIndex);
    }
    
    void AIAttributeEffectResult::reapply()
    {
        if (!isValid || !target)
        {
            return;
        }
        
        if (changeType == ATTR_SWAP)
        {
            attrValueOld = target->attributes[attrIndex];
            target->attributes[attrIndex] = attrValueNew;
        }
        else if (changeType == ATTR_CHANGE_RAW)
        {
            auto theChangeValue = attrValueNew - attrValueOld;
            attrValueOld = target->attributes[attrIndex];
            attrValueNew = attrValueOld + theChangeValue;
            if (attrValueNew < 0)
            {
                attrValueNew = 0;
            }
            target->attributes[attrIndex] = attrValueNew;
        }
        else if (changeType == ATTR_CHANGE_PERCENT)
        {
            attrValueOld = target->attributes[attrIndex];
            attrValueNew = attrValueOld + attrValueOld * changePercent / 100;
            if (attrValueNew < 0)
            {
                attrValueNew = 0;
            }
            target->attributes[attrIndex] = attrValueNew;
        }

		EffectHelper::limitAttr(AIContext::getInstance(), target, attrIndex);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIAttributeEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIAttributeEffect::AIAttributeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	int AIAttributeEffect::caculateChangeValue()
	{
        float changeValue = AIInnerEffect::caculateChangeValue();

		if (innerExec->ID == eEFFECT_CAUSE_HURT_BY_CUR_HP)
		{
            float percent = CharacterFunc::GetCharacterHPRatio(*origin) * 100;
            if (percent < 0)
                percent = 0;
			changeValue *= percent;

		}
		else if (innerExec->ID == eEFFECT_CAUSE_HURT_BY_LOSE_HP)
		{
			int percent = 0;
			if (target->getHP() < 0) {
				percent = 100;
			}
			else
			{
				percent = (float)(target->getHPTop() - target->getHP()) / target->getHPTop() * 100;
			}

			changeValue *= percent;
		}
        else if (innerExec->ID == eEFFECT_CAUSE_EXTRAHURT_BY_ALLFRIENDNUM)
        {// clair_wang  // 根据己方角色数量造成额外伤害(幻象个数也包含在内）
            changeValue *= aiContext.getFriendOrRivalAliveCount(origin->teamSide, origin->clusterNum, true);
        }
        else if (innerExec->ID == eEFFECT_CAUSE_EXTRAHURT_BY_FRIENDNUM)
        {// clair_wang  // 根据己方角色数量造成额外伤害(不包含幻象个数）
            changeValue *= aiContext.getFriendOrRivalAliveCount(origin->teamSide, origin->clusterNum, true, true);
        }
        else if (innerExec->ID == eEFFECT_CAUSE_ON_ATTRDELTA)
        {
            int valueBase = innerExec->valueBase;
            if (valueBase > 0 && valueBase < ATT_NUM)
            {
                int attrDelta = origin->attributes[valueBase] - target->attributes[valueBase];
                if (attrDelta < 0)
                    attrDelta = 0;
                changeValue = attrDelta * (innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;
                if (innerExec->changeType == 1)
                    changeValue = target->attributes[innerExec->targetAttribute] * changeValue / 100.0f;

                if (innerExec->params.size() > 1)
                    changeValue += innerExec->params[1];
            }
        }
        else if (innerExec->ID == eEFFECT_ADD_ATTR_ON_ATTR)
        {
            int attrA = innerExec->valueBase;
            if (attrA > 0 && attrA < ATT_NUM)
            {
                changeValue = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;
                changeValue = target->attributes[attrA] * changeValue / 100.0f;
            }
        }

        return changeValue;
	}

    void AIAttributeEffect::checkCleanSameTypeExec()
    {
        if (innerExec->onlySameType != 1)
            return;

        int attrIdx = innerExec->targetAttribute;
        if (attrIdx > 0 && attrIdx < ATT_NUM && innerExec->exitClear == 0)
        {
            int attrChange = AIContinuousEffectCaster::getEffectValue(target, innerExec->ID);
            int attrValue = target->attributes[attrIdx] - attrChange;
            if (attrValue < 0)
                attrValue = 0;
            target->attributes[attrIdx] = attrValue;

			EffectHelper::limitAttr(AIContext::getInstance(), target, attrIdx);
        }

        AIContinuousEffectCaster::cover_Special_Effect(aiContext, *target, innerExec->ID);
    }

    int AIAttributeEffect::applyOnTarget(CharacterData *theTarget, int attributeIndex, int theChangeValue)
    {
        if (theChangeValue == 0)
            return 0;

        // handle value change
        int realchangevalue = 0;
        int oldHPTop = theTarget->getHPTop();
        if (attributeIndex == ECharacterAttr::HP) {

			// 伤害小于一定阈值伤害不生效，且触发执行id 271 clair_wang
			int lowlimit = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *theTarget, EffectType::eEFFECT_ENDURE_HURT_BY_CONDITION, theTarget->getHPTop());
			if (lowlimit > 0 && (std::abs(theChangeValue) < lowlimit))
			{
				AIEndureHurtByConditionEffect::checkExecute(aiContext, theTarget);
				return 0;
			}

            CharacterData *theTransTarget = nullptr;
            if (theTarget->hasHurtTransFunc())
            {
                theTransTarget = theTarget->getHurtTransTarget(-theChangeValue, parentCaster->effectID); //转移时使用的是伤害值,治疗使用负值
            }
            if (theTransTarget)
            {
                theTarget = theTransTarget;
            }
            
            realchangevalue = EffectHelper::applyHPChange(aiContext, origin, projectile, theTarget, skillID, skillLevel, attributeIndex, theChangeValue, innerExec, parentCaster->markID, parentCaster->effectID, this);
        }
        else {
            realchangevalue = EffectHelper::applyNoneHPChange(theTarget, attributeIndex, theChangeValue);
            
            // MARK: 9.0 impl
//            if (innerExec->changeType == 1 && innerExec->valueBase > 0 && innerExec->valueBase < ECharacterAttr::ATT_LEN)
//            {
//                // TODO: percent impl later
//            }
            // TODO: more attributes impl later
            if (realchangevalue && (attributeIndex == PHY_AT || attributeIndex == MAG_AT))
            {
                AIAttributeEffectResult aResult(getUID(), attributeIndex);
                if (innerExec->changeType == 1)
                {
                    aResult.changeType = AIAttributeEffectResult::ATTR_CHANGE_PERCENT;
                    aResult.changePercent = _originChangeValue;
                }
                else
                {
                    aResult.changeType = AIAttributeEffectResult::ATTR_CHANGE_RAW;
                }
                aResult.target = theTarget;
                aResult.attrValueOld = theTarget->attributes[attributeIndex] - realchangevalue;
                aResult.attrValueNew = theTarget->attributes[attributeIndex];
                target->attrEffectResults[_attrIndex].push_back(aResult);
            }
        }

        if (realchangevalue == 0)
            return 0;

        // handle max hp change
        if (attributeIndex == ECharacterAttr::MAX_HP)
        {
            int oldHP = theTarget->getHP();
            if (realchangevalue > 0) // 增加血量上限
            {
                int addhp = realchangevalue;
                // 316:吸收治疗量
                if (theTarget->hasEffect(316))
                {
                    int replnum = luahelper_innereffect_316_trigger(AIContext::getInstance()->getLuaState(), theTarget, realchangevalue);
                    if (realchangevalue > replnum)
                    {
                        addhp = realchangevalue - replnum;
                    }
                    else
                    {
                        addhp = 0;
                    }
                }
                theTarget->updateHP(theTarget->getHP() + addhp);
            }
            else // 减少血量上限
            {
                if (oldHPTop == 0)
                    oldHPTop = 1;

                if (theTarget->getHPTop() == 0)
                    theTarget->setHPTop(1); // 14.0 保底设置为1

                float newHP = (float)oldHP / (float)oldHPTop * (float)(theTarget->getHPTop());

                if (int(newHP) < 1)
                    newHP = 1;// 14.0 当前血量为1时做的保护处理

                theTarget->updateHP(newHP);
            }

            if (!HeroActionController::isDeadAction(theTarget->currentAction))
            {
                // additional hit label
                int hpChange = theTarget->getHP() - oldHP;
                if (hpChange)
                    AISpecialEffectLabel::excute(hpChange > 0 ? NORMAL_HEAL_LABEL : NORMAL_HURT_LABEL, hpChange, aiContext, theTarget);

                if (realchangevalue > 0)
                    AISpecialEffectLabel::excute(MAX_HP_INCREASE, realchangevalue, aiContext, theTarget); // 上限增加
                else if (realchangevalue < 0)
                    AISpecialEffectLabel::excute(MAX_HP_DECREASE, realchangevalue, aiContext, theTarget); // 上限减少
            }
        }

        // handle main attr change
        if (innerExec->ID == eEFFECT_ATTR_CONVERT_ADD || innerExec->ID == eEFFECT_ADD_ATTR_ON_ATTR || innerExec->ID == eEFFECT_BALANCE_ATTR)
        {
            if (attributeIndex >= ECharacterAttr::PHY && attributeIndex <= ECharacterAttr::SPIRIT)
            {
                EffectHelper::applyConvertAttrChange(aiContext, theTarget, attributeIndex, theChangeValue, getUID(), false);
            }
        }

        // handle hit label
        if (aiContext.businessType == BusinessType::CANNON_KING && attributeIndex == ECharacterAttr::HP && realchangevalue < 0)
        {
            // 船炮之王扣血数字由脚本控制
        }
        else if (innerExec->label > 0)
        {
            int displayValue = realchangevalue;
            if (EffectType::eEFFECT_ATTR_WITH_LOSEHP_ADD == innerExec->ID && 1 == innerExec->changeType)
                displayValue /= 100;
            if (displayValue)
                AISpecialEffectLabel::excute(innerExec->label, displayValue, aiContext, theTarget);
        }

        return realchangevalue;
    }

    void AIAttributeEffect::deapplyOnTarget(CharacterData *theTarget, int attributeIndex, int theChangeValue, int theRealChangeValue)
    {
        if (theRealChangeValue == 0)
            return;

        int oldHPTop = theTarget->getHPTop();
        if (attributeIndex != PHY_AT && attributeIndex != MAG_AT)
        {
            int oldValue = theTarget->attributes[attributeIndex];
            int attrValue = oldValue - theRealChangeValue;
            if (attrValue < 0)
                attrValue = 0;
            theTarget->attributes[attributeIndex] = attrValue;

            if (attributeIndex == ECharacterAttr::ENG)
            {
                theTarget->onENDelta(attrValue - oldValue);
            }

			EffectHelper::limitAttr(AIContext::getInstance(), theTarget, attributeIndex);
        }
        else
        {
            EffectHelper::deapplyAttrEffect(getUID(), attributeIndex, theTarget);
        }

        // 血量上限变化,退出时使用相反击中文字
        if (attributeIndex == ECharacterAttr::MAX_HP)
        {
            if (!HeroActionController::isDeadAction(theTarget->currentAction))
            {
                if (theRealChangeValue > 0)
                    AISpecialEffectLabel::excute(MAX_HP_DECREASE, -theRealChangeValue, aiContext, theTarget);
                else if (theRealChangeValue < 0)
                    AISpecialEffectLabel::excute(MAX_HP_INCREASE, -theRealChangeValue, aiContext, theTarget);
            }

            int oldHP = theTarget->getHP();
            if (theRealChangeValue < 0) // 减少血量上限
            {
                if (oldHPTop == 0)
                    oldHPTop = 1;
                float newHP = (float)oldHP / (float)oldHPTop * (float)(theTarget->getHPTop());
                if (newHP < 1)
                    newHP = 1;
                if (theTarget->hasEffect(316) && newHP > oldHP)
                {
                    int hpdelta = newHP - oldHP;
                    int replnum = luahelper_innereffect_316_trigger(aiContext.getLuaState(), theTarget, hpdelta);
                    if (hpdelta > replnum)
                    {
                        hpdelta -= replnum;
                    }
                    else
                    {
                        hpdelta = 0;
                    }
                    newHP = oldHP + hpdelta;
                }
                theTarget->updateHP(newHP);
                //	theTarget->updateHP(theTarget->getHP() - theRealChangeValue);
            }
            else // 增加血量上限
            {
                theTarget->updateHP(theTarget->getHP());
            }

            // MARK: 9.0 只改显示
            if (!HeroActionController::isDeadAction(theTarget->currentAction))
            {
                int hpChange = theTarget->getHP() - oldHP;
                if (hpChange)
                    AISpecialEffectLabel::excute(hpChange > 0 ? NORMAL_HEAL_LABEL : NORMAL_HURT_LABEL, hpChange, aiContext, theTarget);
            }
        }

        if (theChangeValue == 0)
            return;

        if (innerExec->ID == eEFFECT_ATTR_CONVERT_ADD || innerExec->ID == eEFFECT_ADD_ATTR_ON_ATTR || innerExec->ID == eEFFECT_BALANCE_ATTR)
        {
            if (attributeIndex >= ECharacterAttr::PHY && attributeIndex <= ECharacterAttr::SPIRIT)
            {
                EffectHelper::applyConvertAttrChange(aiContext, theTarget, attributeIndex, -theChangeValue, getUID(), true);
            }
        }
    }

	bool AIAttributeEffect::onEnter()
	{
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _attrIndex = innerExec->targetAttribute;

        if (innerExec->ID == eEFFECT_BALANCE_ATTR)
        {
            int attrA = target->attributes[innerExec->targetAttribute];
            int attrB = target->attributes[innerExec->valueBase];

            if (attrA == attrB)
                return false;

            _changeValue = attrA - attrB;
            if (_changeValue > 0)
            {
                _attrIndex = innerExec->valueBase;
            }
            else
            {
                _attrIndex = innerExec->targetAttribute;
                _changeValue = -_changeValue;
            }
        }
        else
        {
            _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);
        }
            
		if (_changeValue == 0)
		{
			return false;
		}
        
#if BATTLE_LOG_TEST
		if (ECharacterAttr::ENG == _attrIndex && _changeValue > 0)
            OPBattleLog("gainenergy", target->teamSide, target->clusterNum, target->attributes[0], _changeValue);
#endif

        /*
        //4.将变化的数值增加在相应的属性上
        if (attributeIndex != ECharacterAttr::HP) {
            _attrRealChangeValue = EffectHelper::applyNoneHPChange(target, attributeIndex, _changeValue);

            if (attributeIndex == ECharacterAttr::MAX_HP) {
                if (_attrRealChangeValue > 0)
                    AISpecialEffectLabel::excute(MAX_HP_INCREASE, _attrRealChangeValue, aiContext, target); // 上限增加
                else if (_attrRealChangeValue < 0)
                    AISpecialEffectLabel::excute(MAX_HP_DECREASE, _attrRealChangeValue, aiContext, target); // 上限减少
            }
        }
        else {
            _attrRealChangeValue = EffectHelper::applyHPChange(aiContext, origin, projectile, target, skillID, skillLevel, attributeIndex, _changeValue, innerExec);

            // 船炮之王扣血数字由脚本控制
            if (aiContext.businessType == BusinessType::CANNON_KING && _attrRealChangeValue < 0)
                _ignoreHitLabel = true;
        }

        if (attributeIndex == ECharacterAttr::MAX_HP)
        {
            int oldHP = target->getHP();
            if (_attrRealChangeValue > 0) // 增加血量上限
                target->updateHP(target->getHP() + _attrRealChangeValue);
            else
                target->updateHP(target->getHP());
            int hpChange = target->getHP() - oldHP;
            if (hpChange)
                AISpecialEffectLabel::excute(hpChange > 0 ? 100002 : 100000, hpChange, aiContext, target);
        }

        if (innerExec->ID == eEFFECT_ADD_ATTR_ON_ATTR)
        {
            if (attributeIndex >= ECharacterAttr::PHY && attributeIndex <= ECharacterAttr::SPIRIT)
            {
                EffectHelper::applyConvertAttrChange(aiContext, target, attributeIndex, _changeValue);
            }
        }
        */

        _attrRealChangeValue = applyOnTarget(target, _attrIndex, _changeValue);
        if (innerExec->type == 3 && _attrRealChangeValue && target->myID != origin->myID && innerExec->ID != eEFFECT_BALANCE_ATTR) // 自己增加,目标减少
            _attrRealChangeValueForOrigin = applyOnTarget(origin, _attrIndex, -_attrRealChangeValue);

        _ignoreHitLabel = true;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

		return true;
	}

	void AIAttributeEffect::onExit()
	{
        if (!isValid)
            return;
        /*
		//1.退出的时候 根据exitClear决定是否清除角色身上的属性加成
        auto target = getTargetSafe();
		if (innerExec->exitClear == 1 && target)
		{
            int attrIdx = innerExec->targetAttribute;
            int attrValue = target->attributes[attrIdx];
            attrValue -= _attrRealChangeValue;
            if (attrValue < 0)
                attrValue = 0;
            target->attributes[attrIdx] = attrValue;

            // 血量上限变化,退出时使用相反击中文字
            if (attrIdx == ECharacterAttr::MAX_HP) {
                if (_attrRealChangeValue > 0)
                    AISpecialEffectLabel::excute(MAX_HP_DECREASE, -_attrRealChangeValue, aiContext, target);
                else if (_attrRealChangeValue < 0)
                    AISpecialEffectLabel::excute(MAX_HP_INCREASE, -_attrRealChangeValue, aiContext, target);

                int oldHP = target->getHP();
                if (_attrRealChangeValue < 0)
                    target->updateHP(target->getHP() - _attrRealChangeValue);
                else
                    target->updateHP(target->getHP());
                int hpChange = target->getHP() - oldHP;
                if (hpChange)
                    AISpecialEffectLabel::excute(hpChange > 0 ? 100002 : 100000, hpChange, aiContext, target);
            }

            if (innerExec->ID == eEFFECT_ADD_ATTR_ON_ATTR)
            {
                if (attrIdx >= ECharacterAttr::PHY && attrIdx <= ECharacterAttr::SPIRIT)
                {
                    EffectHelper::applyConvertAttrChange(aiContext, target, attrIdx, -_changeValue);
                }
            }
		}
        */

        if (innerExec->exitClear == 1)
        {
            auto theTarget = aiContext.getCharacterByID(_targetMyID);
            if (theTarget && _attrRealChangeValue)
                deapplyOnTarget(theTarget, _attrIndex, _changeValue, _attrRealChangeValue);

            if (innerExec->type == 3 && _targetMyID != _originMyID)
            {
                auto theOrigin = aiContext.getCharacterByID(_originMyID);
                if (theOrigin && _attrRealChangeValueForOrigin)
                    deapplyOnTarget(theOrigin, _attrIndex, -_changeValue, _attrRealChangeValueForOrigin);
            }
        }
	}

	AIAttributeEffect::~AIAttributeEffect()
	{
		// onExit();
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AILoseHPAddEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AILoseHPAddEffect::AILoseHPAddEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
	{

	}

    void AILoseHPAddEffect::checkCleanSameTypeExec()
    {
        if (innerExec->onlySameType != 1)
            return;

        int attrIdx = innerExec->targetAttribute;
        if (attrIdx < ATT_NUM && innerExec->exitClear == 0)
        {
            /*
            auto loseHpAddValIter = target->_loseHpAddAttrMap.find(attrIdx);
            if (loseHpAddValIter != target->_loseHpAddAttrMap.end())
            {
                // {baseAddVal,haveAddVal,minAddVal,maxAddVal,changeType}
                target->attributes[attrIdx] = target->attributes[attrIdx] - loseHpAddValIter->second[1];
                loseHpAddValIter->second[1] = 0;
            }
            */
            auto iter = target->loseHpAddAttrMap.find(attrIdx);
            if (iter != target->loseHpAddAttrMap.end())
            {
                target->attributes[attrIdx] -= iter->second.haveAddVal;
                iter->second.haveAddVal = 0;

				EffectHelper::limitAttr(AIContext::getInstance(), target, attrIdx);
            }
        }
        AIContinuousEffectCaster::cover_Special_Effect(aiContext, *target, innerExec->ID);
    }

	bool AILoseHPAddEffect::onEnter()
	{
        if (isDebuffed())
            return false;

		//1.测试先输出玩家当前的属性值
		int attributeIndex = innerExec->targetAttribute;

        checkCleanSameTypeExec();

		//3.计算出changeValue
		//int finalValue = 0;
		int changeValue = (innerExec->changeValue + innerExec->levelup*skillLevel);

		if (innerExec->changeType == 1)//percentage
		{
			if (innerExec->valueBase>0 && innerExec->valueBase<ATT_NUM)
			{
				//策划觉得这个值太小了,希望在掉血的时候再/100
				changeValue = target->attributes[innerExec->valueBase] * changeValue;
			}
		}

        if (innerExec->limitOnOff)
        {
            int limitUpper = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
            int limitLower = innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel;

            changeValue = std::min(changeValue, limitUpper);
            changeValue = std::max(changeValue, limitLower);
        }

        //_changeValue = getBoundedChangeValue(changeValue, innerExec, skillLevel);
        _changeValue = changeValue;

        if (_changeValue == 0)
		{
            return false;
		}

        /*
		// {baseAddVal,haveAddVal,minAddVal,maxAddVal,changeType}
        std::unordered_map<int, int*>::iterator loseHpAddValIter = target->_loseHpAddAttrMap.find(innerExec->targetAttribute);
		if (loseHpAddValIter != target->_loseHpAddAttrMap.end())
		{
				//一般不会进入这里...
			if (innerExec->onlySameType == 0)
			{
				loseHpAddValIter->second[0] += changeValue;

				if (innerExec->changeType == 1) {
					loseHpAddValIter->second[1] += changeValue / 100;
				}
				else{
					loseHpAddValIter->second[1] += changeValue;
				}

			}
		}
		else
        {
			int *tmpInt = new int[5];
			tmpInt[0] = changeValue;

			if (innerExec->changeType == 1) {
				tmpInt[1] = changeValue / 100;
			}
			else{
				tmpInt[1] = changeValue;
			}
			tmpInt[4] = innerExec->changeType;
			target->_loseHpAddAttrMap.insert(std::make_pair(innerExec->targetAttribute, tmpInt));
		}
        */

        auto iter = target->loseHpAddAttrMap.find(innerExec->targetAttribute);
        if (iter != target->loseHpAddAttrMap.end())
        {
            if (innerExec->onlySameType == 0)
            {
                auto &&sameInfo = iter->second;
                sameInfo.baseAddVal += changeValue;
                //if (innerExec->changeType == 1)
                //    sameInfo.haveAddVal += changeValue / 100;
                //else
                //    sameInfo.haveAddVal += changeValue;
            }
        }
        else
        {
            LoseHPAddAttrInfo info;
            info.baseAddVal = changeValue;
            info.changeType = innerExec->changeType;
            //if (innerExec->changeType == 1)
            //    info.haveAddVal = changeValue / 100;
            //else
            //    info.haveAddVal = changeValue;

            target->loseHpAddAttrMap[innerExec->targetAttribute] = info;
        }
		
		AIContinuousEffectCaster::changeAttrWithLoseHp(aiContext, target);

		return true;
	}


	void AILoseHPAddEffect::onExit()
	{
        if (!isValid)
            return;
        auto safeTarget = getTargetSafe();
        if (innerExec->exitClear > 0 && safeTarget)
		{
            /*
            std::unordered_map<int, int*>::iterator addValIte = target->_loseHpAddAttrMap.find(innerExec->ID);
            if (addValIte != target->_loseHpAddAttrMap.end())
			{
                target->attributes[innerExec->targetAttribute] -= addValIte->second[1];
				delete[] addValIte->second;
                target->_loseHpAddAttrMap.erase(addValIte);
			}
            */
            int attrIdx = innerExec->targetAttribute;
            auto iter = safeTarget->loseHpAddAttrMap.find(attrIdx);
            if (iter != safeTarget->loseHpAddAttrMap.end())
            {
                safeTarget->attributes[attrIdx] -= iter->second.haveAddVal;
                target->loseHpAddAttrMap.erase(iter);
                if (safeTarget->attributes[attrIdx] < 0)
                    safeTarget->attributes[attrIdx] = 0;

				EffectHelper::limitAttr(AIContext::getInstance(), safeTarget, attrIdx);
            }
		}
	}

	AILoseHPAddEffect::~AILoseHPAddEffect()
	{
		// onExit();
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIStateEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIStateEffect::AIStateEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIStateEffect::~AIStateEffect()
	{
		// onExit();
	}

    void AIStateEffect::checkCleanSameTypeExec()
    {
        if (innerExec->onlySameType != 1)
            return;

        if (innerExec->exitClear == 0)
        {
            if (innerExec->ID == eEFFECT_CHARM)
            {
                target->outCharmEffect();
            }
            else if (innerExec->ID == eEFFECT_BE_TAUNTED)
            {
                target->outTauntEffect();
            }
            else if (innerExec->ID == eEFFECT_CHARM_HALF)
            {
                target->outHalfCharmEffect();
            }
        }

        AIContinuousEffectCaster::cover_Special_Effect(aiContext, *target, innerExec->ID);
    }

	bool AIStateEffect::onEnter()
	{
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        //bool isTriggered = false;

		if (innerExec->ID == eEFFECT_OTHER_SKLIST)
			target->m_iSkillSeq2Index = 0;


		if (innerExec->ID == eEFFECT_CHARM)
		{
			target->inCharmEffect();
            //isTriggered = true;
		}
        else if (innerExec->ID == eEFFECT_CHARM_HALF)
        {
            target->inHalfCharmEffect();
            //isTriggered = true;
        }
		else if (innerExec->ID == eEFFECT_STRUCK_HOVER)
		{
			int speedY = 10;
			HeroStruckHoverAction::Enter(*target, aiContext, speedY, effectExec->period, *origin);
            //isTriggered = true;
		}
		else if (innerExec->ID == eEFFECT_EXCHANGE_MYLOWER_HP)
		{
            _ignoreHitLabel = true;

            // 己方血量百分比最低单位的血量和随机一个血量百分比高于他的敌人进行血量百分比调换
            // 若找不到合适目标则直接对其进行少量治疗
            float hpRatio = CharacterFunc::GetCharacterHPRatio(*target);
            //
            CharacterData *enemy = nullptr;
            if (innerExec->params.size() > 0)
            {
                int theCasterId = innerExec->params[0];
                if (IsEffectExec(theCasterId))
                {
                    auto theExec = AIDataHelper::getEffectExec(theCasterId);
                    if (theExec)
                    {
                        if (AITargetHelper::isUseNewHelper())
                        {
                            AITargetContext ctx;
                            std::list<CharacterData *> theTargets;
                            initEffectCasterSearchContext(ctx, origin, origin->target, origin, skillID, skillLevel, theExec, target, true);
                            AITargetHelper::searchTarget(&ctx, &theTargets);
                            if (theTargets.size() > 0)
                                enemy = theTargets.front();
                        }
                        else
                        {
                            auto theTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, theExec->target, theExec->range[0], theExec->range[1], theExec->range[2], theExec->range[3], theExec->range[2], theExec->range[3], &theExec->targetParams, *origin, false, skillID, 0, skillLevel, nullptr, target, true);
                            
                            if (theTargets->size() > 0)
                                enemy = theTargets->front();
                        }
                    }
                }
                else if (IsHurtExec(theCasterId))
                {
                    if (AITargetHelper::isUseNewHelper())
                    {
                        std::list<CharacterData *> theTargets;
                        AIContinuousHurtCaster::SimpleSearchHurtCasterTargetNew(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, true, &theTargets);
                        if (theTargets.size())
                            enemy = theTargets.front();
                    }
                    else
                    {
                        auto theTargets = AIContinuousHurtCaster::SimpleSearchHurtCasterTarget(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, true);
                        if (theTargets && theTargets->size() > 0)
                            enemy = theTargets->front();
                    }
                }
            }
            else
            {
                enemy = AIActionController::getRandomCharacterAboveHPLowerLimit(aiContext, *target, target->getCurrentRivalsList(), hpRatio, false);
            }

            if (enemy && enemy->myID != target->myID && CharacterFunc::GetCharacterHPRatio(*enemy) > hpRatio)
            {
                // 先计算被换目标扣血上下限
                int unboundedEnemyTargetHP = float(enemy->getHPTop()) * hpRatio;
                int unboundedEnemyHPDiff = unboundedEnemyTargetHP - enemy->getHP();
                int boundedEnemyHPDiff = getBoundedChangeValue(unboundedEnemyHPDiff, innerExec, skillLevel);
                float hpDiffRatio = fabsf(boundedEnemyHPDiff) / float(enemy->getHPTop());

                // 调换扣血上下限范围内的百分比
                int hpDiffTarget = hpDiffRatio * float(target->getHPTop());

                // 我方加血
                // 25.0:回血上下限
				// 参数:[0搜索目标执行id,1上下限开关,2数值下限基础,3下限成长,4数值上限基础,5上限成长]
				if (innerExec->params.size() >= 6)
				{
					if (innerExec->params[1])
					{
						int limitUpper = innerExec->params[4] + innerExec->params[5] * skillLevel;
						int limitLower = innerExec->params[2] + innerExec->params[3] * skillLevel;

						limitUpper /= 100;
						limitLower /= 100;

						hpDiffTarget = std::min(hpDiffTarget, limitUpper);
						hpDiffTarget = std::max(hpDiffTarget, limitLower);
					}
				}

                CharacterData *theTransTarget = nullptr;
                if (target->hasHurtTransFunc())
                {
                    theTransTarget = target->getHurtTransTarget(-hpDiffTarget, parentCaster->effectID); //转移时使用的是伤害值,治疗使用负值
                }
                CharacterData *theHealTarget = target;
                if (theTransTarget)
                {
                    theHealTarget = theTransTarget;
                }
                
                HeroHurtcalculate::HurtFuncHealTarget(aiContext, *origin, *theHealTarget, hpDiffTarget, innerExec->label, skillID, false, 0);

                // 对方扣血
                do {
                    if (boundedEnemyHPDiff >= 0)
                        break;

                    int hurtLowlimit = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *enemy, EffectType::eEFFECT_ONLY_HURT_ABOVE_CONDITION, enemy->getHPTop());
                    if (hurtLowlimit > 0 && (std::abs(boundedEnemyHPDiff) < hurtLowlimit))
                        break;

					// 伤害小于一定阈值伤害不生效，且触发执行id 271 clair_wang
					int lowlimit = AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *enemy, EffectType::eEFFECT_ENDURE_HURT_BY_CONDITION, enemy->getHPTop());
					if (lowlimit > 0 && (std::abs(boundedEnemyHPDiff) < lowlimit))
					{
						AIEndureHurtByConditionEffect::checkExecute(aiContext, enemy);
						break;
					}

                    theTransTarget = nullptr;
                    if (enemy->hasHurtTransFunc())
                    {
                        theTransTarget = enemy->getHurtTransTarget(-boundedEnemyHPDiff, parentCaster->effectID); //转移时使用的是伤害值,治疗使用负值
                    }
                    if (theTransTarget)
                    {
                        enemy = theTransTarget;
                    }

                    int hpEnemy = enemy->getHP() + boundedEnemyHPDiff;
                    int oldHP = enemy->getHP();
                    // OP2 fix: 换血不会致死
                    if (hpEnemy < 1)
                    {
                        hpEnemy = 1;
                        boundedEnemyHPDiff = hpEnemy - oldHP;
                    }
                    enemy->updateHP(hpEnemy);

                    AISpecialEffectLabel::excute(NORMAL_HURT_LABEL, boundedEnemyHPDiff, aiContext, enemy);

                    StatsFunc::AppendHeroHurt(*origin, HurtType::PHYSICAL_HURT_TYPE, oldHP - enemy->getHP(), *target);
                    enemy->onHPDelta(enemy->getHP() - oldHP);
#if BATTLE_LOG_TEST
                    auto logOrigin = origin->isSummon() ? origin->getSummonSource() : origin;
                    OPBattleLog("dps", logOrigin->teamSide, logOrigin->clusterNum, logOrigin->attributes[0], enemy->teamSide, enemy->clusterNum, enemy->attributes[0], -boundedEnemyHPDiff, HeroActionController::getSkillID(origin->actionAtt) % 10);
#endif

                    //isTriggered = true;
                } while (false);
            }
            else
            {
				int healValue = caculateChangeValue();
				// 25.0:回血上下限
				// 参数:[0搜索目标执行id,1上下限开关,2数值下限基础,3下限成长,4数值上限基础,5上限成长]
				if (innerExec->params.size() >= 6 && innerExec->params[1])
				{
					int limitUpper = innerExec->params[4] + innerExec->params[5] * skillLevel;
					int limitLower = innerExec->params[2] + innerExec->params[3] * skillLevel;

					limitUpper /= 100;
					limitLower /= 100;

					healValue = std::min(healValue, limitUpper);
					healValue = std::max(healValue, limitLower);
				}

                CharacterData *theTransTarget = nullptr;
                if (target->hasHurtTransFunc())
                {
                    theTransTarget = target->getHurtTransTarget(-healValue, parentCaster->effectID); //转移时使用的是伤害值,治疗使用负值
                }
                CharacterData *theHealTarget = target;
                if (theTransTarget)
                    theHealTarget = theTransTarget;
                
                int realHealValue = HeroHurtcalculate::HurtFuncHealTarget(aiContext, *origin, *theHealTarget, healValue, innerExec->label, skillID, false, 0);
                if (realHealValue <= 0)
                {
                    return false;
                }

                //isTriggered = true;
            }
		}
        else if (innerExec->ID == eEFFECT_BE_TAUNTED)
		{
            if (target->hasTag("tauntprime"))
                return false;
            
            target->m_pTmpTarget = origin;
            target->tauntTarget = origin;
            target->inTauntEffect();
		}
        else if (innerExec->ID == eEFFECT_BE_TAUNTED_PRIME)
        {
            if (target->hasTag("tauntprime"))
                return false;
            else if (target->isInTauntEffect())
                target->outTauntEffect();

            target->m_pTmpTarget = origin;
            target->tauntTarget = origin;
            target->inTauntEffect();
            target->addTag("tauntprime");
        }
		else if (innerExec->ID == eEFFECT_WUDI_NOSKILL_APPEAR)
		{
			if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, eEFFECT_WUDI_NOSKILL_DISPEAR, 0) > 0)
				AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, eEFFECT_WUDI_NOSKILL_DISPEAR);

            if (aiContext.summonProxy != nullptr)
                aiContext.summonProxy->hideOrShowSource(aiContext, target, false);

            return 0;
		}
		else if (innerExec->ID == eEFFECT_WUDI_NOSKILL_DISPEAR)
		{
			if (aiContext.summonProxy != nullptr)
			{
				aiContext.summonProxy->hideOrShowSource(aiContext, target, true);
			}
		}
        else if (innerExec->ID == eEFFECT_RESTORE)
        {
            if (target->getRestoredByEffectCount() <= 0)
            {
                auto addHP = _changeValue;
                if (innerExec->changeType == 1)
                    addHP = target->getHPTop() * _changeValue / 100;
                target->updateHP(target->getHP() + addHP);
                target->onEffectRestore();
                AISpecialEffectLabel::excute(NORMAL_HEAL_LABEL, addHP, aiContext, target);
                //isTriggered = true;
            }
        }
        else if (innerExec->ID == eEFFECT_NO_SKILL_MARK)
        {
            target->onEnterSilence();
            //isTriggered = true;
        }
        else if (innerExec->ID == eEFFECT_CLEAR_BUFF)
        {
            bool hasStateAnimationBuff = false;
            for (auto pEffCast : target->effectCasts)
            {
                if (!pEffCast || pEffCast->myID == parentCaster->myID)
                    continue;
                auto iter = std::find(innerExec->params.begin(), innerExec->params.end(), pEffCast->staticData->isBuff);
                if (iter == innerExec->params.end())
                    continue;
                if (pEffCast->staticData->stateAnimation > 0 && !pEffCast->staticData->canStateAnimationBeDebuffed)
                    continue;
                pEffCast->totalTick = 0;
                if (!hasStateAnimationBuff)
                    hasStateAnimationBuff = (pEffCast->staticData->stateAnimation > 0);
            }

            if (hasStateAnimationBuff && target->currentAction == EFFECT_ACTION && !target->hasTag("effnodebuff"))
                target->totalTick = 0;

            //isTriggered = true;
        }
        else if (innerExec->ID == eEFFECT_EXEC_BY_JUEXINGLV)
        {
            int jxlv = innerExec->targetAttribute;
            if (target->juexingLevel == jxlv)
            {
                int execID = innerExec->changeValue;
                AIActionController::executeExec(aiContext, execID, skillID, skillLevel, target, projectile, target->target, 0);
                //isTriggered = true;
            }
        }
        else if (innerExec->ID == eEFFECT_INSERT_ATTACK_ACTION)
        {
            int skillID = innerExec->targetAttribute;
            for (auto iter = target->skillList.begin(); iter != target->skillList.end(); ++iter) {
                if (skillID == iter->first) {
                    //如果打断大招,则要延长大招的暂停时间
                    //OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezetime
                    for (auto iter2 = aiContext.ultraFreezeList.begin(); iter2 != aiContext.ultraFreezeList.end(); ++iter2) {
                        if ((*iter2)[2] != target->myID)
                            continue;
                        (*iter2)[1] += HeroSkillController::calculateSkillLength(skillID);
                    }
                    HeroAttackAction::ForceEnter(*target, aiContext, -1, skillID, iter->second, true, 0, false);
                    //isTriggered = true;
                    break;
                }
            }
        }
        else if (innerExec->ID == eEFFECT_INSERT_ATTACK_ACTION_PLUS) {
            int skillID = innerExec->targetAttribute;
            target->checkForceCastSkill(skillID);
        }
        else if (innerExec->ID == eEFFECT_EXCUTE_EXEC) {
            int theExecID = innerExec->changeValue;
            ExecExcuteInfo execInfo(theExecID, skillID, skillLevel, origin, projectile, target, 0);
            aiContext.scheduleExcuteExec(execInfo);
        }
        else if (innerExec->ID == eEFFECT_RESIST_N_REBOUND) {
            if (innerExec->changeType == 1)
                _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        }
        else if (innerExec->ID ==  eEFFECT_ADD_HURT_ON_DIST) {
            if (innerExec->changeType == 1)
                _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        }
        else if (innerExec->ID == eEFFECT_DUEL)
        {
            origin->onEnterDuel(target);
            target->onEnterDuel(origin);

            // 12.0
            if (!origin->isInTauntEffect())
                HeroActionController::setCurrentTarget(*origin, target);
            if (!target->isInTauntEffect())
                HeroActionController::setCurrentTarget(*target, origin);
        }
        else if (innerExec->ID == eEFFECT_EXTEND_EXIST_BUFF)
        {
            bool shouldExtendEffectState = false;
            for (auto pEffCast : target->effectCasts)
            {
                if (!pEffCast || pEffCast->myID == parentCaster->myID)
                    continue;
                if (innerExec->targetAttribute != pEffCast->staticData->isBuff)
                    continue;
                //auto iter = std::find(innerExec->params.begin(), innerExec->params.end(), pEffCast->staticData->isBuff);
                //if (iter == innerExec->params.end())
                //    continue;
                if (innerExec->changeType == 1)
                    pEffCast->totalTick += pEffCast->totalTick * getChangeValue() / 100;
                else
                    pEffCast->totalTick += getChangeValue();

                pEffCast->extendInnerEffectUITick();

                if (target->currentAction == EFFECT_ACTION && pEffCast->effectID == HeroActionController::getEffectID(target->actionAtt))
                    shouldExtendEffectState = true;
            }

            if (shouldExtendEffectState)
            {
                if (innerExec->changeType == 1)
                    target->totalTick += target->totalTick * getChangeValue() / 100;
                else
                    target->totalTick += getChangeValue();
            }
        }
        else if (innerExec->ID == eEFFECT_EXTEND_HIT_FLYNDOWN)
        {
            //_changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
            _undividedChangeValue = calculateUndividedChangeValue();
        }
        else if (innerExec->ID == eEFFECT_CAST_IFHAS_SKILL)
        {
            int castID = innerExec->targetAttribute;
            int cmpSkillID = innerExec->valueBase;
            for (auto iter = origin->skillList.begin(); iter != origin->skillList.end(); ++iter) {
                if (cmpSkillID == iter->first) {
                    AIActionController::executeExec(aiContext, castID, skillID, skillLevel, origin, projectile, target, 0);
                    break;
                }
            }
        }
        else if (innerExec->ID == eEFFECT_ADD_HURT)
        {
            if (innerExec->changeType == 1)
                _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        }
        else if (innerExec->ID == eEFFECT_REVIVE)
        {
            if (target->isSummon())
                return false;
            if (innerExec->changeType == 1)
                _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        }
        else if (innerExec->ID == eEFFECT_CANNOT_BE_FOUND_BY_SIDE)
        {
            auto theSide = innerExec->targetAttribute;
            if (theSide == 0 || theSide == 2)
                target->cannotBeFoundConfig[0] += 1;
            
            if (theSide == 1 || theSide == 2)
                target->cannotBeFoundConfig[1] += 1;

            target->increaseAllCannotBeFoundCnt();

            if (innerExec->valueBase == 1)
            {
                target->addTag("misseff263withorigin");
            }
        }
        else if (innerExec->ID == eEFFECT_CANNOT_FIND_TARGET)
        {
            // 12.0
            if (aiContext.isBeingFinalClear())
                return false;
            // 0-敌方 1-我方 2-全体
            auto theSide = innerExec->targetAttribute;
            if (theSide == 0 || theSide == 2)
            {
                auto rivalSide = target->teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
                int rivalClusterNum = rivalSide == TeamSide::kEnemy ? aiContext.current_ClusterNumEnemy : aiContext.current_ClusterNumOur;
                auto cs = aiContext.getClusterOrigin(rivalSide, rivalClusterNum);

                for (auto aCharacterId : innerExec->params)
                {
                    target->cannotFindRivalTargets[aCharacterId] += 1;

                    /// 12.0
                    for (auto c : *cs)
                    {
                        if (c->getID() == aCharacterId && !HeroActionController::isDeadAction(c->currentAction))
                            c->increaseAllCannotBeFoundCnt();
                    }
                    ///
                }
            }

            if (theSide == 1 || theSide == 2)
            {
                auto cs = aiContext.getClusterOrigin(target->teamSide, target->clusterNum);
                for (auto aCharacterId : innerExec->params)
                {
                    target->cannotFindFriendTargets[aCharacterId] += 1;

                    for (auto c : *cs)
                    {
                        if (c->getID() == aCharacterId && !HeroActionController::isDeadAction(c->currentAction))
                            c->increaseAllCannotBeFoundCnt();
                    }
                }
            }
        }
        else if (innerExec->ID == eEFFECT_FOCUS_TARGET)
        {
            if (innerExec->params.size() > 0)
            {
                int theCasterId = innerExec->params[0];
                if (IsEffectExec(theCasterId))
                {
                    auto theExec = AIDataHelper::getEffectExec(theCasterId);
                    if (theExec)
                    {
                        if (AITargetHelper::isUseNewHelper())
                        {
                            AITargetContext ctx;
                            initEffectCasterSearchContext(ctx, origin, origin->target, origin, skillID, skillLevel, theExec, target, false);
                            std::list<CharacterData *> theTargets;
                            AITargetHelper::searchTarget(&ctx, &theTargets);
                            if (theTargets.size() > 0)
                                target->targetForFocus = theTargets.front();
                        }
                        else
                        {
                            auto theTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, theExec->target, theExec->range[0], theExec->range[1], theExec->range[2], theExec->range[3], theExec->range[2], theExec->range[3], &theExec->targetParams, *origin, false, skillID, 0, skillLevel, nullptr, target);
                            
                            if (theTargets->size() > 0)
                                target->targetForFocus = theTargets->front();
                        }
                    }
                }
                else if (IsHurtExec(theCasterId))
                {
                    if (AITargetHelper::isUseNewHelper())
                    {
                        std::list<CharacterData *> theTargets;
                        AIContinuousHurtCaster::SimpleSearchHurtCasterTargetNew(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, false, &theTargets);
                        if (theTargets.size())
                            target->targetForFocus = theTargets.front();
                    }
                    else
                    {
                        auto theTargets = AIContinuousHurtCaster::SimpleSearchHurtCasterTarget(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, false);
                        if (theTargets && theTargets->size() > 0)
                            target->targetForFocus = theTargets->front();
                    }
                }
            }
        }
        else if (innerExec->ID == eEFFECT_TAUNT_EXTRA)
        {
            if (innerExec->targetAttribute == 1)
            {
                target->addTag("charm253includesummon");
            }
        }

        //if (isTriggered)
        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

		return true;
	}

	void AIStateEffect::onExit()
	{
        if (!isValid)
            return;
		if (innerExec->exitClear > 0)
		{
            auto theTarget = getTargetSafe();
            if (!theTarget)
                return;

            TeamSide target;
			if (innerExec->ID == eEFFECT_CHARM)
			{
                theTarget->outCharmEffect();
			}
			else if (innerExec->ID == eEFFECT_BE_TAUNTED)
			{
                if (!theTarget->hasTag("tauntprime"))
                    theTarget->outTauntEffect();
			}
            else if (innerExec->ID == eEFFECT_BE_TAUNTED_PRIME)
            {
                theTarget->eraseTag("tauntprime");
                theTarget->outTauntEffect();
            }
			else if (innerExec->ID == eEFFECT_WUDI_NOSKILL_DISPEAR)
			{
				if (aiContext.summonProxy != nullptr)
				{
                    aiContext.summonProxy->hideOrShowSource(aiContext, theTarget, false);
				}
			}
            else if (innerExec->ID == eEFFECT_CHARM_HALF)
            {
                theTarget->outHalfCharmEffect();
            }
            else if (innerExec->ID == eEFFECT_DUEL)
            {
                origin->onExitDuel();
                theTarget->onExitDuel();

                // 类型字段
                // 0 - 效果退出时执行
                // 1 - 有角色死亡时执行
                if (innerExec->type == 0 || HeroActionController::isDeadAction(origin->currentAction) || HeroActionController::isDeadAction(theTarget->currentAction))
                {
                    if (!HeroActionController::isDeadAction(origin->currentAction))
                    {
                        int originExecID = innerExec->targetAttribute;
                        ExecExcuteInfo execInfoOrigin(originExecID, skillID, skillLevel, origin, projectile, origin, 0);
                        aiContext.scheduleExcuteExec(execInfoOrigin);
                    }
                    
                    if (!HeroActionController::isDeadAction(theTarget->currentAction))
                    {
                        int targetExecID = innerExec->valueBase;
                        ExecExcuteInfo execInfoTarget(targetExecID, skillID, skillLevel, origin, projectile, theTarget, 0);
                        aiContext.scheduleExcuteExec(execInfoTarget);
                    }
                }
            }
            else if (innerExec->ID == eEFFECT_CANNOT_BE_FOUND_BY_SIDE)
            {
                //原始相对阵营 0-敌方 1-友方
                auto theSide = innerExec->targetAttribute;
                if (theSide == 0 || theSide == 2)
                {
                    theTarget->cannotBeFoundConfig[0] -= 1;
                    if (theTarget->cannotBeFoundConfig[0] <= 0)
                        theTarget->cannotBeFoundConfig.erase(0);
                }

                if (theSide == 1 || theSide == 2)
                {
                    theTarget->cannotBeFoundConfig[1] -= 1;
                    if (theTarget->cannotBeFoundConfig[1] <= 0)
                        theTarget->cannotBeFoundConfig.erase(1);
                }

                theTarget->decreaseAllCannotBeFoundCnt();

                if (innerExec->valueBase == 1)
                {
                    theTarget->eraseTag("misseff263withorigin");
                }
            }
            else if (innerExec->ID == eEFFECT_CANNOT_FIND_TARGET)
            {
                // 12.0
                if (aiContext.isBeingFinalClear())
                    return;
                auto theSide = innerExec->targetAttribute;
                if (theSide == 0 || theSide == 2)
                {
                    auto rivalSide = theTarget->teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
                    int rivalClusterNum = rivalSide == TeamSide::kEnemy ? aiContext.current_ClusterNumEnemy : aiContext.current_ClusterNumOur;
                    auto cs = aiContext.getClusterOrigin(rivalSide, rivalClusterNum);

                    for (auto aCharacterId : innerExec->params)
                    {
                        theTarget->cannotFindRivalTargets[aCharacterId] -= 1;
                        if (theTarget->cannotFindRivalTargets[aCharacterId] <= 0)
                            theTarget->cannotFindRivalTargets.erase(aCharacterId);

                        /// 12.0
                        for (auto c : *cs)
                        {
                            if (c->getID() == aCharacterId && !HeroActionController::isDeadAction(c->currentAction))
                                c->decreaseAllCannotBeFoundCnt();
                        }
                        ///
                    }
                }

                if (theSide == 1 || theSide == 2)
                {
                    auto cs = aiContext.getClusterOrigin(theTarget->teamSide, theTarget->clusterNum);
                    for (auto aCharacterId : innerExec->params)
                    {
                        theTarget->cannotFindFriendTargets[aCharacterId] -= 1;
                        if (theTarget->cannotFindFriendTargets[aCharacterId] <= 0)
                            theTarget->cannotFindFriendTargets.erase(aCharacterId);

                        /// 12.0
                        for (auto c : *cs)
                        {
                            if (c->getID() == aCharacterId && !HeroActionController::isDeadAction(c->currentAction))
                                c->decreaseAllCannotBeFoundCnt();
                        }
                        ///
                    }
                }
            }
            else if (innerExec->ID == eEFFECT_FOCUS_TARGET)
            {
                theTarget->targetForFocus = nullptr;
            }
            else if (innerExec->ID == eEFFECT_TAUNT_EXTRA)
            {
                if (innerExec->targetAttribute == 1)
                {
                    theTarget->eraseTag("charm253includesummon");
                    theTarget->checkOutSummonCharm253();
                }
            }
		}
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIExchangePowerEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIExchangePowerEffect::AIExchangePowerEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIExchangePowerEffect::~AIExchangePowerEffect()
    {
        // onExit();
    }

    bool AIExchangePowerEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //_changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        // 与一名随机己方的物理法术攻击力互换
        CharacterData *exchangeHero = nullptr;
        if (innerExec->params.size() > 0)
        {
            int theCasterId = innerExec->params[0];
            if (IsEffectExec(theCasterId))
            {
                auto theExec = AIDataHelper::getEffectExec(theCasterId);
                if (theExec)
                {
                    if (AITargetHelper::isUseNewHelper())
                    {
                        AITargetContext ctx;
                        initEffectCasterSearchContext(ctx, origin, origin->target, origin, skillID, skillLevel, theExec, target, false);
                        std::list<CharacterData *> theTargets;
                        AITargetHelper::searchTarget(&ctx, &theTargets);
                        if (theTargets.size() > 0)
                            exchangeHero = theTargets.front();
                    }
                    else
                    {
                        auto theTargets = AIActionController::searchTarget(*origin, origin->target, nullptr, aiContext, theExec->target, theExec->range[0], theExec->range[1], theExec->range[2], theExec->range[3], theExec->range[2], theExec->range[3], &theExec->targetParams, *origin, false, skillID, 0, skillLevel, nullptr, target);
                        
                        if (theTargets->size() > 0)
                            exchangeHero = theTargets->front();
                    }
                }
            }
            else if (IsHurtExec(theCasterId))
            {
                if (AITargetHelper::isUseNewHelper())
                {
                    std::list<CharacterData *> theTargets;
                    AIContinuousHurtCaster::SimpleSearchHurtCasterTargetNew(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, false, &theTargets);
                    if (theTargets.size())
                        exchangeHero = theTargets.front();
                }
                else
                {
                    auto theTargets = AIContinuousHurtCaster::SimpleSearchHurtCasterTarget(aiContext, theCasterId, origin, origin, skillID, skillLevel, target, false);
                    if (theTargets && theTargets->size() > 0)
                        exchangeHero = theTargets->front();
                }
            }
        }
        else
        {
            auto friends = AIActionController::getFilteredFriendList(*target, *target, false, false, false, nullptr, TargetFriendRandom);
            exchangeHero = AIActionController::getRandomCharacterWithoutTarget(aiContext, *target, friends.get(), target, false);
        }
        
        if (!exchangeHero || exchangeHero->myID == target->myID)
            return false;

        _exchangeTarget = exchangeHero;
        _exchangeTargetHeroID = exchangeHero->myID;
        
        /*
        CharacterFunc::SetAttrValueWhenNotExisted(target->myID, ECharacterAttr::PHY_AT, target->getBAPower());
        CharacterFunc::SetAttrValueWhenNotExisted(target->myID, ECharacterAttr::MAG_AT, target->getMAPower());
        CharacterFunc::SetAttrValueWhenNotExisted(exchangeHero->myID, ECharacterAttr::PHY_AT, exchangeHero->getBAPower());
        CharacterFunc::SetAttrValueWhenNotExisted(exchangeHero->myID, ECharacterAttr::MAG_AT, exchangeHero->getMAPower());

        int tmpValue = target->getBAPower();
        target->setBAPower( exchangeHero->getBAPower() );
        exchangeHero->setBAPower( tmpValue );

        tmpValue = target->getMAPower();
        target->setMAPower( exchangeHero->getMAPower() );
        exchangeHero->setMAPower( tmpValue );
         */
        
        // MARK: 9.0 impl
        int theOldBaPowerTarget = target->getBAPower();
        int theOldMaPowerTarget = target->getMAPower();
        EffectHelper::applyAttrSwapEffect(getUID(), PHY_AT, target, exchangeHero->getBAPower());
        EffectHelper::applyAttrSwapEffect(getUID(), PHY_AT, exchangeHero, theOldBaPowerTarget);
        EffectHelper::applyAttrSwapEffect(getUID(), MAG_AT, target, exchangeHero->getMAPower());
        EffectHelper::applyAttrSwapEffect(getUID(), MAG_AT, exchangeHero, theOldMaPowerTarget);

        // 9.0 add
        if (innerExec->targetAttribute > 0)
            AIActionController::executeExec(aiContext, innerExec->targetAttribute, skillID, skillLevel, origin, projectile, target, 0);
        if (innerExec->valueBase > 0)
            AIActionController::executeExec(aiContext, innerExec->valueBase, skillID, skillLevel, origin, projectile, exchangeHero, 0);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIExchangePowerEffect::onExit()
    {
        if (!isValid)
            return;

        /*
        auto theTarget = getTargetSafe();
        if (theTarget)
            EffectHelper::RestoreCharacterBAMAPower(theTarget);

        auto theExchanger = aiContext.getCharacterByID(_exchangeTargetHeroID);
        if (theExchanger)
            EffectHelper::RestoreCharacterBAMAPower(theExchanger);
         */
        
        // MARK: 9.0 impl
        auto theTarget = aiContext.getCharacterByID(_targetMyID);
        if (theTarget)
        {
            EffectHelper::deapplyAttrEffect(getUID(), PHY_AT, theTarget);
            EffectHelper::deapplyAttrEffect(getUID(), MAG_AT, theTarget);
        }
        
        theTarget = aiContext.getCharacterByID(_exchangeTargetHeroID);
        if (theTarget)
        {
            EffectHelper::deapplyAttrEffect(getUID(), PHY_AT, theTarget);
            EffectHelper::deapplyAttrEffect(getUID(), MAG_AT, theTarget);
        }
    }

    void AIExchangePowerEffect::castUIEffect(int ticks)
    {
        // 交换物理法术攻击力效果使用的UI特效，需要根据交换后攻击力较高的属性类型显示相应的特效
        // 静态数据中约定：
        // 特效1：物理
        // 特效2：法术
        auto castFunc = [this, ticks](CharacterData *target){
            int uiEffectIndex = 0;
            if (target->getMAPower() > target->getBAPower())
                uiEffectIndex++;

            std::vector<int> indices;
            indices.push_back(uiEffectIndex);
            indices.push_back(2);

            for (auto idx : indices)
            {
                int execID = innerExec->uiSpecialEffects[idx];
                if (execID <= 0)
                    continue;
                AIActionController::executeSpecialEffect(aiContext, _uid, execID, skillLevel, origin, projectile, target, ticks);
                parentCaster->addUIEffAffectedCharacter(target->myID);
            }
        };

        castFunc(target);
        castFunc(_exchangeTarget);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIKnockoutEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIKnockoutEffect::AIKnockoutEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec)
    {
        
    }

    AIKnockoutEffect::~AIKnockoutEffect()
    {
        // onExit();
    }

    bool AIKnockoutEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        //数值大小填基础秒杀几率，成长值填随单位血量变化增长率
        _changeValue = innerExec->changeValue + (1.0f - CharacterFunc::GetCharacterHPRatio(*target)) * innerExec->levelup;
        if (innerExec->changeType == 1)
            _changeValue /= 100;
        
        _changeValue = getBoundedChangeValue(_changeValue, innerExec, skillLevel);

        // 致命一击未命中时不显示伤害数字
        _ignoreHitLabel = true;

        if (!_changeValue)
            return false;

        if (aiContext.localRandom->nextInt(0, 100) >= _changeValue)
            return false;

        int oldValue = target->getHP();
        target->updateHP(0);
        target->onHurt(true, innerExec->ID, oldValue, innerExec->hurtType - 1, origin, parentCaster->effectID);
        StatsFunc::AppendHeroHurt(*origin, HurtType::PHYSICAL_HURT_TYPE, oldValue, *target);
        target->onHPDelta(target->getHP() - oldValue);

#if BATTLE_LOG_TEST
        auto logOrigin = origin->isSummon() ? origin->getSummonSource() : origin;
        OPBattleLog("dps", logOrigin->teamSide, logOrigin->clusterNum, logOrigin->attributes[0], target->teamSide, target->clusterNum, target->attributes[0], -_changeValue, HeroActionController::getSkillID(origin->actionAtt) % 10);
#endif

        HeroHurtcalculate::characterDeadByHurt(aiContext, target, origin, skillID, skillLevel, nullptr, innerExec);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        // 船炮之王击杀怪物不显示伤害数字
        if (aiContext.businessType != CANNON_KING)
            _ignoreHitLabel = false;

        return true;
    }

    void AIKnockoutEffect::onExit()
    {
        
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                           AIFatalHurtTransferEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIFatalHurtTransferEffect::AIFatalHurtTransferEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AIFatalHurtTransferEffect::~AIFatalHurtTransferEffect()
    {
        // onExit();
    }

    bool AIFatalHurtTransferEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIFatalHurtTransferEffect::onExit()
    {
    }

    void AIFatalHurtTransferEffect::castUIEffect(int ticks)
    {
        //ui特效在触发时播放
    }

    void AIFatalHurtTransferEffect::onTrigger()
    {
        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        //生效时触发，数值大小填执行ID，受致命一击的对象和施法者都起效，释放时以两个人作为施法者，两人各自的目标做为释放目标
        int execID = innerExec->changeValue;
        AIActionController::executeExec(aiContext, execID, skillID, skillLevel, origin, nullptr, origin->target, 0);

        int execID2 = innerExec->targetAttribute;
        AIActionController::executeExec(aiContext, execID2, skillID, skillLevel, target, nullptr, target->target, 0);

        auto castFunc = [this](CharacterData *pTarget, int idx){
            int execID = innerExec->uiSpecialEffects[idx];
            if (execID <= 0)
                return;
            AIActionController::executeSpecialEffect(aiContext, _uid, execID, skillLevel, origin, projectile, pTarget, 0);
            parentCaster->addUIEffAffectedCharacter(pTarget->myID);

            SpecialUIEffectExec* effExec = AIDataHelper::getSpecialUIEffectExec(execID);
            if (effExec)
                aiContext.scheduleNotifyUIEff(aiContext.currentTick + effExec->duration, _uid);
        };

        castFunc(origin, 0);
        castFunc(target, 1);
    }

    AIFatalHurtTransferEffect *AIFatalHurtTransferEffect::getActivatedFatalTransEff(CharacterData *transfer, long worldTick, int sourceMyID)
    {
        auto iter = transfer->m_aiEffectMap.find(eEFFECT_FATAL_HURT_TRANS);
        if (iter == transfer->m_aiEffectMap.end())
            return nullptr;
        auto &effs = iter->second;
        for (auto pEff : effs)
        {
            auto pFatal = (AIFatalHurtTransferEffect *)(pEff);
            if (!pFatal || !pFatal->isTriggered || pFatal->isTempInvalid())
                continue;
            auto source = pFatal->origin;
            if (source && !HeroActionController::isDeadAction(source->currentAction) && source->myID != transfer->myID &&
                pFatal->triggerWorldTick == worldTick && pFatal->triggerCharacterUUID == sourceMyID)
                return pFatal;
        }
        return nullptr;
    }

    AIFatalHurtTransferEffect *AIFatalHurtTransferEffect::getUnactivatedFatalTransEff(CharacterData *transfer, long worldTick, int sourceMyID)
    {
        auto iter = transfer->m_aiEffectMap.find(eEFFECT_FATAL_HURT_TRANS);
        if (iter == transfer->m_aiEffectMap.end())
            return nullptr;

        auto &effs = iter->second;
        for (auto pEff : effs)
        {
            auto pFatal = (AIFatalHurtTransferEffect *)(pEff);
            if (!pFatal || pFatal->isTriggered || pFatal->isTempInvalid())
                continue;
            auto source = pFatal->origin;
            if (source && !HeroActionController::isDeadAction(source->currentAction) && source->myID != transfer->myID)
                return pFatal;
        }
        return nullptr;
    }

    AIFatalHurtTransferEffect *AIFatalHurtTransferEffect::getValidFatalTransferEffect(CharacterData *transfer, long worldTick, int sourceMyID)
    {
        auto pEff = getActivatedFatalTransEff(transfer, worldTick, sourceMyID);
        if (!pEff)
            pEff = getUnactivatedFatalTransEff(transfer, worldTick, sourceMyID);
        return pEff;
    }

    void AIFatalHurtTransferEffect::activateFatalTransferEffect(CharacterData *transfer, long worldTick, int sourceMyID, AIFatalHurtTransferEffect *pEffect)
    {
        if (!pEffect || pEffect->isTriggered)
            return;

        pEffect->isTriggered = true;
        pEffect->triggerWorldTick = worldTick;
        pEffect->triggerCharacterUUID = sourceMyID;

        pEffect->parentCaster->removeAfterTick(1);
        pEffect->onTrigger();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AILinkenSphereEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    AILinkenSphereEffect::AILinkenSphereEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AILinkenSphereEffect::~AILinkenSphereEffect()
    {
        // onExit();
    }

    bool AILinkenSphereEffect::onEnter()
    {
        // 可触发时的特效id：原来的UI特效字段，在触发后需要消失
        // 触发执行时：初始施法者是 罗宾，初始目标是 罗宾的当前目标
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        // 触发时间间隔：效果数值
        activateCD = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;

        // 触发后执行id：数值计算基准
        activateExecID = innerExec->valueBase;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _ignoreHitLabel = true;

        return true;
    }

    void AILinkenSphereEffect::onExit()
    {
        
    }

    void AILinkenSphereEffect::onScheduleTick()
    {
        nextActivateTick = 0;

        if (!HeroActionController::isDeadAction(target->currentAction)) {
            int uitick = parentCaster->totalTick - parentCaster->localTick + parentCaster->startTick;
            castUIEffect(uitick);
        }
    }

    void AILinkenSphereEffect::onActivate()
    {
        AIActionController::executeExec(aiContext, activateExecID, skillID, skillLevel, origin, projectile, target, 0);

        nextActivateTick = parentCaster->localTick + activateCD;
        parentCaster->scheduleInnerEff(nextActivateTick, this);

        // UI特效，在触发后需要消失
        if (aiContext.seffProxy)
            aiContext.seffProxy->notify(aiContext, _uid);

        if (innerExec->label > 0)
            AISpecialEffectLabel::excute(innerExec->label, 0, aiContext, target);
    }

    bool AILinkenSphereEffect::canActivate()
    {
        return (parentCaster->localTick > nextActivateTick);
    }

    bool AILinkenSphereEffect::activateLinkenSphereEffect(CharacterData *source, CharacterData *target, bool isSimulate)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_LINKEN_SPHERE);
        if (iter == target->m_aiEffectMap.end())
            return false;

        auto &&linkens = iter->second;
        for (auto pEff : linkens) {
            if (!pEff || pEff->isTempInvalid())
                continue;
            auto pLinken = (AILinkenSphereEffect*)(pEff);
            if (pLinken->canActivate()) {
                // 设置攻击者，现在只有林肯法球触发的执行目标会用到
                //target->attacker = source;
                if (!isSimulate)
                    pLinken->onActivate();
                //target->attacker = nullptr;
                return true;
            }
        }

        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIFlimsyBuryingEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIFlimsyBuryingEffect::AIFlimsyBuryingEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AIFlimsyBuryingEffect::~AIFlimsyBuryingEffect() {
        // onExit();
    }

    bool AIFlimsyBuryingEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //数值大小, 等级成长->生效时间（要乘以100）
        activeTicks = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;

        //数值计算基准->临界值
        limitHP = innerExec->valueBase;
        if (innerExec->changeType == 1)
            limitHP = limitHP * target->getHPTop() / 100;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIFlimsyBuryingEffect::onExit() {

    }

	void AIFlimsyBuryingEffect::onScheduleTick() 
	{
		EffectHelper::collect_innereffect_record(this);
	}

    void AIFlimsyBuryingEffect::onActivate() {
        if (isActivated)
            return;

        isActivated = true;
        endParentTick = parentCaster->localTick + activeTicks;

		parentCaster->scheduleInnerEff(activeTicks + 1, this);
    }

    bool AIFlimsyBuryingEffect::canActivate(int HPDelta) {
        if (isActivated && parentCaster->localTick > endParentTick)
            return false;
        int tmpHP = target->getHP() - HPDelta;
        return (tmpHP < limitHP);
    }

    int AIFlimsyBuryingEffect::activateFilmsyBuringEffect(CharacterData *target, int HPDelta, bool isSimulate)
    {

        auto iter = target->m_aiEffectMap.find(eEFFECT_FLIMSY_BURYING);
        if (iter == target->m_aiEffectMap.end())
            return HPDelta;

        auto &&burys = iter->second;
        // 找到最高阈值的薄葬
        // 如果有75%和50%血量作为节点，不论如何都是先触发75%的
        AIFlimsyBuryingEffect *pHighestBury = nullptr;
        for (auto pEff : burys) {
            if (!pEff || pEff->isTempInvalid())
                continue;
            AIFlimsyBuryingEffect *bury = (AIFlimsyBuryingEffect *)(pEff);
            if (bury->canActivate(HPDelta)) {
                if (!pHighestBury || pHighestBury->limitHP < bury->limitHP)
                    pHighestBury = bury;
            }
        }

        if (pHighestBury) {
            if (!isSimulate)
                pHighestBury->onActivate();
            return target->getHP() - pHighestBury->limitHP;
        }

        return HPDelta;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIShareHurtEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIShareHurtEffect::AIShareHurtEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }

    AIShareHurtEffect::~AIShareHurtEffect() {
        // onExit();
    }

    bool AIShareHurtEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIShareHurtEffect::onExit() {

    }

    void AIShareHurtEffect::onActivate() {
        lastActivateWorldTick = aiContext.currentTick;
    }

    bool AIShareHurtEffect::isShareHurtValid(AIInnerEffect *eff) {
        if (!eff)
            return false;
        auto pEff = (AIShareHurtEffect *)(eff);
        if (pEff)
            return pEff->lastActivateWorldTick != pEff->aiContext.currentTick;
        return false;
    }

    void AIShareHurtEffect::activateShareHurt(AIShareHurtEffect *pEff) {
        if (pEff)
            pEff->onActivate();
    }

    AIShareHurtEffect *AIShareHurtEffect::getFirstValidShareHurt(CharacterData *character) {
        auto iter = character->m_aiEffectMap.find(eEFFECT_SHARE_HURT);
        if (iter != character->m_aiEffectMap.end()) {
            for (auto pEff : iter->second) {
                if (pEff && !pEff->isTempInvalid() && isShareHurtValid(pEff))
                    return (AIShareHurtEffect *)pEff;
            }
        }
        return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AICritHitNoFailEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICritHitNoFailEffect::AICritHitNoFailEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AICritHitNoFailEffect::~AICritHitNoFailEffect() {
        // onExit();
    }

    bool AICritHitNoFailEffect::checkActivateCritHitNoFailEffect(CharacterData *target) {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CRITHIT_NOFAIL);
        if (iter == target->m_aiEffectMap.end())
            return false;
        for (auto pEff : iter->second) {
            auto pCritEff = (AICritHitNoFailEffect *)(pEff);
            if (!pEff || pEff->isTempInvalid())
                continue;

            pCritEff->onHit();
            if (pCritEff->canActivate()) {
                return true;
            }
        }
        return false;
    }

    void AICritHitNoFailEffect::activateCritHitNoFailEffect(CharacterData *target, bool isCritHit) {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CRITHIT_NOFAIL);
        if (iter == target->m_aiEffectMap.end())
            return;
        for (auto pEff : iter->second) {
            auto pCritEff = (AICritHitNoFailEffect *)(pEff);
            if (!pEff || pEff->isTempInvalid())
                continue;
            if (isCritHit)
                pCritEff->onActivate();
            else {
                pCritEff->onActivateNormalHit();
                return;
            }
        }
    }

    bool AICritHitNoFailEffect::onEnter() {
        if (isDebuffed())
            return false;

        _changeValue = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AICritHitNoFailEffect::onExit() {
    }

    bool AICritHitNoFailEffect::canActivate() {
        return (normalHitCount > _changeValue);
    }

    void AICritHitNoFailEffect::onActivate() {
        normalHitCount = 0;

        // 暴击时触发ID：目标属性
        int execID = innerExec->targetAttribute;
        AIActionController::executeExec(aiContext, execID, skillID, skillLevel, target, nullptr, target->target, 0);
    }

    void AICritHitNoFailEffect::onActivateNormalHit() {
        // 未暴击时触发ID：数值计算基础
        int execID = innerExec->valueBase;
        AIActionController::executeExec(aiContext, execID, skillID, skillLevel, target, nullptr, target->target, 0);
    }

    void AICritHitNoFailEffect::onHit() {
        normalHitCount++;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIPickRangeSkillEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int AIPickRangeSkillEffect::getRangeSkillIndex(CharacterData *pTarget) {
        auto iter = pTarget->m_aiEffectMap.find(eEFFECT_PICK_RANGE_SKILL);
        if (iter == pTarget->m_aiEffectMap.end())
            return -1;
        for (auto pEff : iter->second) {
            auto pRangeEff = (AIPickRangeSkillEffect *)(pEff);
            if (pRangeEff && !pRangeEff->isTempInvalid() && pRangeEff->isCharacterInRange(pTarget->target))
                return CharacterFunc::GetSkillIndexByID(pTarget, pRangeEff->getChangeValue());
        }
        return -1;
    }

    AIPickRangeSkillEffect::AIPickRangeSkillEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {

    }

    AIPickRangeSkillEffect::~AIPickRangeSkillEffect() {
        // onExit();
    }

    bool AIPickRangeSkillEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = caculateChangeValue();

        rangeLower = (innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel) / 100;
        rangeUpper = (innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel) / 100;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIPickRangeSkillEffect::onExit() {

    }

    bool AIPickRangeSkillEffect::isCharacterInRange(CharacterData *character) {
        if (character) {
            int offsetX = (character->getX() - target->getX()) * target->direction;
            return (offsetX >= rangeLower && offsetX <= rangeUpper);
        }
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AILimitMoveRangeEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool AILimitMoveRangeEffect::isTargetOutofLimitMoveEffectRange(CharacterData *pTarget, int posX) {
        auto iter = pTarget->m_aiEffectMap.find(eEFFECT_LIMIT_MOVE_RANGE);
        if (iter == pTarget->m_aiEffectMap.end())
            return false;
        for (auto pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto pLimitRangeEff = (AILimitMoveRangeEffect *)(pEff);
            return (posX < pLimitRangeEff->rangeLower || posX > pLimitRangeEff->rangeUpper);
        }
        return false;
    }

    int AILimitMoveRangeEffect::getLimitMoveEffectLower(CharacterData *pTarget) {
        auto iter = pTarget->m_aiEffectMap.find(eEFFECT_LIMIT_MOVE_RANGE);
        if (iter == pTarget->m_aiEffectMap.end())
            return 0;
        for (auto pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto pLimitRangeEff = (AILimitMoveRangeEffect *)(pEff);
            return pLimitRangeEff->rangeLower;
        }
        return 0;
    }

    int AILimitMoveRangeEffect::getLimitMoveEffectUpper(CharacterData *pTarget) {
        auto iter = pTarget->m_aiEffectMap.find(eEFFECT_LIMIT_MOVE_RANGE);
        if (iter == pTarget->m_aiEffectMap.end())
            return 0;
        for (auto pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto pLimitRangeEff = (AILimitMoveRangeEffect *)(pEff);
            return pLimitRangeEff->rangeUpper;
        }
        return 0;
    }

    AILimitMoveRangeEffect::AILimitMoveRangeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec){

    }

    AILimitMoveRangeEffect::~AILimitMoveRangeEffect() {
        // onExit();
    }

    bool AILimitMoveRangeEffect::onEnter() {
        if (isDebuffed())
            return false;

        int totalRange = aiContext.clusterRangeToCenterX * 2;

        int lowerRatio = (innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel);
        rangeLower = aiContext.getBattleFieldXMostLeft() + totalRange * lowerRatio / 10000;

        int upperRatio = (innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel);
        rangeUpper = aiContext.getBattleFieldXMostLeft() + totalRange * upperRatio / 10000;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AILimitMoveRangeEffect::onExit() {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AILazyEnterBattleEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AILazyEnterBattleEffect::AILazyEnterBattleEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {

    }
    AILazyEnterBattleEffect::~AILazyEnterBattleEffect() {
        // onExit();
    }
    bool AILazyEnterBattleEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }
    void AILazyEnterBattleEffect::onExit() {
        if (!isValid)
            return;
        std::vector<int> execs = { 
            innerExec->limitValLowerBase,
            innerExec->limitValLowerGrowth,
            innerExec->limitValUpperBase,
            innerExec->limitValUpperGrowth
        };
        for (auto execID : execs) {
            if (execID > 0)
                AIActionController::executeExec(aiContext, execID, skillID, skillLevel, target, nullptr, target->target, 0);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            EffectHelper
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIThresholdExecEffect::AIThresholdExecEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }
    AIThresholdExecEffect::~AIThresholdExecEffect() {
        // onExit();
    }
    bool AIThresholdExecEffect::onEnter() {
        if (isDebuffed())
            return false;
        checkCleanSameTypeExec();

        _changeValue = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;
        int attrIdx = innerExec->targetAttribute;
        if (attrIdx > 0 && attrIdx < ATT_NUM) {
            // 2 - 目标属性低于阈值下限触发
            // 3 - 目标属性高于阈值上限触发
            // 4 - 目标属性在阈值内触发
            bool canExec = false;
            int lowerLimit = (innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel) / 100;
            int upperLimit = (innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel) / 100;
            if (innerExec->changeType == 1) {
                int originAttrValue = origin->attributes[attrIdx];
                lowerLimit = originAttrValue * lowerLimit / 100;
                upperLimit = originAttrValue * upperLimit / 100;
            }
            int attrValue = target->attributes[attrIdx];
            switch (innerExec->limitOnOff) {
            case 2:
                canExec = (attrValue < lowerLimit);
                break;
            case 3:
                canExec = (attrValue > upperLimit);
                break;
            case 4:
                canExec = (attrValue >= lowerLimit && attrValue <= upperLimit);
                break;
            default:
                break;
            }
            if (canExec) {
                int execID = _changeValue;
                AIActionController::executeExec(aiContext, execID, skillID, skillLevel, origin, projectile, target, 0);
            }
        }

        return true;
    }
    void AIThresholdExecEffect::onExit() {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            EffectHelper
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICountMarkLevelEffect::AICountMarkLevelEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }

    AICountMarkLevelEffect::~AICountMarkLevelEffect() {
        // onExit();
    }

    bool AICountMarkLevelEffect::onEnter() {
        if (isDebuffed())
            return false;
        checkCleanSameTypeExec();
        return true;
    }

    void AICountMarkLevelEffect::onExit() {

    }

    int AICountMarkLevelEffect::getMarkCount() {
        if (innerExec->ID == eEFFECT_COUNT_MARK_LEVEL_PLUS && HeroActionController::isDeadAction(target->currentAction))
            return 0;
        int markID = innerExec->targetAttribute;
        int count = 0;
        for (auto pMark : target->markCasts) {
            if (pMark && pMark->markID == markID)
                count += pMark->getMarkLevel();
        }
        return count;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIHealByEffCountEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIHealByEffCountEffect::AIHealByEffCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }

    AIHealByEffCountEffect::~AIHealByEffCountEffect() {
        // onExit();
    }

    bool AIHealByEffCountEffect::onEnter() {
        if (isDebuffed())
            return false;

        _ignoreHitLabel = true;

        checkCleanSameTypeExec();
        int effID = innerExec->targetAttribute;
        int totalCount = 0;
        int originMyID = _originMyID;
        InnerEffectExec *pEffExec = nullptr;
        
        auto calcTotalCount = [&totalCount, &pEffExec, effID, originMyID](CharacterDataListPtr characters){
            for (auto pCharacter : *characters) {
                auto iter = pCharacter->m_aiEffectMap.find(effID);
                if (iter != pCharacter->m_aiEffectMap.end()) {
                    for (auto pEff : iter->second) {
                        if (pEff->origin->myID != originMyID)
                            continue;
                        auto pCountEff = (AICountMarkLevelEffect *)(pEff);
                        totalCount += pCountEff->getMarkCount();
                        if (!pEffExec)
                            pEffExec = pEff->innerExec;
                        break;
                    }
                }
            }
        };

        CharacterDataListPtr currentOurCluster = aiContext.getClusterOrigin(TeamSide::kOur, aiContext.current_ClusterNumOur);
        CharacterDataListPtr currentRivals = aiContext.getClusterOrigin(TeamSide::kEnemy, aiContext.current_ClusterNumEnemy);
        calcTotalCount(currentOurCluster);
        calcTotalCount(currentRivals);

        if (totalCount <= 0)
            return false;

        if (pEffExec) {
            int countLimit = (pEffExec->limitValUpperBase + pEffExec->limitValUpperGrowth * skillLevel) / 100;
            if (totalCount > countLimit)
                totalCount = countLimit;
        }

        //每层对应的物理/法术攻击力百分比系数，作为治疗值计算基准。
        //总的治疗量百分比 = a*x^2+b*x+c
        //其中a=0.002,b=0.055,c=0.25
        //需用该结果乘以毒Q的法术攻击力
        int formula = totalCount * totalCount * 2 + totalCount * 55 + 250;
        int mapower = origin->getMAPower();
        if (eEFFECT_HEAL_BY_EFFECT_COUNT_PLUS == innerExec->ID) {
            if (parentCaster->originFirstHealMAPower)
                mapower = *(parentCaster->originFirstHealMAPower);
            else
                parentCaster->originFirstHealMAPower = std::unique_ptr<int>(new int(mapower));
        }
        int healValue = mapower * formula / 1000;
        healValue = getBoundedChangeValue(healValue, innerExec, skillLevel);

        CharacterData *theTransTarget = nullptr;
        if (target->hasHurtTransFunc())
        {
            theTransTarget = target->getHurtTransTarget(-healValue, parentCaster->effectID); //转移时使用的是伤害值,治疗使用负值
        }
        CharacterData *theHealTarget = target;
        if (theTransTarget)
            theHealTarget = theTransTarget;

        _changeValue = HeroHurtcalculate::HurtFuncHealTarget(aiContext, *origin, *theHealTarget, healValue, innerExec->label, skillID, false, 0);

        return true;
    }

    void AIHealByEffCountEffect::onExit() {

    }
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AISavePositionAndResetEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AISavePositionAndResetEffect::AISavePositionAndResetEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		:AIInnerEffect(pParent, pTarget, pExec) 
	{
	}

	AISavePositionAndResetEffect::~AISavePositionAndResetEffect()
	{
		// onExit();
	}

	bool AISavePositionAndResetEffect::onEnter()
	{
		if (isDebuffed())
			return false;

        checkCleanSameTypeExec();

		pos_x = target->getX();
		pos_y = target->getY();

		if (target->currentAction == STRUCK_FLY_AWAY_ACTION)
		{
			pos_y = target->oldY;
		}

		return true;
	}

	void AISavePositionAndResetEffect::onExit() 
	{
        if (!isValid)
            return;
		if (innerExec && innerExec->exitClear > 0)
		{
			auto target = getTargetSafe();
			if (!target)
				return;
		
			/*
			可以打断他的效果：
			击飞，击倒，击退，攻击硬直，眩晕，石化，冰冻。
			*/

			bool bFlag = false;

			for (int i = 0; i < innerExec->params.size(); i++)
			{
				if (innerExec->params[i] == target->currentAction)
				{
					bFlag = true;
					break;
				}
			}

			if (!bFlag)
			{
				target->setX(pos_x);
				target->setY(pos_y);
			}
			

			/*switch (target->currentAction)
			{
			case ActionType::EFFECT_ACTION:
			case ActionType::IN_HURT_ACTION:
			case ActionType::STRUCK_BACKWORD_ACTION:
			case ActionType::STRUCK_FLY_AWAY_ACTION:
			case ActionType::STRUCK_HOVER_ACTION:
			case ActionType::FALL_DOWN_ACTION:
			case ActionType::STANDUP_ACTION:
			case ActionType::TURN_ACTION:
				break;
			default:
			{
				target->setX(pos_x);
				target->setY(pos_y);
			}
				break;
			}*/
		}
	}

    // clair_wang 新增效果：提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）	
    AIPromoteAppointSkillLevelEffect::AIPromoteAppointSkillLevelEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {}

    AIPromoteAppointSkillLevelEffect::~AIPromoteAppointSkillLevelEffect()
    {
        // onExit();
    }

    bool AIPromoteAppointSkillLevelEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;

        for (auto iter = target->skillList.begin(); iter != target->skillList.end(); ++iter)
        {
            if (innerExec->targetAttribute == iter->first)
            {
                //iter->second += _changeValue;

                if (_changeValue >= (innerExec->limitValLowerBase / 100))
                {
                    int execID = innerExec->limitValLowerGrowth;

                    ExecExcuteInfo info(execID, skillID, skillLevel, origin, projectile, target, 0);
                    aiContext.scheduleExcuteExec(info);

                    //	AIActionController::executeExec(aiContext, execID, skillLevel, target, nullptr, target->target, 0);
                }

                break;
            }
        }


        return true;
    }

    void AIPromoteAppointSkillLevelEffect::onExit()
    {
    }

    bool AIAttackIgnoreNoDieEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto pSource = aiContext.getCharacterByID(target->killerUID);
        if (!pSource)
            return false;

        const auto &characters = aiContext.getIgnoreNoDieEffCharacters();
        for (auto iter = characters.begin(); iter != characters.end(); ++iter)
        {
            auto pCharacter = aiContext.getCharacterByID(iter->first);
            if (!pCharacter)
                continue;

            auto iter2 = pCharacter->m_aiEffectMap.find(eEFFECT_IGNORE_NO_DIE);
            if (iter2 == pCharacter->m_aiEffectMap.end())
                return false;

            for (auto pIgnore : iter2->second)
            {
                auto pEff = (AIAttackIgnoreNoDieEffect *)(pIgnore);
                if (!pEff || pEff->isTempInvalid()) continue;
                if (pEff->mCharacterID == pSource->getID() && pEff->mSkillID == target->killerSkillID)
                {
                    if (pEff->mCamp == 1 && pSource->teamSide == pCharacter->teamSide)
                        return true;
                    else if (pEff->mCamp == 2 && pSource->teamSide != pCharacter->teamSide)
                        return true;
                }
            }
        }

        return false;
    }

    AIAttackIgnoreNoDieEffect::AIAttackIgnoreNoDieEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		:AIInnerEffect(pParent, pTarget, pExec)
	{
	}

    AIAttackIgnoreNoDieEffect::~AIAttackIgnoreNoDieEffect()
	{
		// onExit();
	}

    bool AIAttackIgnoreNoDieEffect::onEnter()
	{
		if (isDebuffed())
			return false;

        checkCleanSameTypeExec();

		mCamp = innerExec->targetAttribute;// 目标属性 -> 指定阵营
		mCharacterID =  innerExec->valueBase; // 数值计算基准-> 指定角色ID
		mSkillID = innerExec->changeValue;// 数值大小 -> 指定技能ID

        aiContext.addNoDieEffCharacter(target->myID);

		return true;
	}

    void AIAttackIgnoreNoDieEffect::onExit()
	{
        if (!isValid)
            return;
        aiContext.removeNoDieEffCharacter(target->myID);
    }

	// clair_wang 新增效果：目标的攻击序列中非普攻技能之前多加一个技能。
    std::vector<int> *AIAttackListAddSomeOneEffect::getAlternateAttackSeq(CharacterData *target, bool useSeq2)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_ATTACKLIST_ADD_ONE);
        if (iter == target->m_aiEffectMap.end())
            return nullptr;

        if (iter->second.empty())
            return nullptr;

        for (auto inneriter = iter->second.rbegin(); inneriter != iter->second.rend(); ++inneriter)
        {
            auto pEff = (AIAttackListAddSomeOneEffect *)(*inneriter);
            if (pEff && !pEff->isTempInvalid())
                return useSeq2 ? &pEff->_attackSeq2 : &pEff->_attackSeq;
        }

        // auto pEff = (AIAttackListAddSomeOneEffect *)iter->second.back();
        // return useSeq2 ? &pEff->_attackSeq2 : &pEff->_attackSeq;
        return nullptr;
    }

	AIAttackListAddSomeOneEffect::AIAttackListAddSomeOneEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		:AIInnerEffect(pParent, pTarget, pExec)
	{
	}

	AIAttackListAddSomeOneEffect::~AIAttackListAddSomeOneEffect()
	{
		// onExit();
	}

	bool AIAttackListAddSomeOneEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int insertIndex = innerExec->targetAttribute;

        auto insertFunc = [insertIndex](const std::vector<int> &originSeq, std::vector<int> &targetSeq, std::vector<int> &newMapOld, std::vector<int> &oldMapNew){
            for (int i = 0; i < originSeq.size(); i++)
            {
                if (originSeq[i] > 0)
                {
                    targetSeq.push_back(insertIndex);
                    newMapOld.push_back(i);
                }
                targetSeq.push_back(originSeq[i]);
                newMapOld.push_back(i);
                oldMapNew.push_back(newMapOld.size() - 1);
            }
        };

        auto pSeq1 = &target->staticData->attackSeq;
        auto pSeq2 = &target->staticData->attackSeq2;

        auto iter = target->m_aiEffectMap.find(eEFFECT_ATTACKLIST_ADD_ONE);
        if (iter != target->m_aiEffectMap.end() && iter->second.size() > 0)
        {
            auto pLastEff = (AIAttackListAddSomeOneEffect *)iter->second.back();
            pSeq1 = &pLastEff->_attackSeq;
            pSeq2 = &pLastEff->_attackSeq2;
        }

        insertFunc(*pSeq1, _attackSeq, _newMapOldIndex1, _oldMapNewIndex1);
        insertFunc(*pSeq2, _attackSeq2, _newMapOldIndex2, _oldMapNewIndex2);

        auto index1 = HeroActionController::getSkillSeq(target->actionAtt);
        if (index1 >= 0 && index1 < _oldMapNewIndex1.size())
            HeroActionController::setSkillSeq(target->actionAtt, _oldMapNewIndex1[index1]);

        auto index2 = target->m_iSkillSeq2Index;
        if (index2 >= 0 && index2 < _oldMapNewIndex2.size())
            target->m_iSkillSeq2Index = _oldMapNewIndex2[index2];

		return true;
	}

	void AIAttackListAddSomeOneEffect::onExit()
	{
        if (!isValid)
            return;
        auto pTarget = getTargetSafe();
        if (!pTarget)
            return;

        auto index1 = HeroActionController::getSkillSeq(pTarget->actionAtt);
        if (index1 >= 0 && index1 < _newMapOldIndex1.size())
            HeroActionController::setSkillSeq(pTarget->actionAtt, _newMapOldIndex1[index1]);

        auto index2 = pTarget->m_iSkillSeq2Index;
        if (index2 >= 0 && index2 < _newMapOldIndex2.size())
            pTarget->m_iSkillSeq2Index = _newMapOldIndex2[index2];
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                 AICreateHurtAddPureValueEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AICreateHurtAddPureValueEffect::AICreateHurtAddPureValueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		:AIInnerEffect(pParent, pTarget, pExec)
	{}

	AICreateHurtAddPureValueEffect::~AICreateHurtAddPureValueEffect()
	{
		// onExit();
	}

	bool AICreateHurtAddPureValueEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		float deltaComp = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;
		if (innerExec->changeType == 1)
		{
			deltaComp = deltaComp / 100.0f * target->getHPTop();
		}

		mAddHurtPercent = innerExec->limitOnOff / 10000.0f;

		limitUpper = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
		limitLower = innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel;

		if (target->lastHurtExecRealHurtDelta <= deltaComp)
		{
			_changeValue = mAddHurtPercent * target->lastHurtExecInitHurtDelta;

			if (_changeValue > limitUpper)
			{
				_changeValue = limitUpper;
			}
			if (_changeValue < limitLower)
			{
				_changeValue = limitLower;
			}

			_changeValue = EffectHelper::applyHPChange(aiContext, origin, projectile, target, skillID, skillLevel, ECharacterAttr::HP, _changeValue, innerExec, parentCaster->markID, parentCaster->effectID, this);

			return true;
		}

		return false;
	}

	void AICreateHurtAddPureValueEffect::onExit()
	{}


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEscMissExcuteEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void AIEscMissExcuteEffect::checkActivateEscMissExcute(CharacterData *pTarget, int effectID) {
        if (effectID != eEFFECT_MISS_EXCUTE && effectID != eEFFECT_ESCAPE_PHY_EXCUTE && effectID != eEFFECT_ESCAPE_MAG_EXCUTE)
            return;

        auto iter = pTarget->m_aiEffectMap.find(effectID);
        if (iter == pTarget->m_aiEffectMap.end())
            return;
        if (iter->second.size() < 1)
            return;
        for (auto inneriter = iter->second.begin(); inneriter != iter->second.end(); ++inneriter)
        {
            auto pEff = *inneriter;//iter->second[0];
            if (!pEff || pEff->isTempInvalid()) continue;
            AIEscMissExcuteEffect *pExcuteEff = (AIEscMissExcuteEffect *)pEff;
            if (!pExcuteEff->isActivated || pExcuteEff->parentCaster->localTick >= pExcuteEff->resetParentTick)
            {
                pExcuteEff->isActivated = true;
                pExcuteEff->resetParentTick = pExcuteEff->parentCaster->localTick + pExcuteEff->excuteCD;
                if (pExcuteEff->isInActivating)
                    return;
                pExcuteEff->isInActivating = true;
                AIActionController::executeExec(pExcuteEff->aiContext, pExcuteEff->excuteID, pExcuteEff->skillID, pExcuteEff->skillLevel, pExcuteEff->origin, pExcuteEff->projectile, pExcuteEff->target, 0);
                pExcuteEff->isInActivating = false;
            }
            return;
        }
    }

    AIEscMissExcuteEffect::AIEscMissExcuteEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }

    AIEscMissExcuteEffect::~AIEscMissExcuteEffect() {
        // onExit();
    }

    bool AIEscMissExcuteEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //数值大小, 等级成长->触发后冷却时间（要乘以100）
        excuteCD = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100;

        //数值计算基准->执行ID
        excuteID = innerExec->valueBase;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIEscMissExcuteEffect::onExit() {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIFloatStateEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIFloatStateEffect::AIFloatStateEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec) {
    }

    AIFloatStateEffect::~AIFloatStateEffect() {
        // onExit();
    }

    bool AIFloatStateEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = (innerExec->changeValue + innerExec->levelup * skillLevel);

        int valueBase = innerExec->valueBase;
        if (innerExec->changeType == 1)//percentage
        {
            if (valueBase>0 && valueBase<ATT_NUM)
            {
                _changeValue = target->attributes[valueBase] * _changeValue / 100;
            }
        }

        if (innerExec->limitOnOff)
        {
            int limitUpper = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
            int limitLower = innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel;

            if (_changeValue > limitUpper)
                _changeValue = limitUpper;
            if (_changeValue < limitLower)
                _changeValue = limitLower;
        }

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIFloatStateEffect::onExit() {

    }

    // MARK: deprecated in 8.0
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    //                                                            AIAttrConvertAddEffect
//    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    AIAttrConvertAddEffect::AIAttrConvertAddEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
//        :AIInnerEffect(pParent, pTarget, pExec) {
//    }
//
//    AIAttrConvertAddEffect:: ~AIAttrConvertAddEffect() {
//       onExit();
//    }
//
//    bool AIAttrConvertAddEffect::onEnter() {
//        if (isDebuffed())
//            return false;
//
//        checkCleanSameTypeExec();
//
//        int attrIndex = innerExec->targetAttribute;
//        if (attrIndex < ECharacterAttr::PHY || attrIndex > ECharacterAttr::SPIRIT)
//            return false;
//
//        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);
//        if (!_changeValue)
//            return false;
//
//        int oldhp = target->getHP();
//
//        int attrValue = target->attributes[attrIndex];
//        int oldAttrValue = attrValue;
//        attrValue += _changeValue;
//        if (attrValue < 0) {
//            attrValue = 0;
//            _changeValue = -oldAttrValue;
//        }
//        target->attributes[attrIndex] = attrValue;
//
//#if COCOS2D_DEBUG
//        CCLOG("enter AttrConvertAddEffect(%d), index:%d, old value: %d, new value: %d", _uid, attrIndex, oldAttrValue, attrValue);
//#endif
//
//        EffectHelper::applyConvertAttrChange(aiContext, target, attrIndex, _changeValue);
//
//        AIContinuousEffectCaster::startEffectCD(*target, innerExec);
//
//        return true;
//    }
//
//    void AIAttrConvertAddEffect::onExit() {
//        if (!isValid)
//            return;
//        auto pSafeTarget = getTargetSafe();
//        if (innerExec->exitClear != 1 || !pSafeTarget)
//            return;
//
//        int attrIndex = innerExec->targetAttribute;
//        target->attributes[attrIndex] -= _changeValue;
//
//#if COCOS2D_DEBUG
//        CCLOG("exit AttrConvertAddEffect(%d), new value:%d", _uid, target->attributes[attrIndex]);
//#endif
//        
//        EffectHelper::applyConvertAttrChange(aiContext, target, attrIndex, -_changeValue);
//    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIAddHurtOnSameTargetEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int AIAddHurtOnSameTargetEffect::checkExcute(CharacterData *pSource, CharacterData *pTarget, int hpdelta, int hurtType) {
        if (!pSource || !pTarget)
            return 0;
        auto iter = pSource->m_aiEffectMap.find(eEFFECT_ADD_HURT_ON_SAME_TARGET);
        if (iter == pSource->m_aiEffectMap.end())
            return 0;
        int additionDelta = 0;
        for (auto pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            AIAddHurtOnSameTargetEffect *eff = (AIAddHurtOnSameTargetEffect *)pEff;
            eff->onHurtTarget(pTarget);
            if (eff->sameCount > 1) {
                // TODO: 判断伤害类型-物理法术治疗
                if (eff->innerExec->changeType == 1)
                    additionDelta += eff->getChangeValue() * eff->sameCount * hpdelta / 100;
                else
                    additionDelta += eff->getChangeValue() * eff->sameCount;

                if (additionDelta > 0)
                    additionDelta = eff->getBoundedChangeValue(additionDelta, eff->innerExec, eff->skillLevel);
            }
        }
        return additionDelta;
    }

    AIAddHurtOnSameTargetEffect::AIAddHurtOnSameTargetEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AIAddHurtOnSameTargetEffect::~AIAddHurtOnSameTargetEffect()
    {
        // onExit();
    }

    bool AIAddHurtOnSameTargetEffect::onEnter() {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();
        //_changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        _changeValue = caculateChangeValue(true);

        return true;
    }

    void AIAddHurtOnSameTargetEffect::onExit() {
    }

    void AIAddHurtOnSameTargetEffect::onHurtTarget(CharacterData *aTarget) {
        if (lastTarget && aTarget && lastTarget->myID == aTarget->myID) {
            sameCount += 1;
        }
        else {
            lastTarget = aTarget;
            sameCount = 0;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIExcuteOnAttrChangeEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void AIExcuteOnAttrChangeEffect::checkExcute(CharacterData *source, int attr)
    {
        auto iter = source->m_aiEffectMap.find(eEFFECT_EXCUTE_ON_ATTR);
        if (iter == source->m_aiEffectMap.end())
            return;
        for (auto pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto eff = (AIExcuteOnAttrChangeEffect *)pEff;
            eff->excute(attr);
        }
    }

    AIExcuteOnAttrChangeEffect::AIExcuteOnAttrChangeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIExcuteOnAttrChangeEffect::~AIExcuteOnAttrChangeEffect()
    {
        // onExit();
    }

    bool AIExcuteOnAttrChangeEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();
        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        excute(innerExec->targetAttribute);

        return true;
    }

    void AIExcuteOnAttrChangeEffect::onExit()
    {

    }

    void AIExcuteOnAttrChangeEffect::excute(int attr)
    {
        if (innerExec->targetAttribute != attr)
            return;

        bool meetCondition = false;
        int attrVal = target->attributes[attr];
        int compVal = getChangeValue();
        if (attr == ECharacterAttr::HP && innerExec->changeType == 1)
        {
            compVal = (float)target->getHPTop() * ((float)_originChangeValue / 100.0f);
        }
        switch (innerExec->valueBase) {
        case 0: // 等于
            meetCondition = (attrVal == compVal);
            break;
        case 1: // 小于
            meetCondition = (attrVal < compVal);
            break;
        case 2: // 大于
            meetCondition = (attrVal > compVal);
            break;
        default:break;
        }

        if (!meetCondition)
            return;

        if (nextActiveParentTick > parentCaster->localTick)
            return;

        int execID = innerExec->params[0];
        int excuteCD = innerExec->params[1];
        nextActiveParentTick = parentCaster->localTick + excuteCD;

        ExecExcuteInfo info(execID, skillID, skillLevel, origin, projectile, target, 0);
        aiContext.scheduleExcuteExec(info);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIExcuteOnAttrChangeEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool AICritOnAttrEffect::checkExcute(CharacterData *source, CharacterData *target)
    {
        auto iter = source->m_aiEffectMap.find(eEFFECT_CRIT_ON_ATTR);
        if (iter == source->m_aiEffectMap.end())
            return false;
        for (auto &&pEff : iter->second) {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto attr = pEff->innerExec->targetAttribute;

            bool meetCondition = false;
            auto attrVal = target->attributes[attr];
            if (pEff->innerExec->changeType == 1) {
                if (attr == ECharacterAttr::HP)
                    attrVal = CharacterFunc::getSimplifiedHPRatio(target);
                else if (attr == ECharacterAttr::ENG)
                    attrVal = target->getEnergy() / 10;
            }
            auto compVal = pEff->getChangeValue();
            switch (pEff->innerExec->valueBase) {
            case 0: // 等于
                meetCondition = (attrVal == compVal);
                break;
            case 1: // 小于
                meetCondition = (attrVal < compVal);
                break;
            case 2: // 大于
                meetCondition = (attrVal > compVal);
                break;
            default:break;
            }

            if (meetCondition)
                return true;
        }

        return false;
    }

    AICritOnAttrEffect::AICritOnAttrEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AICritOnAttrEffect::~AICritOnAttrEffect()
    {
        // onExit();
    }

    bool AICritOnAttrEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();
        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        AIContinuousEffectCaster::startEffectCD(*target, innerExec);
        return true;
    }

    void AICritOnAttrEffect::onExit()
    {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIDebufClearBodyExecEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    bool AIDebufClearBodyExecEffect::checkExcute(CharacterData *pTarget, int bodyExecID)
    {
        auto iter = pTarget->m_aiEffectMap.find(eEFFECT_DEBUF_CLEAR_BODY_EXEC);
        if (iter == pTarget->m_aiEffectMap.end())
            return false;

        for (auto && pEff : iter->second)
        {
            if (pEff->innerExec->targetAttribute == bodyExecID)
                return true;
        }

        return false;
    }

    AIDebufClearBodyExecEffect::AIDebufClearBodyExecEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIDebufClearBodyExecEffect::~AIDebufClearBodyExecEffect()
    {
        // onExit();
    }

    bool AIDebufClearBodyExecEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        int theBodyEffID = innerExec->targetAttribute;

        for (auto && iter : target->m_aiEffectMap)
        {
            for (auto && pEff : iter.second)
            {
                if (!pEff || pEff->isTempInvalid()) continue;
                for (auto && bodyExecID : pEff->innerExec->bodyEffects)
                {
                    if (theBodyEffID == bodyExecID)
                    {
                        if (aiContext.bodyEffectProxy)
                            aiContext.bodyEffectProxy->notify(aiContext, pEff->getUID());
                    }
                }
            }
        }

        return true;
    }

    void AIDebufClearBodyExecEffect::onExit()
    {
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIExcuteOnHurtResultEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIExcuteOnHurtResultEffect::AIExcuteOnHurtResultEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIExcuteOnHurtResultEffect::~AIExcuteOnHurtResultEffect()
    {
        // onExit();
    }

    bool AIExcuteOnHurtResultEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //数值计算基础：触发间隔
        //目标属性：指定状态
        //数值大小：执行ID（不用乘100）
        excuteInterval = innerExec->valueBase;
        targetActionType = innerExec->targetAttribute;
        //if (excuteInterval > 0)
        //    parentCaster->scheduleInnerEff(parentCaster->localTick + excuteInterval, this);
        
        checkActivate();

        return true;
    }

    void AIExcuteOnHurtResultEffect::onExit()
    {

    }

    bool AIExcuteOnHurtResultEffect::isActionMatch()
    {
        // OP2 2.4
        // 999判断是否压制状态
        if (targetActionType != 999)
        {
            // 防止处于压制状态误触发老效果
            if (target->hasTag("besuppressed"))
                return false;
            return (targetActionType == target->currentAction);
        }
        else
            return target->hasTag("besuppressed");
        return false;
    }

    void AIExcuteOnHurtResultEffect::onScheduleTick()
    {
        checkActivate();

        if (isStartInterval && excuteInterval > 0)
        {
            //if (targetActionType == target->currentAction)
            if (isActionMatch())
                parentCaster->appendScheduleInnerEff(parentCaster->localTick + excuteInterval, this);
            else
                onEnd();
        }
    }

    void AIExcuteOnHurtResultEffect::checkActivate()
    {
        //if (targetActionType != target->currentAction)
        if (!isActionMatch())
            return;

        if (lastExcuteParentTick >= 0 && parentCaster->localTick == lastExcuteParentTick)
            return;

        lastExcuteParentTick = parentCaster->localTick;
        int theExecID = innerExec->changeValue;
        ExecExcuteInfo execInfo(theExecID, skillID, skillLevel, origin, projectile, target, 0);
        aiContext.scheduleExcuteExec(execInfo);
    }

    void AIExcuteOnHurtResultEffect::onStart()
    {
        if (isStartInterval)
            return;

        isStartInterval = true;
        if (excuteInterval > 0)
            parentCaster->appendScheduleInnerEff(parentCaster->localTick + excuteInterval, this);
    }

    void AIExcuteOnHurtResultEffect::onEnd()
    {
        isStartInterval = false;
        parentCaster->unscheduleInnerEff(getUID());
    }

    void AIExcuteOnHurtResultEffect::checkExcute(CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_ON_HURT_RESULT);
        if (iter == target->m_aiEffectMap.end())
            return;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AIExcuteOnHurtResultEffect *)(pEff);
            theEff->checkActivate();

            //if (theEff->targetActionType == target->currentAction)
            if (theEff->isActionMatch())
            {
                theEff->onStart();
            }
        }
    }
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AISpecifySexConditionResultEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang 若触发时我方（或者敌方）存活的角色中，性别A与性别B的人数满足指定关系，则触发执行，否则跳过。
	AISpecifySexConditionResultEffect::AISpecifySexConditionResultEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AISpecifySexConditionResultEffect::~AISpecifySexConditionResultEffect()
	{
		// onExit();
	}

	bool AISpecifySexConditionResultEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();
		
		TeamSide _teamSide;
		int _SexA, _SexB;
		int _execId = 0;
		int _rType = 0;
		bool canExec = false;
		int _NumSexA = 0, _NumSexB = 0;
		int _clusterNum = 0;
		bool _includeCharmed = false;
		TeamSide _reallyTeamSide;

		_teamSide = (TeamSide)innerExec->limitOnOff; // 上下限开关 // 阵营，默认0（1：我方  2：敌方）
		_SexA = innerExec->limitValLowerBase; // 数值下限基础值 // 性别（0-男，1-女，2-人妖）
		_SexB = innerExec->limitValLowerGrowth;// 下限成长 // 性别（0-男，1-女，2-人妖）
		_rType = innerExec->limitValUpperBase; // 数值上限基础值 //  关系类型，默认0（＜:1，≤:2，=:3，≥:4，＞:5）
		_execId = innerExec->limitValUpperGrowth; // 上限成长 // 执行id
		_includeCharmed = (bool)innerExec->levelup; // 等级成长   // 被魅惑的人是否算数  0：不算  1：算（算到对方头上）


		
		if (_teamSide == TeamSide::kOur)
		{
			_reallyTeamSide = target->teamSide;
		}
		else
		{
			_reallyTeamSide = (target->teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur);
		}

		_clusterNum = (_reallyTeamSide == TeamSide::kOur ? aiContext.current_ClusterNumOur : aiContext.current_ClusterNumEnemy);

		auto allTargets = aiContext.getClusterOrigin(_reallyTeamSide, _clusterNum);
		for (auto iter = allTargets->begin(); iter != allTargets->end(); ++iter)
		{
			auto role = (*iter);
			if (!HeroActionController::isDeadAction(role->currentAction) && !role->isSummon() && !role->isInCharmEffect() && !role->isInHalfCharmEffect())
			{
				if (role->getSex() == _SexA)
				{
					_NumSexA++;
				}
				else if (role->getSex() == _SexB)
				{
					_NumSexB++;
				}
			}
		}

		TeamSide _charmTeamSide = (_reallyTeamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur);
		int _charmClusterNum = (_charmTeamSide == TeamSide::kOur ? aiContext.current_ClusterNumOur : aiContext.current_ClusterNumEnemy);
		auto charmTargets = aiContext.getClusterOrigin(_charmTeamSide, _charmClusterNum);
		for (auto itt = charmTargets->begin(); itt != charmTargets->end(); ++itt)
		{
			auto roleChar = (*itt);
			if (_includeCharmed && (roleChar->isInCharmEffect() || roleChar->isInHalfCharmEffect()))
			{
				if (roleChar->getSex() == _SexA)
				{
					_NumSexA++;
				}
				else if (roleChar->getSex() == _SexB)
				{
					_NumSexB++;
				}
			}
		}

		switch (_rType)
		{
		case e_OperateT_LessThan:// 小于 ＜ : 1
			canExec = (_NumSexA < _NumSexB);
			break;
		case e_OperateT_LessEqual:// 小于等于 ≤ : 2
			canExec = (_NumSexA <= _NumSexB);
			break;
		case e_OperateT_Equal:// 等于 = : 3
			canExec = (_NumSexA == _NumSexB);
			break;
		case e_OperateT_GreaterEqual:// 大于等于 ≥ : 4
			canExec = (_NumSexA >= _NumSexB);
			break;
		case e_OperateT_GreaterThan:// 大于 ＞ : 5
			canExec = (_NumSexA > _NumSexB);
			break;
		default:
			break;
		}

		if (canExec && _execId > 0)
		{
			ExecExcuteInfo info(_execId, skillID, skillLevel, origin, projectile, target, 0);
			aiContext.scheduleExcuteExec(info);
		}
		return true;
	}

	void AISpecifySexConditionResultEffect::onExit()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIChangeSexEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang // 在一段时间内改变性别
	AIChangeSexEffect::AIChangeSexEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIChangeSexEffect::~AIChangeSexEffect()
	{
		// onExit();
	}

	bool AIChangeSexEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int theSex = innerExec->limitOnOff; // 上下限开关 // 性别（0-男，1-女，2-人妖）

        target->setTempSex(theSex);

		return true;
	}

	void AIChangeSexEffect::onExit()
	{
		if (!isValid)
			return;

		auto pTarget = getTargetSafe();

		if (!pTarget)
			return;

        pTarget->setTempSex(-1);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIAccordanceSexExectueEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang // 根据指定性别调用不同执行（主要是动画执行）。
	AIAccordanceSexExectueEffect::AIAccordanceSexExectueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIAccordanceSexExectueEffect::~AIAccordanceSexExectueEffect()
	{
		// onExit();
	}

	bool AIAccordanceSexExectueEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int _nSex = innerExec->changeValue;
		int _nExecID = innerExec->levelup;

		if (target->getSex() == _nSex && _nExecID > 0)
		{
			ExecExcuteInfo info(_nExecID, skillID, skillLevel, origin, projectile, target, 0);
			aiContext.scheduleExcuteExec(info);
		}

		return true;
	}

	void AIAccordanceSexExectueEffect::onExit()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIPrimayAttributeConditionExectueEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang // 若触发时主属性A与其他两个主属性BC的关系满足条件，则触发执行id
	AIPrimayAttributeConditionExectueEffect::AIPrimayAttributeConditionExectueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIPrimayAttributeConditionExectueEffect::~AIPrimayAttributeConditionExectueEffect()
	{
		// onExit();
	}

	bool AIPrimayAttributeConditionExectueEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int _nAttrAIdx, _nAttrBIdx, _nAttrCIdx;
		int _nAttrA = 0, _nAttrB = 0, _nAttrC = 0;
		int _nExecID = 0;
		int _nOperAB = 0, _nOperAC = 0;
		bool _bFlagAB = false, _bFlagAC = false;

		_nAttrAIdx = innerExec->changeValue; // 数值大小 --》 属性A
		_nExecID = innerExec->levelup; // 等级成长 - 》执行id //直接填真正的执行id，不要乘以100
		_nAttrBIdx = innerExec->limitOnOff; // 上下限开关 --》 属性B
		_nAttrCIdx = innerExec->limitValLowerBase; // 数值下限基数 - 》属性C
		
		_nOperAB = innerExec->limitValLowerGrowth; // 下限成长 - 》A与B关系类型，默认0（＜:1，≤ : 2， = : 3，≥ : 4，＞ : 5）
		_nOperAC = innerExec->limitValUpperBase; // 数值上限基数 - 》 A与C关系类型，默认0（＜ : 1，≤ : 2， = : 3，≥ : 4，＞ : 5）.


		if (_nAttrAIdx >= ECharacterAttr::PHY && _nAttrAIdx <= ECharacterAttr::SPIRIT)
		{
			_nAttrA = target->attributes[_nAttrAIdx];
		}

		if (_nAttrBIdx >= ECharacterAttr::PHY && _nAttrBIdx <= ECharacterAttr::SPIRIT)
		{
			_nAttrB = target->attributes[_nAttrBIdx];
		}

		if (_nAttrCIdx >= ECharacterAttr::PHY && _nAttrCIdx <= ECharacterAttr::SPIRIT)
		{
			_nAttrC = target->attributes[_nAttrCIdx];
		}

		switch (_nOperAB)
		{
		case e_OperateT_LessThan:// 小于 ＜ : 1
			_bFlagAB = (_nAttrA < _nAttrB);
			break;
		case e_OperateT_LessEqual:// 小于等于 ≤ : 2
			_bFlagAB = (_nAttrA <= _nAttrB);
			break;
		case e_OperateT_Equal:// 等于 = : 3
			_bFlagAB = (_nAttrA == _nAttrB);
			break;
		case e_OperateT_GreaterEqual:// 大于等于 ≥ : 4
			_bFlagAB = (_nAttrA >= _nAttrB);
			break;
		case e_OperateT_GreaterThan:// 大于 ＞ : 5
			_bFlagAB = (_nAttrA > _nAttrB);
			break;
		default:
			break;
		}

		if (_bFlagAB)
		{
			switch (_nOperAC)
			{
			case e_OperateT_LessThan:// 小于 ＜ : 1
				_bFlagAC = (_nAttrA < _nAttrC);
				break;
			case e_OperateT_LessEqual:// 小于等于 ≤ : 2
				_bFlagAC = (_nAttrA <= _nAttrC);
				break;
			case e_OperateT_Equal:// 等于 = : 3
				_bFlagAC = (_nAttrA == _nAttrC);
				break;
			case e_OperateT_GreaterEqual:// 大于等于 ≥ : 4
				_bFlagAC = (_nAttrA >= _nAttrC);
				break;
			case e_OperateT_GreaterThan:// 大于 ＞ : 5
				_bFlagAC = (_nAttrA > _nAttrC);
				break;
			default:
				break;
			}
		}

		if (_bFlagAB && _bFlagAC && _nExecID > 0)
		{
			ExecExcuteInfo info(_nExecID, skillID, skillLevel, origin, projectile, target, 0);
			aiContext.scheduleExcuteExec(info);
		}

		return true;
	}

	void AIPrimayAttributeConditionExectueEffect::onExit()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIExecuteJudgeSummonCountEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang // 拥有效果时（瞬时），判断自己的召唤物数量。
	AIExecuteJudgeSummonCountEffect::AIExecuteJudgeSummonCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIExecuteJudgeSummonCountEffect::~AIExecuteJudgeSummonCountEffect()
	{
		// onExit();
	}

	bool AIExecuteJudgeSummonCountEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		bool canExec = false;
		int summonCount = 0;
		int oper = innerExec->limitOnOff; // 上下限开关 --》 关系类型，默认0（＜ : 1，≤ : 2， = : 3，≥ : 4，＞ : 5）
		int count = innerExec->limitValLowerBase; // 数值下限基数 - 》数量1
		int execID = innerExec->limitValLowerGrowth; // 下限成长 - 》触发的执行ID
		int count2 = innerExec->limitValUpperBase; // 数量2 // 阈值内触发时才需要填写 
		
		for (CharacterDataList::iterator ite = aiContext.m_summonCharacterList.begin(); ite != aiContext.m_summonCharacterList.end(); ++ite)
		{
			auto role = (*ite);
			if (role->isSummon() && role->getFakeSummonType() == 0 /*12.0*/ && role->getSummonSource()->myID == target->myID && (!HeroActionController::isDeadAction(role->currentAction)) && role->isSummonActive)
			{
				summonCount++;
			}
		}

		switch (oper)
		{
		case e_OperateT_LessThan:// 小于 ＜ : 1
			canExec = (summonCount < count);
			break;
		case e_OperateT_LessEqual:// 小于等于 ≤ : 2
			canExec = (summonCount <= count);
			break;
		case e_OperateT_Equal:// 等于 = : 3
			canExec = (summonCount == count);
			break;
		case e_OperateT_GreaterEqual:// 大于等于 ≥ : 4
			canExec = (summonCount >= count);
			break;
		case e_OperateT_GreaterThan:// 大于 ＞ : 5
			canExec = (summonCount > count);
			break;
		case e_OperateT_Threshold: // 大于等于且小于等于 [>= && <= ]：6
			canExec = ((summonCount >= count) && (summonCount <= count2));
			break;
		default:
			break;
		}


		if (canExec && execID > 0)
		{
			ExecExcuteInfo info(execID, skillID, skillLevel, origin, projectile, target, 0);
			aiContext.scheduleExcuteExec(info);
		}


		return true;
	}

	void AIExecuteJudgeSummonCountEffect::onExit()
	{

	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIReleaseCurAttrAndMaxAttrContionEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// clair_wang // 瞬间释放（多为伤害执行中的击中执行），目标当前属性与当前最大属性的比值与填写的百分比作比较，决定触发执行。
	AIReleaseCurAttrAndMaxAttrContionEffect::AIReleaseCurAttrAndMaxAttrContionEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIReleaseCurAttrAndMaxAttrContionEffect::~AIReleaseCurAttrAndMaxAttrContionEffect()
	{
		// onExit();
	}

	bool AIReleaseCurAttrAndMaxAttrContionEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int _nCurValue = 0, _nMaxValue = 0;
		float _fCurRatio = 0.0f, _fSetRatio = 0.0f, _fRatio2 = 0.0f;
		int _nType = innerExec->changeValue; // 数值大小 --》 属性(生命或能量）
		int _nExecID = innerExec->levelup; // 等级成长  -》执行id //直接填真正的执行id，不要乘以100
		int _nOper = innerExec->limitOnOff; // 上下限开关 --》 关系类型，默认0（＜:1，≤:2，=:3，≥:4，＞:5   [>= && <=]：6）
		bool _bCanExec = false;

		_fSetRatio = (float)innerExec->limitValLowerBase / 100.0f; // 数值下限基数-》比值1，默认0 // 要乘以100  eg:0.5就填50
		_fRatio2 = (float)innerExec->limitValLowerGrowth / 100.0f; // 下限成长      -》比值2，默认0 // 阈值内触发时才需要填写 // 要乘以100  eg:0.5就填50 

		if (_nType == ECharacterAttr::HP)
		{
			_nCurValue = target->getHP();
			_nMaxValue = target->getHPTop();
		}
		else if (_nType == ECharacterAttr::ENG)
		{
			_nCurValue = target->getEnergy();
			_nMaxValue = AIDataHelper::getEnergyRecover()[0];
		}
		
		if (_nMaxValue != 0)
		{
			_fCurRatio = (float)_nCurValue / (float)_nMaxValue;
		}

		switch (_nOper)
		{
		case e_OperateT_LessThan:// 小于 ＜ : 1
			_bCanExec = (_fCurRatio < _fSetRatio);
			break;
		case e_OperateT_LessEqual:// 小于等于 ≤ : 2
			_bCanExec = (_fCurRatio <= _fSetRatio);
			break;
		case e_OperateT_Equal:// 等于 = : 3
			_bCanExec = (_fCurRatio == _fSetRatio);
			break;
		case e_OperateT_GreaterEqual:// 大于等于 ≥ : 4
			_bCanExec = (_fCurRatio >= _fSetRatio);
			break;
		case e_OperateT_GreaterThan:// 大于 ＞ : 5
			_bCanExec = (_fCurRatio > _fSetRatio);
			break;
		case e_OperateT_Threshold: // 大于等于且小于等于 [>= && <= ]：6
			_bCanExec = ((_fCurRatio >= _fSetRatio) && (_fCurRatio <= _fRatio2));
			break;
		default:
			break;
		}

		if (_bCanExec && _nExecID > 0)
		{
			ExecExcuteInfo info(_nExecID, skillID, skillLevel, origin, projectile, target, 0);
			aiContext.scheduleExcuteExec(info);
		}

		return true;
	}

	void AIReleaseCurAttrAndMaxAttrContionEffect::onExit()
	{

	}
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AISkipSkillSeqEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int AISkipSkillSeqEffect::checkExcute(CharacterData *target, std::vector<int> &seq, int currentIndex)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SKIP_SKILL_SEQ);
        if (iter == target->m_aiEffectMap.end() || seq.size() <= 1)
            return currentIndex;

        //若该指定技能序号是目标技能序列里的唯一构成序号，则该效果无效

        std::map<int, bool> skipIndices;
        for (auto &&pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            for (auto theSeqIndex : pEff->innerExec->params)
                skipIndices[theSeqIndex] = true;
        }

        int seqIndex = currentIndex;
        if (seqIndex >= seq.size())
            seqIndex = 0;

        bool isFirstLoop = true;
        do {
            if (seqIndex >= seq.size())
            {
                if (isFirstLoop)
                {
                    seqIndex = 0;
                    isFirstLoop = false;
                }
                else
                    return currentIndex;
            }

            int theSkillIDIndex = seq[seqIndex];
            if (skipIndices.find(theSkillIDIndex) == skipIndices.end())
            {
                return seqIndex;
            }
            seqIndex++;
        } while (seqIndex != currentIndex);

        return currentIndex;
    }

    AISkipSkillSeqEffect::AISkipSkillSeqEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AISkipSkillSeqEffect::~AISkipSkillSeqEffect()
    {
        // onExit();
    }

    bool AISkipSkillSeqEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AISkipSkillSeqEffect::onExit()
    {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIBAMACritCastEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIBAMACritCastEffect::AIBAMACritCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIBAMACritCastEffect::~AIBAMACritCastEffect()
    {
        // onExit();
    }

    bool AIBAMACritCastEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

		vCastTypes.clear();

		for (auto eID : innerExec->params)
		{
			if (eID >= 0)
				vCastTypes.push_back(eID);
		}		

		if (target->lastHurtExecId > 0)
		{
			HurtExec* hurtExec = AIDataHelper::getHurtExec(target->lastHurtExecId);
			if (hurtExec && hurtExec->type)
			{
				return isMatchCondition(hurtExec->type);
			}
		}

        return true;
    }

    void AIBAMACritCastEffect::onExit()
    {

    }
	
	bool AIBAMACritCastEffect::isMatchCondition(int hurtType)
	{
		for (int i = 0; i < vCastTypes.size(); i++)
		{
			if (hurtType == vCastTypes[i])
			{
				return true;
			}
		}

		return false;
	}


/*    void AIBAMACritCastEffect::checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target)
    {
        auto iter = source->m_aiEffectMap.find(eEFFECT_BAMA_CRIT_NCAST);
        if (iter == source->m_aiEffectMap.end())
            return;

        for (auto &&pEff : iter->second)
        {
            auto theEff = std::static_pointer_cast<AIBAMACritCastEffect, AIInnerEffect>(pEff);
            const int currentParentLocalTick = theEff->parentCaster->localTick;
            auto foundIter = theEff->castTargetRecord.find(target->myID);
            if (foundIter != theEff->castTargetRecord.end())
            {
                auto lastCastTick = foundIter->second;
                if (lastCastTick == currentParentLocalTick || lastCastTick + theEff->castInterval >= currentParentLocalTick)
                {
                    continue;
                }
            }
			assert(theEff->castID <= 0);
            aiContext.scheduleExcute(theEff->castID, theEff->skillID, theEff->skillLevel, source, nullptr, target, 0);
            theEff->castTargetRecord[target->myID] = currentParentLocalTick;
        }
    }
	*/

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AISlipCastEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AISlipCastEffect::AISlipCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AISlipCastEffect::~AISlipCastEffect()
    {
        // onExit();
    }

    bool AISlipCastEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        // params: [a,b,c,d,e]
        // a-触发概率
        // b-是否无视120效果免晕免击倒
        // c-触发间隔
        // d-滑倒时间  -1不滑倒
        // e-执行id  触发时执行(不管有没有滑倒)

        castProb = innerExec->params[0];
        ignoreDebuff = innerExec->params[1];
        castInterval = innerExec->params[2];
        slipTickLen = innerExec->params[3];
        castID = innerExec->params[4];

        return true;
    }

    void AISlipCastEffect::onExit()
    {

    }

    AISlipCastEffect *AISlipCastEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SLIP_NCAST);
        if (iter == target->m_aiEffectMap.end())
            //return -1;
            return nullptr;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AISlipCastEffect *)(pEff);
            if (theEff->lastCastParentLocalTick < 0 || theEff->parentCaster->localTick > theEff->lastCastParentLocalTick + theEff->castInterval)
            {
                if (aiContext.localRandom->nextInt(0, 100) < theEff->castProb)
                {
                    if (theEff->ignoreDebuff || AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_HIT_DOWN_FLY_BACK, 0) == 0)
                    {
                        theEff->lastCastParentLocalTick = theEff->parentCaster->localTick;
                        aiContext.scheduleExcute(theEff->castID, theEff->skillID, theEff->skillLevel, theEff->origin, nullptr, target, 0);

                        //return theEff->slipTickLen;
                        return theEff;
                    }
                }
            }
        }

        //return -1;
        return nullptr;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AISkipCDCastEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AISkipCDCastEffect::AISkipCDCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AISkipCDCastEffect::~AISkipCDCastEffect()
    {
        // onExit();
    }

    bool AISkipCDCastEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        if (innerExec->params.size() > 0)
            castSkillID = innerExec->params[0];

        return true;
    }

    void AISkipCDCastEffect::onExit()
    {

    }

    int AISkipCDCastEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SKIP_SKILLCD_NCAST);
        if (iter == target->m_aiEffectMap.end())
            return 0;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AISkipCDCastEffect *)(pEff);
            if (theEff->getChangeValue() > 0 && theEff->castSkillID > 0)
            {
                return theEff->castSkillID;
            }
        }

        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AISacrificeHurtRecv
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AISacrificeHurtRecv::AISacrificeHurtRecv(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AISacrificeHurtRecv::~AISacrificeHurtRecv()
    {
        // onExit();
    }

    bool AISacrificeHurtRecv::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //_changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        sacrificeID = innerExec->targetAttribute;
        //supportHurtType = innerExec->hurtType;
        for (int i = 0; i < innerExec->params.size(); i++)
        {
            int value = innerExec->params[i];
            if (i < 3)
            {
                // 效果执行对应的伤害类型要-1
                if (value == 1)
                {
                    if (i == 0)
                        supportHurtTypes[-1] = true;
                    else if (i == 1)
                        supportHurtTypes[PHYSICAL_HURT_TYPE] = true;
                    else if (i == 2)
                        supportHurtTypes[MAGICAL_HURT_TYPE] = true;
                }
            }
            else
            {
                supportHurtPropTypes[value] = true;
            }
        }

        return true;
    }

    void AISacrificeHurtRecv::onExit()
    {

    }

    bool AISacrificeHurtRecv::canRecvHurt(int theSendOriginMyID, int theSendSacrificeID, int hurtType, int hurtProperty)
    {
        if (theSendOriginMyID != this->_originMyID)
            return false;
        if (this->sacrificeID == -1)
            return true;
        if (this->sacrificeID == 0)
            return false;
        if (this->sacrificeID != theSendSacrificeID)
            return false;
        if (this->supportHurtTypes.find(hurtType) == this->supportHurtTypes.end())
            return false;
        if (this->supportHurtPropTypes.find(hurtProperty) == this->supportHurtPropTypes.end())
            return false;
        return true;
    }

    bool AISacrificeHurtRecv::checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &hurtParam, int theSendOriginMyID, int theSendSacrificeID)
    {
        auto theRecvIter = target->m_aiEffectMap.find(eEFFECT_SACRIFICE_HURT_RECV);
        if (theRecvIter == target->m_aiEffectMap.end())
            return false;
        
        for (auto pRecvEff : theRecvIter->second)
        {
            if (!pRecvEff || pRecvEff->isTempInvalid()) continue;
            auto theRecvEff = (AISacrificeHurtRecv *)(pRecvEff);
            if (theRecvEff->canRecvHurt(theSendOriginMyID, theSendSacrificeID, hurtParam.hurtType, hurtParam.hurtProperty))
            {
//                auto result = HeroHurtcalculate::HurtFuncHurtTargetScript(aiContext, source, target, hurtParam);
                HitResult result;
                HeroHurtcalculate::HurtFuncHurtTarget(aiContext, source, target, hurtParam, &result);
                if (result.blockResult != HitResult::BlockType::HIT_BLOCKED)
                    target->onHurt(true, 0, hurtParam.HPDelta, hurtParam.hurtType, source, hurtParam.casterID);

                if (target->getHP() <= 0)
                {
                    HeroHurtcalculate::characterDeadByHurt(aiContext, target, source, hurtParam.skillID, hurtParam.skillLevel, nullptr, nullptr);
                }
                else
                {
                    // 牺牲效果不触发标记
                   /* if (hurtParam.hurtProperty > 0)
                    {
						
                        HeroHurtcalculate::ToTriggerMarkCasterFun(aiContext, *target, 0, hurtParam.skillLevel, hurtParam.hurtType, hurtParam.hurtProperty, hurtParam.HPDelta, hurtParam.hitType, source);
                    }*/
                    HeroSkillController::checkReleaseHPSkill(aiContext, target);
                }

                return true;
            }
        }
        
        return false;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AISacrificeHurtSend
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AISacrificeHurtSend::AISacrificeHurtSend(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AISacrificeHurtSend::~AISacrificeHurtSend()
    {
        // onExit();
    }

    bool AISacrificeHurtSend::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        sacrificeID = innerExec->targetAttribute;
        //supportHurtType = innerExec->hurtType;
        //for (int i = 0; i < innerExec->params.size(); i++)
        //    supportHurtPropTypes[innerExec->params[i]] = true;

        return true;
    }

    void AISacrificeHurtSend::onExit()
    {

    }

    //bool AISacrificeHurtSend::canSendHurt(int hurtType, int hurtProperty)
    //{
    //    if (supportHurtType - 1 != hurtType)
    //        return false;
    //    if (supportHurtPropTypes.find(hurtProperty) == supportHurtPropTypes.end())
    //        return false;
    //    return true;
    //}

    int AISacrificeHurtSend::getSendHurtDelta(HurtCalcParam &hurtParam)
    {
        //if (!canSendHurt(hurtParam.hurtType, hurtParam.hurtProperty))
        //    return 0;

        if (innerExec->changeType == 1)
        {
            int sendValue = _changeValue * hurtParam.HPDelta / 100;
            if (innerExec->limitOnOff)
            {
                int limit = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
                limit /= 100;

                if (sendValue > limit)
                    sendValue = limit;
            }
            return sendValue;
        }
        else
        {
            if (_changeValue <= hurtParam.HPDelta)
                return _changeValue;
            else
                return hurtParam.HPDelta;
        }

        return 0;
    }

    int AISacrificeHurtSend::checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &hurtParam)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SACRIFICE_HURT_SEND);
        if (iter == target->m_aiEffectMap.end())
            return hurtParam.HPDelta;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theSendEff = (AISacrificeHurtSend *)(pEff);
            int sendHurtValue = theSendEff->getSendHurtDelta(hurtParam);
            if (sendHurtValue)
            {
                HurtCalcParam sendHurtParam(hurtParam);
                sendHurtParam.HPDelta = sendHurtValue;
                sendHurtParam.isFromSacrificeRecv = true;
                sendHurtParam.ignoreHurtLabel = false;

                std::list<CharacterData* > characterList;
                auto friendList = aiContext.getClusterOrigin(target->teamSide, target->clusterNum);
                if (friendList)
                    characterList.assign(friendList->begin(), friendList->end());

                TeamSide theOtherSide = target->teamSide == TeamSide::kOur ? TeamSide::kEnemy : TeamSide::kOur;
                int theOtherClusterNum = target->teamSide == TeamSide::kOur ? aiContext.current_ClusterNumOur : aiContext.current_ClusterNumEnemy;
                auto rivalList = aiContext.getClusterOrigin(theOtherSide, theOtherClusterNum);
                if (rivalList)
                    characterList.insert(characterList.end(), rivalList->begin(), rivalList->end());

                for (auto theIter = characterList.begin(); theIter != characterList.end(); ++theIter)
                {
                    auto pCharacter = *theIter;
                    if (pCharacter->myID == target->myID)
                        continue;
                    bool isRecvSuccess = AISacrificeHurtRecv::checkExcute(aiContext, source, pCharacter, sendHurtParam, theSendEff->_originMyID, theSendEff->sacrificeID);
                    if (isRecvSuccess)
                        return (hurtParam.HPDelta - sendHurtValue);
                }
            }
        }

        return hurtParam.HPDelta;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIHpLostEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIHpLostEffect::AIHpLostEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AIHpLostEffect::~AIHpLostEffect()
    {
    }

    bool AIHpLostEffect::onEnter()
    {
        if (isDebuffed())
            return false;
        
        //时间、频率、物理、法术、纯粹、次数
        if (innerExec->params.size() < 6)
            return false;

        if (innerExec->params[1]) {
            stepCount = innerExec->params[0] / innerExec->params[1];
        }
        if (!stepCount)
            return false;

        auto iter = target->m_aiEffectMap.find(eEFFECT_HP_LOST);
        if (iter != target->m_aiEffectMap.end()) {
            for (auto pEff : iter->second) {
                auto theEff = (AIHpLostEffect*)(pEff);
                if (theEff->canTrigger()) 
                    return false;
            }
        }

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    bool AIHpLostEffect::canTrigger()
    {
        if (!isValid || isTempInvalid())
            return false;

        if (innerExec->params[5] && triggerCount >= innerExec->params[5])
            return false;

        if (parentCaster->totalTick - (parentCaster->localTick - parentCaster->startTick - 1) < innerExec->params[0])
            return false;

        return true;
    }

    int AIHpLostEffect::checkExcute(AIContext& aiContext, CharacterData* source, CharacterData* target, HurtCalcParam& hurtParam)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_HP_LOST);
        if (iter == target->m_aiEffectMap.end())
            return hurtParam.HPDelta;

        HurtCalcParam param(hurtParam);
        param.hurtProperty = HurtPropertyType::kNoProperty;
        param.isFromHpLost = true;
        param.innerEffect = nullptr;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AIHpLostEffect*)(pEff);
            if (!theEff->canTrigger()) continue;
            if ((param.hurtType == HurtType::PHYSICAL_HURT_TYPE && !theEff->innerExec->params[2])
                || (param.hurtType == HurtType::MAGICAL_HURT_TYPE && !theEff->innerExec->params[3])
                || (param.hurtType == -1 && !theEff->innerExec->params[4])
                || (param.hurtType == HurtType::RAW_HURT_TYPE && !theEff->innerExec->params[4]))
                continue;

            int lostHp = 0;
            if (theEff->innerExec->changeType == 1) {
                lostHp = theEff->_changeValue * hurtParam.HPDelta / 100;
            } else {
                lostHp = std::min(theEff->_changeValue, hurtParam.HPDelta);
            }
            if (lostHp > 0) {
                hurtParam.HPDelta -= lostHp;
                int stepLostHp = lostHp / theEff->stepCount;
                if (stepLostHp > 0) {
                    param.HPDelta = stepLostHp;
                    theEff->m_aiHurtMap[theEff->parentCaster->localTick].push_back(std::make_tuple(source, target, param));
                }
            }

            theEff->triggerCount++;
            break;
        }

        return hurtParam.HPDelta;
    }

    void AIHpLostEffect::checkLostExcute(AIContext& aiContext, CharacterData* target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_HP_LOST);
        if (iter == target->m_aiEffectMap.end())
            return ;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AIHpLostEffect*)(pEff);
            for (auto iter = theEff->m_aiHurtMap.begin(); iter != theEff->m_aiHurtMap.end(); ++iter)
            {
                int deltaTick = theEff->parentCaster->localTick - iter->first;
                int stepIndex = deltaTick / theEff->innerExec->params[1];
                if (deltaTick > 0 && !(deltaTick % theEff->innerExec->params[1]) && stepIndex <= theEff->stepCount) {
                    for (auto iiter = iter->second.begin(); iiter != iter->second.end(); ++iiter) {
                        auto& tup = *iiter;
                        CharacterData* theSource = std::get<0>(tup);
                        CharacterData* theTarget = std::get<1>(tup);
                        HurtCalcParam param = std::get<2>(tup);
                        if ((HeroActionController::isDeadAction(theTarget->currentAction) || theTarget->hasTag("deadattacking")))
                            return;

                        HitResult result;
                        HeroHurtcalculate::HurtFuncHurtTarget(aiContext, theSource, theTarget, param, &result);

                        //if (result.blockResult != HitResult::BlockType::HIT_BLOCKED)
                        //    theTarget->onHurt(true, theEff->innerExec->ID, param.HPDelta, param.hurtType, theSource, theEff->parentCaster->effectID);

                        if (theTarget->getHP() <= 0)
                        {
                            HeroHurtcalculate::characterDeadByHurt(aiContext, theTarget, theSource, param.skillID, param.skillLevel, param.hurtExec);
                        }
                        else
                        {
                            if (result.blockResult != HitResult::BlockType::HIT_BLOCKED && result.hitResult == HitResult::HitResultType::HIT_HASHURT)
                            {
                                HeroHurtcalculate::ToTriggerMarkCasterFun(aiContext, *theTarget, param.skillID, param.skillLevel, param.hurtType, param.hurtProperty, param.HPDelta, param.hitType, theSource);
                            }
                            HeroSkillController::checkReleaseHPSkill(aiContext, theTarget);
                        }
                        if ((HeroActionController::isDeadAction(theTarget->currentAction) || theTarget->hasTag("deadattacking"))) {
                            AIContinuousEffectCaster::zero_Special_Effect(aiContext, *theTarget, eEFFECT_HP_LOST);
                            return;
                        }
                            
                    }
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIHurtByDeadFriendEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIHurtByDeadFriendEffect::AIHurtByDeadFriendEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIHurtByDeadFriendEffect::~AIHurtByDeadFriendEffect()
    {
        // onExit();
    }

    bool AIHurtByDeadFriendEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //参数1：基础扣血百分比（填1代表0.01%）
        //参数2：每个死亡人头提供的额外扣血百分比（填1代表0.01%）
        //公式：扣除百分比 = 参数1 + 死亡人数 * 参数2
        if (innerExec->params.size() < 3)
            return false;

        float param1 = innerExec->params[1];
        float param2 = innerExec->params[2];

        int hpdelta = 0;
        int deadfriendcnt = 0;
        bool includesummon = innerExec->valueBase;
        auto theFriends = origin->getOriginFriends();
        for (auto iter = theFriends->begin(); iter != theFriends->end(); ++iter)
        {
            auto pCharacter = *iter;
            if (HeroActionController::isDeadAction(pCharacter->currentAction))
            {
                if (pCharacter->isSummon())
                {
                    if (!includesummon || !pCharacter->hasBeenSummoned)
                        continue;
                }

                deadfriendcnt++;
            }
        }

        float theRatio = (param1 + deadfriendcnt * param2) / 10000.0f;
        hpdelta = target->getHPTop() * theRatio;
        if (hpdelta == 0)
            return false;

        _changeValue = EffectHelper::applyHPChange(aiContext, origin, projectile, target, skillID, skillLevel, ECharacterAttr::HP, hpdelta, innerExec, parentCaster->markID, parentCaster->effectID, this);

        return true;
    }

    void AIHurtByDeadFriendEffect::onExit()
    {

    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIKOByCountEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIKOByCountEffect::AIKOByCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIKOByCountEffect::~AIKOByCountEffect()
    {
        // onExit();
    }

    bool AIKOByCountEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);

        return true;
    }

    void AIKOByCountEffect::onExit()
    {

    }

    void AIKOByCountEffect::checkExcute(AIContext &aiContext, CharacterData *target, int hurtType, int hurtvalue)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_KO_BY_COUNT);
        if (iter == target->m_aiEffectMap.end())
            return;

        for (auto pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AIKOByCountEffect *)(pEff);
            auto count = theEff->getChangeValue();
            if (count > 0)
            {
                count--;
                theEff->setChangeValue(count);

                continue;
            }

            bool isHurtTransfered = false;
            if (HeroHurtcalculate::HurtFuncCanTransferHurt(aiContext, target, target->getHP()))
            {
                HurtCalcParam param(theEff->projectile, theEff->skillID, theEff->skillLevel, hurtvalue, hurtType, theEff->parentCaster->effectID);
                //param.ignoreHurtLabel = true;
                param.isFromEffectCaster = true;

                isHurtTransfered = HeroHurtcalculate::HurtFuncTransferHurt(aiContext, theEff->origin, target, param);
            }

            if (!isHurtTransfered)
            {
                int oldHP = target->getHP();
                StatsFunc::AppendHeroHurt(*theEff->origin, hurtType >= 0 ? HurtType(hurtType) : HurtType::PHYSICAL_HURT_TYPE, target->getHP(), *target);
                target->updateHP(0);
                target->onHPDelta(target->getHP() - oldHP);
                HeroHurtcalculate::characterDeadByHurt(aiContext, target, theEff->origin, theEff->skillID, theEff->skillLevel, nullptr, theEff->innerExec);

                return;
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIResistFatalHurtNCastEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIResistFatalHurtNCastEffect::AIResistFatalHurtNCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIResistFatalHurtNCastEffect::~AIResistFatalHurtNCastEffect()
    {
        // onExit();
    }

    bool AIResistFatalHurtNCastEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AIResistFatalHurtNCastEffect::onExit()
    {

    }

    bool AIResistFatalHurtNCastEffect::canCast(AIContext &aiContext, CharacterData *target, int hpdelta)
    {
        if (hpdelta < target->getHP())
            return false;

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, eEFFECT_RESIST_FATAL_HURT_NCAST, 0) <= 0)
            return false;

        if (!AIAttackIgnoreNoDieEffect::checkExcute(aiContext, target))
        {
            if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_DIE, 0) > 0)
                return false;
        }

        return true;
    }

    void AIResistFatalHurtNCastEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_RESIST_FATAL_HURT_NCAST);
        if (iter == target->m_aiEffectMap.end())
            return;

        for (auto &&pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            aiContext.scheduleExcute(pEff->innerExec->targetAttribute, pEff->skillID, pEff->skillLevel, pEff->origin, pEff->projectile, target, 0);
            return;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIForceAttackSelfEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIForceAttackSelfEffect::AIForceAttackSelfEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AIForceAttackSelfEffect::~AIForceAttackSelfEffect()
    {
        // onExit();
    }

    bool AIForceAttackSelfEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);

        expectedTick = parentCaster->totalTick;

        return true;
    }

    void AIForceAttackSelfEffect::onExit()
    {
        auto theTarget = getTargetSafe();
        if (!theTarget)
            return;

        this->setTempInvalid(true);

        //若效果没有解除，走完了持续时间，则触发另一个执行
        if (expectedTick >= 0 && expectedTick == parentCaster->totalTick)
        {
            int deltaTick = parentCaster->localTick - parentCaster->startTick;
            if ((expectedTick == 0 && deltaTick > expectedTick) || (expectedTick != 0 && deltaTick >= expectedTick))
            {
                if (innerExec->params.size() > 0)
                    aiContext.scheduleExcute(innerExec->params[0], skillID, skillLevel, origin, projectile, target, 0);
            }
        }

        if (theTarget->target == theTarget && AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *theTarget, 248, 0) <= 0)
        {
            auto anotherTarget = theTarget->lockTarget(aiContext);
            HeroActionController::setCurrentTarget(*theTarget, anotherTarget);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AICastByTeamHPRatioEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICastByTeamHPRatioEffect::AICastByTeamHPRatioEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AICastByTeamHPRatioEffect::~AICastByTeamHPRatioEffect()
    {
        // onExit();
    }

    bool AICastByTeamHPRatioEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(false), innerExec, skillLevel);
        if (innerExec->changeType == 1)
        {
            float changeValue = (float)(innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;

            auto friends = target->getOriginFriends();
            int totalHPTop = 0;
            for (auto iter = friends->begin(); iter != friends->end(); ++iter)
            {
                auto pCharacter = *iter;
                if (!pCharacter->isSummon())
                    totalHPTop += pCharacter->getHPTop();
            }

            _changeValue = totalHPTop * changeValue / 100;
        }

        aiContext.addEventListener("hpchange249", target);

        target->eff249cnt++;

        return true;
    }

    void AICastByTeamHPRatioEffect::onExit()
    {
        auto safeTarget = getTargetSafe();
        if (safeTarget)
        {
            safeTarget->eff249cnt--;
            if (safeTarget->eff249cnt <= 0)
                aiContext.removeEventListener("hpchange249", safeTarget);
        }
    }

    void AICastByTeamHPRatioEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_TEAM_HP_RATIO);
        if (iter == target->m_aiEffectMap.end())
            return;

        auto friends = target->getOriginFriends();
        int currentHP = 0;
        for (auto iter = friends->begin(); iter != friends->end(); ++iter)
        {
            auto pCharacter = *iter;
            if (!pCharacter->isSummon())
                currentHP += pCharacter->getHP();
        }

        for (auto &&pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            if (!pEff->isValid)
                continue;
            // TODO: check is begin fighting
            if (currentHP <= pEff->getChangeValue())
            {
                if (pEff->innerExec->params.size() > 0)
                {
                    aiContext.scheduleExcute(pEff->innerExec->params[0], pEff->skillID, pEff->skillLevel, pEff->origin, pEff->projectile, target, 0);
                    pEff->isValid = false;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AICastByTeamDeadCntEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICastByTeamDeadCntEffect::AICastByTeamDeadCntEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        : AIInnerEffect(pParent, pTarget, pExec)
    {

    }

    AICastByTeamDeadCntEffect::~AICastByTeamDeadCntEffect()
    {
        // onExit();
    }

    bool AICastByTeamDeadCntEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(false), innerExec, skillLevel);

        aiContext.addEventListener("herodead250", target);

        target->eff250cnt++;

        return true;
    }

    void AICastByTeamDeadCntEffect::onExit()
    {
        auto safeTarget = getTargetSafe();
        if (safeTarget)
        {
            safeTarget->eff250cnt--;
            if (safeTarget->eff250cnt <= 0)
                aiContext.removeEventListener("herodead250", safeTarget);
        }
    }

    void AICastByTeamDeadCntEffect::checkExcute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_TEAM_DEAD_CNT);
        if (iter == target->m_aiEffectMap.end())
            return;

        auto friends = target->getOriginFriends();
        int deadCnt = 0;
        for (auto iter = friends->begin(); iter != friends->end(); ++iter)
        {
            auto pCharacter = *iter;
            if (!pCharacter->isSummon() && HeroActionController::isDeadAction(pCharacter->currentAction))
                deadCnt++;
        }

        for (auto &&pEff : iter->second)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            if (!pEff->isValid)
                continue;
            // TODO: check is begin fighting
            if (deadCnt >= pEff->getChangeValue())
            {
                if (pEff->innerExec->params.size() > 0)
                {
                    aiContext.scheduleExcute(pEff->innerExec->params[0], pEff->skillID, pEff->skillLevel, pEff->origin, pEff->projectile, target, 0);
                    pEff->isValid = false;
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AICastWhenFriendSummonEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICastWhenFriendSummonEffect::AICastWhenFriendSummonEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
        :AIInnerEffect(pParent, pTarget, pExec)
    {
    }

    AICastWhenFriendSummonEffect::~AICastWhenFriendSummonEffect()
    {
        // onExit();
    }

    bool AICastWhenFriendSummonEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AICastWhenFriendSummonEffect::onExit()
    {
    }

    void AICastWhenFriendSummonEffect::checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *summon)
    {
        if (!target->hasEffect(eEFFECT_CAST_WHEN_FRIEND_SUMMON) || !summon->isSummon())
            return;

        auto &effects = target->m_aiEffectMap[eEFFECT_CAST_WHEN_FRIEND_SUMMON];
        for (auto pEff : effects)
        {
            if (!pEff || pEff->isTempInvalid()) continue;
            if (pEff->innerExec->params.size() > 0)
            {
                auto &&excludeParams = pEff->innerExec->params;
                auto excludeIter = std::find(excludeParams.begin(), excludeParams.end(), summon->getFakeSummonType());
                if (excludeIter != excludeParams.end())
                    continue;
            }
            
            auto execId = pEff->innerExec->targetAttribute;
            if (execId > 0)
            {
                AIActionController::executeExec(aiContext, execId, pEff->skillID, pEff->skillLevel, target, nullptr, target, 0);
            }
        }
    }

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIDurHitCritCastEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AIDurHitCritCastEffect::AIDurHitCritCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		: AIInnerEffect(pParent, pTarget, pExec)
	{

	}

	AIDurHitCritCastEffect::~AIDurHitCritCastEffect()
	{
		// onExit();
	}

	bool AIDurHitCritCastEffect::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		castInterval = innerExec->valueBase;
		castID = innerExec->changeValue;
		castHurtType = innerExec->targetAttribute;

		return true;
	}

	void AIDurHitCritCastEffect::onExit()
	{

	}



	void AIDurHitCritCastEffect::checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, int hurtType)
	{
		auto iter = source->m_aiEffectMap.find(eEFFECT_DUR_HIT_CRIT_EFFECT);
		if (iter == source->m_aiEffectMap.end())
			return;

		for (auto pEff : iter->second)
		{
            if (!pEff || pEff->isTempInvalid()) continue;
            auto theEff = (AIDurHitCritCastEffect *)(pEff);
			const int currentParentLocalTick = theEff->parentCaster->localTick;

			auto foundIter = theEff->castTargetRecord.find(target->myID);
			if (foundIter != theEff->castTargetRecord.end())
			{
				auto lastCastTick = foundIter->second;
				if (lastCastTick == currentParentLocalTick || lastCastTick + theEff->castInterval >= currentParentLocalTick)
				{
					continue;
				}
			}

			if (hurtType != theEff->castHurtType)
			{
				continue; 
			}

			aiContext.scheduleExcute(theEff->castID, theEff->skillID, theEff->skillLevel, source, nullptr, target, 0);
			theEff->castTargetRecord[target->myID] = currentParentLocalTick;
			theEff->bIsTriggered = true;
		}
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AICastWhenFriendDownEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AICastWhenFriendDownEffect::AICastWhenFriendDownEffect(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec)
        : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AICastWhenFriendDownEffect::~AICastWhenFriendDownEffect()
    {
        // onExit();
    }

    bool AICastWhenFriendDownEffect::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AICastWhenFriendDownEffect::onExit()
    {

    }

    void AICastWhenFriendDownEffect::checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *theFriend)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_WHEN_FRIEND_DIE);
        if (iter == target->m_aiEffectMap.end())
            return;


        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AICastWhenFriendDownEffect *)(innerEff);

            if (theEff->innerExec->params.size() > 0)
            {
                bool shouldExcludeFriend = false;
                for (auto anExcludeType : theEff->innerExec->params)
                {
                    if (theFriend->isSummon() && theFriend->getFakeSummonType() == anExcludeType)
                    {
                        shouldExcludeFriend = true;
                        break;
                    }
                }
                if (shouldExcludeFriend)
                    continue;
            }

            
            auto execInterval = theEff->innerExec->valueBase;
            if (execInterval > 0)
            {
                if (theEff->lastTriggerTick >= 0)
                {
                    if (theEff->parentCaster->localTick <= theEff->lastTriggerTick + execInterval)
                        continue;
                }

                theEff->lastTriggerTick = theEff->parentCaster->localTick;
            }


            auto execId = theEff->innerExec->targetAttribute;
            if (execId > 0)
            {
                AIActionController::executeExec(aiContext, execId, theEff->skillID, theEff->skillLevel, target, nullptr, target, 0);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect258
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect258::AIEffect258(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect258::~AIEffect258()
    {
        // onExit();
    }

    bool AIEffect258::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AIEffect258::onExit()
    {

    }

    void AIEffect258::checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *attacker, int theCasterId)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_SPEC_LABEL_HURT);
        if (iter == target->m_aiEffectMap.end() || !attacker)
            return;

        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect258 *)(innerEff);
            // 该效果不会被本身配置的伤害或效果执行触发
            if (theCasterId > 0 && theCasterId == theEff->innerExec->targetAttribute)
                continue;
            if (theCasterId > 0 && theCasterId == theEff->innerExec->valueBase)
                continue;
            
            for (auto theLabel : theEff->innerExec->params)
            {
                auto &&theLabels = attacker->staticData->vLabels;
                if (std::find(theLabels.begin(), theLabels.end(), theLabel) != theLabels.end())
                {
                    auto execId = theEff->innerExec->targetAttribute;
                    if (execId > 0)
                    {
                        //AIActionController::executeExec(aiContext, execId, theEff->skillID, theEff->skillLevel, attacker, nullptr, attacker, 0);
                        aiContext.scheduleExcute(execId, theEff->skillID, theEff->skillLevel, attacker, nullptr, attacker, 0);
                    }

                    execId = theEff->innerExec->valueBase;
                    if (execId > 0)
                    {
                        //AIActionController::executeExec(aiContext, execId, theEff->skillID, theEff->skillLevel, target, nullptr, target, 0);
                        aiContext.scheduleExcute(execId, theEff->skillID, theEff->skillLevel, target, nullptr, target, 0);
                    }
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect259
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect259::AIEffect259(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect259::~AIEffect259()
    {
        // onExit();
    }

    bool AIEffect259::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);
        if (_changeValue == 0)
            return false;

        return true;
    }

    void AIEffect259::onExit()
    {

    }

    int AIEffect259::checkExecute(AIContext &aiContext, CharacterData *target, int theMarkID, int hpdelta)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_HURT_DOWN_FROM_MARK);
        if (iter == target->m_aiEffectMap.end())
            return 0;

        int value = 0;
        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect259 *)(innerEff);
            auto &&params = theEff->innerExec->params;
            if (std::find(params.begin(), params.end(), theMarkID) == params.end())
                continue;

            if (theEff->innerExec->changeType == 1)
            {
                value += hpdelta * theEff->getChangeValue() / 100;
            }
            else
            {
                value += theEff->getChangeValue();
            }
        }
        return value;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect260
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect260::AIEffect260(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) :AIInnerEffect(theParent, theTarget, theExec)
    {
    }

    AIEffect260::~AIEffect260()
    {
        // onExit();
    }

    bool AIEffect260::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _triggerEffectId = innerExec->targetAttribute; //指定效果ID
        if (_triggerEffectId <= 0)
            return false;

        _triggerMode = innerExec->valueBase; //判断模式 0-等于或已有 1-不等于或没有
        _triggerRelativeTeam = innerExec->changeValue; //指定可触发阵营 0-无限制 1-敌方 2-我方
        _triggerCharacterId = innerExec->levelup; //指定可触发角色 0-无限制

        executeActive();

        return true;
    }

    void AIEffect260::onExit()
    {
    }

    void AIEffect260::executeActive()
    {
        if (_hasTriggered)
            return;

        bool haveFoundMatch = false;
        auto iter = target->m_aiEffectMap.find(_triggerEffectId);
        if (iter != target->m_aiEffectMap.end())
        {
            for (auto &&innerEff : iter->second)
            {
                if (!innerEff || innerEff->isTempInvalid()) continue;
                if (isMatchCondition(innerEff->origin, iter->first))
                {
                    haveFoundMatch = true;
                    break;
                }
            }
        }

        if (_triggerMode == 0 && !haveFoundMatch) // 0-等于或已有
            return;
        else if (_triggerMode == 1 && haveFoundMatch) // 1-不等于或没有
            return;

        _hasTriggered = true;
        for (auto execId : innerExec->params)
        {
            if (execId > 0)
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }

    void AIEffect260::executePassive(CharacterData *source, int theEffectId)
    {
        if (_hasTriggered)
            return;

        bool isMatchSpecCondition = isMatchCondition(source, theEffectId);
        if (_triggerMode == 0 && !isMatchSpecCondition) // 0-等于或已有
            return;
        else if (_triggerMode == 1 && isMatchSpecCondition) // 1-不等于或没有
            return;

        if (!isEffectTargetMatchCondition())
            return;

        _hasTriggered = true;

        for (auto execId : innerExec->params)
        {
            if (execId > 0)
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }

    bool AIEffect260::isMatchCondition(CharacterData *source, int theEffectId)
    {
        if (theEffectId != _triggerEffectId)
            return false;

        if (_triggerRelativeTeam == 1 && source->teamSide == target->teamSide) //1-敌方
            return false;
        else if (_triggerRelativeTeam == 2 && source->teamSide != target->teamSide) //2-我方
            return false;

        if (_triggerCharacterId > 0 && source->getID() != _triggerCharacterId)
            return false;

        return true;
    }

    bool AIEffect260::isEffectTargetMatchCondition()
    {
        bool haveFoundMatch = false;
        auto iter = target->m_aiEffectMap.find(_triggerEffectId);
        if (iter != target->m_aiEffectMap.end())
        {
            for (auto &&innerEff : iter->second)
            {
                if (!innerEff || innerEff->isTempInvalid()) continue;
                if (isMatchCondition(innerEff->origin, iter->first))
                {
                    haveFoundMatch = true;
                    break;
                }
            }
        }

        if (_triggerMode == 0 && !haveFoundMatch) // 0-等于或已有
            return false;
        else if (_triggerMode == 1 && haveFoundMatch) // 1-不等于或没有
            return false;

        return true;
    }

    void AIEffect260::checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *source, int theEffectId)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_SPEC_EFFECT);
        if (iter == target->m_aiEffectMap.end())
            return;

        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect260*)(innerEff);
            theEff->executePassive(source, theEffectId);
        }
    }
    
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect262
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    AIEffect262::AIEffect262(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {
    }
    
    AIEffect262::~AIEffect262()
    {
        // onExit();
    }
    
    bool AIEffect262::onEnter()
    {
        if (isDebuffed())
            return false;
        
        checkCleanSameTypeExec();
        
        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        
        AIContinuousEffectCaster::startEffectCD(*target, innerExec);
        
        return true;
    }
    
    void AIEffect262::onExit()
    {
    }
    
    int AIEffect262::checkExecute(CharacterData *target, int skillId)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CRIT_IF_HIT);
        int changeValue = 0;
        if (iter != target->m_aiEffectMap.end())
        {
            for (auto &&innerEff : iter->second)
            {
                if (!innerEff || innerEff->isTempInvalid()) continue;
                if (skillId != innerEff->innerExec->targetAttribute)
                    continue;
                
                changeValue += innerEff->getChangeValue();
            }
        }
        
        return changeValue;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect264
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect264::AIEffect264(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) :AIInnerEffect(theParent, theTarget, theExec)
    {
    }

    AIEffect264::~AIEffect264()
    {
        // onExit();
    }

    bool AIEffect264::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _triggerActionType = innerExec->targetAttribute; //指定效果ID
        if (_triggerActionType <= 0)
            return false;
        
        _triggerMode = innerExec->valueBase; //判断模式 0-等于 1-不等于
        _triggerRelativeTeam = innerExec->changeValue; //指定可触发阵营 0-无限制 1-敌方 2-我方
        _triggerCharacterId = innerExec->levelup; //指定可触发角色 0-无限制
        
        executeActive();

        return true;
    }

    void AIEffect264::onExit()
    {
    }

    void AIEffect264::executeActive()
    {
        if (_hasTriggered)
            return;

//        // 0-相同状态 1-不同状态
//        if (innerExec->valueBase == 0 && target->currentAction != innerExec->targetAttribute)
//            return;
//        else if (innerExec->valueBase == 1 && target->currentAction == innerExec->targetAttribute)
//            return;
//
//        _hasTriggered = true;
//        for (auto execId : innerExec->params)
//        {
//            if (execId > 0)
//                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
//        }
        
        bool haveFoundMatch = false;
        if (target->currentAction == _triggerActionType)
        {
            if (isMatchCondition(target->actionCauserMyID, target->currentAction))
                haveFoundMatch = true;
        }
        
        if (_triggerMode == 0 && !haveFoundMatch) // 0-等于或已有
            return;
        else if (_triggerMode == 1 && haveFoundMatch) // 1-不等于或没有
            return;
        
        _hasTriggered = true;
        for (auto execId : innerExec->params)
        {
            if (execId > 0)
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }
    
    void AIEffect264::executePassive(int sourceMyId, int theTargetActionType)
    {
        if (_hasTriggered)
            return;
        
        bool isMatchSpecCondition = isMatchCondition(sourceMyId, theTargetActionType);
        if (_triggerMode == 0 && !isMatchSpecCondition) // 0-等于
            return;
        else if (_triggerMode == 1 && isMatchSpecCondition) // 1-不等于
            return;
        
        _hasTriggered = true;
        
        for (auto execId : innerExec->params)
        {
            if (execId > 0)
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }
    
    bool AIEffect264::isMatchCondition(int sourceMyId, int theActionType)
    {
        if (theActionType != _triggerActionType)
            return false;
        
        if (sourceMyId < 0)
            return false;
        
        auto source = aiContext.getCharacterByID(sourceMyId);
        if (!source)
            return false;
        
        if (_triggerRelativeTeam == 1 && source->teamSide == target->teamSide) //1-敌方
            return false;
        else if (_triggerRelativeTeam == 2 && source->teamSide != target->teamSide) //2-我方
            return false;
        
        if (_triggerCharacterId > 0 && source->getID() != _triggerCharacterId)
            return false;
        
        return true;
    }

//    void AIEffect264::checkExecute(AIContext &aiContext, CharacterData *target)
//    {
//        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_SPEC_STATE);
//        if (iter == target->m_aiEffectMap.end())
//            return;
//
//        for (auto &&innerEff : iter->second)
//        {
//            auto theEff = (AIEffect264 *)(innerEff.get());
//            theEff->executeActive();
//        }
//    }
    
    void AIEffect264::checkExecute(AIContext &aiContext, CharacterData *target, int causerMyId, int theTargetActionType)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_CAST_BY_SPEC_STATE);
        if (iter == target->m_aiEffectMap.end())
            return;
        
        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect264 *)(innerEff);
            theEff->executePassive(causerMyId, theTargetActionType);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect265
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect265::AIEffect265(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec):AIInnerEffect(theParent, theTarget, theExec)
    {
    }

    AIEffect265::~AIEffect265()
    {
        // onExit();
    }

    bool AIEffect265::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);

        return true;
    }

    void AIEffect265::onExit()
    {
        isValid = false;
        auto theTarget = getTargetSafe();
        auto theSource = aiContext.getCharacterByID(_originMyID);
        if (theTarget && theSource && _absorbedHpdelta > 0)
        {
            int hurtExecType = innerExec->valueBase; // 0-物理，1-法术，2-纯粹
            if (hurtExecType == 2)
            {
                hurtExecType = -1;
            }

            float ratio = (float)getChangeValue() / 10000.0f;
            int hpdelta = (float)_absorbedHpdelta * ratio;

            HurtCalcParam param(projectile, skillID, skillLevel, hpdelta, hurtExecType, parentCaster->effectID);
            param.isRawHurt = (hurtExecType == -1);
            
            param.ignoreHurtLabel = false;
            param.isFromEffectCaster = true;
            param.markID = parentCaster->markID;

            if (innerExec->params.size() > 0)
                param.hurtProperty = innerExec->params[0];

            //auto result = HeroHurtcalculate::HurtFuncHurtTargetScript(aiContext, theSource, theTarget, param);
            HitResult result;
            HeroHurtcalculate::HurtFuncHurtTarget(aiContext, theSource, theTarget, param, &result);

            if (result.blockResult != HitResult::BlockType::HIT_BLOCKED)
                theTarget->onHurt(true, innerExec->ID, param.HPDelta, param.hurtType, theSource, parentCaster->effectID);

            if (theTarget->getHP() <= 0)
            {
                HeroHurtcalculate::characterDeadByHurt(aiContext, theTarget, theSource, skillID, skillLevel, nullptr, innerExec);
            }
            else
            {
				// 13.0 modify
				if (result.blockResult != HitResult::BlockType::HIT_BLOCKED && result.hitResult == HitResult::HitResultType::HIT_HASHURT)
                {
					if (param.hurtType == -1)
					{
						param.hurtType = 3; // 效果伤害里的纯粹伤害类型-1 ->> 标记执行纯粹伤害配置 类型：-1所有 0-物理 1-法术 3-纯粹伤害
					}
                    HeroHurtcalculate::ToTriggerMarkCasterFun(aiContext, *theTarget, 0, skillLevel, param.hurtType, param.hurtProperty, param.HPDelta, param.hitType, theSource);
                }
                HeroSkillController::checkReleaseHPSkill(aiContext, theTarget);
            }
        }
    }

    void AIEffect265::absorbHpdelta(int hpdelta)
    {
        _absorbedHpdelta += hpdelta;
    }

    int AIEffect265::checkExecute(AIContext &aiContext, CharacterData *target, int hpdelta, bool isSimulate)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SHIELD_TRANSFER_HURT);
        if (iter == target->m_aiEffectMap.end())
            return 0;

        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect265 *)(innerEff);
            if (theEff->isValid)
            {
                if (!isSimulate)
                    theEff->absorbHpdelta(hpdelta);
                return hpdelta;
            }
        }

        return 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect266
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect266::AIEffect266(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {
    }

    AIEffect266::~AIEffect266()
    {
        // onExit();
    }

    bool AIEffect266::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        return true;
    }

    void AIEffect266::onExit()
    {
    }
    
    void AIEffect266::setRemainEffectActionTick(int theRemainTick)
    {
        _remainEffectActionTick = theRemainTick;
    }

    bool AIEffect266::checkExecute(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_STATE_VERTIGO);
        if (iter == target->m_aiEffectMap.end())
            return false;

        for (auto innerIter = iter->second.rbegin(); innerIter != iter->second.rend(); ++innerIter)
        {
            auto innerEff = *innerIter;
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect266 *)(innerEff);
            int stateTick = theEff->getRemainEffectActionTick();
            if (stateTick <= 0)
                continue;
            auto theEffCaster = innerEff->parentCaster;
            auto staticData = theEffCaster->staticData;
            theEffCaster->stateAniAffectedChars[target->myID] = true;

            if (HeroActionController::isDeadAction(target->currentAction))
                return false;

            if (staticData->canStateAnimationBeDebuffed)
            {
                if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_DEBUF_REPEL, 0))
                    return false;
            }
            
            ActionType oldAction = (ActionType)HeroActionController::getOldAction(target->actionAtt);
            int oldTotalTick = HeroActionController::getOldTotalTick(target->actionAtt);
            int oldTick = HeroActionController::getOldTick(target->actionAtt);

            HeroEffectAction::Enter(*target, aiContext, staticData->ID, stateTick, oldAction, oldTotalTick, oldTick, staticData->canStateAnimationBeDebuffed, *theEffCaster->origin);

            return true;
        }

        return false;
    }
    
    void AIEffect266::checkSaveRemainTick(AIContext &aiContext, CharacterData *target)
    {
        if (target->currentAction != EFFECT_ACTION)
            return;
        
        if (target->localTick > target->startTick + target->totalTick)
            return;
        
        auto iter = target->m_aiEffectMap.find(eEFFECT_STATE_VERTIGO);
        if (iter == target->m_aiEffectMap.end())
            return;
        
        int remainTick = target->totalTick - (target->localTick - target->startTick);
        for (auto innerEff : iter->second)
        {
            if (!innerEff || innerEff->isTempInvalid()) continue;
            auto theEff = (AIEffect266 *)(innerEff);
            theEff->setRemainEffectActionTick(remainTick);
        }
    }


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIEffect267
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// clair_wang 新增效果：若对目标造成的伤害低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的真实伤害。 
	AIEffect267::AIEffect267(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec)
		:AIInnerEffect(pParent, pTarget, pExec)
	{}

	AIEffect267::~AIEffect267()
	{
		// onExit();
	}

	bool AIEffect267::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		setBaseInfo();

		executeActive(origin);

		return true;
	}

	void AIEffect267::onExit()
	{
		if (!isValid)
		{
			return;
		}
	}

	void AIEffect267::setBaseInfo()
	{
		if (innerExec->changeType == 1)
		{
			float deltaComp = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f;

			mCauseHurt = deltaComp / 100.0f * target->getHPTop();
		}
		else
		{
			mCauseHurt = (innerExec->levelup * skillLevel) / 100.0f;
		}

		mAddHurtPercent = innerExec->limitOnOff / 10000.0f;

		limitUpper = innerExec->limitValUpperBase + innerExec->limitValUpperGrowth * skillLevel;
		limitLower = innerExec->limitValLowerBase + innerExec->limitValLowerGrowth * skillLevel;


		if (innerExec->targetAttribute == 0)
		{
			bIsUsefulFlag = true;
		}
		else
		{
			mTriggerExecId = innerExec->targetAttribute;
		}
	}


	int AIEffect267::executeActive(CharacterData *attacker)
	{
		if (_hasTriggered)
			return 0;

		if (attacker == nullptr)
			return 0;

		if (target->pLastHurtOriginCharacter == nullptr)
			return 0;

		if ((!bIsUsefulFlag) && (attacker->myID != target->pLastHurtOriginCharacter->myID))
		{
			return 0;
		}

		bool bIsSpecialFlag = false;
		if (!bIsUsefulFlag && mTriggerExecId > 0)
		{
			if (IsHurtExec(mTriggerExecId))
			{
				if (target->lastHurtExecId == mTriggerExecId)
				{
					bIsSpecialFlag = true;
				}
			}
			else
			{
				if (IsEffectExec(mTriggerExecId))
				{
					int mExecID = mTriggerExecId;
					auto theExec = AIDataHelper::getEffectExec(mExecID);
					if (theExec)
					{
						for (auto &&innerExec : theExec->execs) {
							if (innerExec.targetAttribute == mExecID || innerExec.valueBase == mExecID || innerExec.changeValue == mExecID || innerExec.levelup == mExecID || innerExec.limitOnOff == mExecID || innerExec.limitValLowerBase == mExecID || innerExec.limitValLowerGrowth == mExecID || innerExec.limitValUpperBase == mExecID || innerExec.limitValUpperGrowth == mExecID)
							{
								bIsSpecialFlag = true;
							}
						}
					}
				}
			}
		}

		if (!bIsUsefulFlag && (!bIsSpecialFlag))
			return 0;

		if (target->lastHurtExecRealHurtDelta > mCauseHurt)
		{
			return 0;
		}

		_changeValue = mAddHurtPercent * target->lastHurtExecInitHurtDelta;

		if (_changeValue > limitUpper)
		{
			_changeValue = limitUpper;
		}
		if (_changeValue < limitLower)
		{
			_changeValue = limitLower;
		}

		_hasTriggered = true;


		_changeValue = EffectHelper::applyHPChange(aiContext, origin, projectile, target, skillID, skillLevel, ECharacterAttr::HP, _changeValue, innerExec, parentCaster->markID, parentCaster->effectID, this);

		// 击中文字,可以考虑移到内部效果自行处理
		if (innerExec->label > 0 && !isIgnoreHitLabel())
		{
			int displayValue = getHitLabelDisplayValue();
			
			if (displayValue)
				AISpecialEffectLabel::excute(innerExec->label, displayValue, aiContext, target);
		}

		_ignoreHitLabel = true;

		return 1;
	}

	int AIEffect267::checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *attacker)
	{
		if (attacker == nullptr && target == nullptr)
			return 0;

		auto iter = target->m_aiEffectMap.find(eEFFECT_CREATE_HURT_ADD_VALUE_NEW);
		if (iter == target->m_aiEffectMap.end() || !attacker)
			return 0;

		int changevalue = 0;

		for (auto innerEff : iter->second)
		{
            if (!innerEff || innerEff->isTempInvalid()) continue;
			auto theEff = (AIEffect267 *)(innerEff);
			changevalue += theEff->executeActive(attacker);
		}

		return changevalue;
	}

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIRepelEffect
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    AIRepelEffect::AIRepelEffect(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {
    }
    
    AIRepelEffect::~AIRepelEffect()
    {
        // onExit();
    }
    
    bool AIRepelEffect::onEnter()
    {
        if (isDebuffed())
            return false;
        
        checkCleanSameTypeExec();
        
        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);
        
        AIContinuousEffectCaster::startEffectCD(*target, innerExec);
        
        return true;
    }
    
    void AIRepelEffect::onExit()
    {
        
    }
    
    int AIRepelEffect::checkExecute(int repelEffectType, AIContext &aiContext, CharacterData *target, int HPDelta, bool isSimulate)
    {
        auto iter = target->m_aiEffectMap.find(repelEffectType);
        if (iter == target->m_aiEffectMap.end())
            return 0;
        
        int repelNum = 0;
        
        for (auto effIter = iter->second.begin(); effIter != iter->second.end();)
        {
            auto theEff = *effIter;
            if (!theEff || theEff->isTempInvalid())
            {
                ++effIter;
                continue;
            }
            HPDelta -= theEff->getChangeValue();
            repelNum += theEff->getChangeValue();
            if (HPDelta >= 0)
            {
                if (!isSimulate)
                {
                    theEff->setChangeValue(0);
                    
                    //for (auto theExecId : theEff->innerExec->params)
                    //{
                    //    AIActionController::executeExec(aiContext, theExecId, theEff->skillID, theEff->skillLevel, theEff->origin, theEff->projectile, target, 0);
                    //}

                    // HQ:10.0 new impl
                    // 前9位作为执行id
                    for (int i = 0; i < theEff->innerExec->params.size() && i < 9; i++)
                    {
                        int theExecId = theEff->innerExec->params[i];
                        if (theExecId > 0)
                            AIActionController::executeExec(aiContext, theExecId, theEff->skillID, theEff->skillLevel, theEff->origin, theEff->projectile, target, 0);
                    }
                    
                    theEff->onEraseEffect();
                    effIter = iter->second.erase(effIter);
                }
            }
            else
            {
                repelNum += HPDelta;
                if (!isSimulate)
                {
                    theEff->setChangeValue(-HPDelta);
                }
                break;
            }
            
        }
        
        if (!isSimulate && iter->second.size() == 0)
        {
            target->m_aiEffectMap.erase(iter);
        }
        
        if (!isSimulate && repelNum > 0)
        {
            switch (repelEffectType) {
                case eEFFECT_BAREPEL_ADD:
                    AISpecialEffectLabel::excute(PHY_IMMUNIZE_LABEL, repelNum, aiContext, target);
                    break;
                case eEFFECT_MAREPEL_ADD:
                    AISpecialEffectLabel::excute(MAG_IMMUNIZE_LABEL, repelNum, aiContext, target);
                    break;
                case eEFFECT_ALL_REPEL:
                    AISpecialEffectLabel::excute(ABSORB_HURT_LABEL, repelNum, aiContext, target);
                    break;
                default:
                    break;
            }
            
        }
        
        return repelNum;
    }

    void AIRepelEffect::checkCleanEffectOnCharacterHpChange(AIContext &aiContext, CharacterData *target)
    {
        checkCleanOneEffectOnCharacterHpChange(eEFFECT_BAREPEL_ADD, aiContext, target);
        checkCleanOneEffectOnCharacterHpChange(eEFFECT_MAREPEL_ADD, aiContext, target);
        checkCleanOneEffectOnCharacterHpChange(eEFFECT_ALL_REPEL, aiContext, target);
    }

    void AIRepelEffect::checkCleanOneEffectOnCharacterHpChange(int repelEffectType, AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(repelEffectType);
        if (iter == target->m_aiEffectMap.end())
            return;

        if (target->getHPTop() == 0)
            return;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end();)
        {
            auto theEff = *effIter;
            if (!theEff || theEff->isTempInvalid())
            {
                ++effIter;
                continue;
            }
            bool theShouldClean = false;
            if (theEff->innerExec->params.size() > 9)
            {
                // 第10位作为血量百分比参数
                float theHpRatioLimit = float(theEff->innerExec->params[9]) / 100.0f;
                float targetHPRatio = CharacterFunc::GetCharacterHPRatio(*target) * 100.0f;
                if (targetHPRatio <= theHpRatioLimit)
                {
                    theShouldClean = true;
                }
            }

            if (theShouldClean)
            {
                theEff->setChangeValue(0);

                for (int i = 0; i < theEff->innerExec->params.size() && i < 9; i++)
                {
                    int theExecId = theEff->innerExec->params[i];
                    if (theExecId > 0)
                        AIActionController::executeExec(aiContext, theExecId, theEff->skillID, theEff->skillLevel, theEff->origin, theEff->projectile, target, 0);
                }

                theEff->onEraseEffect();
                effIter = iter->second.erase(effIter);
            }
            else
            {
                ++effIter;
            }
        }

        if (iter->second.size() == 0)
        {
            target->m_aiEffectMap.erase(iter);
        }
    }
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIEffect270
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AIEffect270::AIEffect270(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
	{
	}

	AIEffect270::~AIEffect270()
	{
		// onExit();
	}

	bool AIEffect270::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		float mChangeValue = 0.f;
		mChangeValue = (innerExec->changeValue + innerExec->levelup * skillLevel) / 100.0f; //最终计算时除以100
        if (innerExec->changeType == 1)
        {
            int valueBase = innerExec->valueBase;
            if (valueBase > 0 && valueBase < ATT_NUM)
            {
				mChangeValue *= origin->attributes[valueBase] / 100.f; //最终计算时再除以100
            }
        }
		else
		{
			mChangeValue /= 100.f;
		}

		// mChangeValue *= innerExec->params[0] / 100.f; //系数a使用时除以100
		mChangeValue *= innerExec->limitOnOff / 100.f; //系数a使用时除以100

		_changeValue = getBoundedChangeValue(mChangeValue, innerExec, skillLevel);

        _changeValue = EffectHelper::applyHPChange(aiContext, origin, projectile, target, skillID, skillLevel, ECharacterAttr::HP, _changeValue, innerExec, parentCaster->markID, parentCaster->effectID, this);

		AIContinuousEffectCaster::startEffectCD(*target, innerExec);

		return true;
	}

	void AIEffect270::onExit()
	{

	}


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIEffect271
	// // 271 - 承受伤害小于一定阈值伤害不生效，且触发执行id
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	AIEffect271::AIEffect271(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
	{
	}

	AIEffect271::~AIEffect271()
	{
		// onExit();
	}

	bool AIEffect271::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		_changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

		_triggerCD = innerExec->targetAttribute;

		return true;
	}

	void AIEffect271::onExit()
	{

	}
	
	void AIEffect271::checkExecute(AIContext &aiContext, CharacterData *target)
	{
		auto iter = target->m_aiEffectMap.find(eEFFECT_ENDURE_HURT_BY_CONDITION);
		if (iter == target->m_aiEffectMap.end())
			return;

		for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
		{
            auto anEff = (AIEffect271 *)(*effIter);
            if (!anEff || anEff->isTempInvalid()) continue;

			if (anEff->onStartTrigger())
			{
				anEff->castInnerExec();
			}

			AIContinuousEffectCaster::startEffectCD(*target, anEff->innerExec);
		}
	}


	bool AIEffect271::onStartTrigger()
	{
		if (_triggerCD > 0)
		{
			if (_lastTrigParentTick >= 0 && parentCaster->localTick <= _lastTrigParentTick + _triggerCD)
			{
				return false;
			}
		}


		_lastTrigParentTick = parentCaster->localTick;

		return true;
	}

	void AIEffect271::castInnerExec()
	{
		// 数值计算基准作为触发的执行ID
		int theExecId = innerExec->valueBase;
		if (theExecId > 0)
		{
			AIActionController::executeExec(aiContext, theExecId, skillID, skillLevel, origin, nullptr, target, 0);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                                            AIEffect272
	// 272 - 受到致命伤害（本次伤害会造成死亡）时，保留1滴血量不死，触发一个执行
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AIEffect272::AIEffect272(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
	{
	}

	AIEffect272::~AIEffect272()
	{
		// onExit();
	}

	bool AIEffect272::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		//_changeValue = getBoundedChangeValue(caculateChangeValue(true), innerExec, skillLevel);
        setInnerData("efftick", innerExec->changeValue / 100);

		return true;
	}

	void AIEffect272::onExit()
	{

	}

    void AIEffect272::onScheduleTick()
    {
        setTempInvalid(true);
    }

	bool AIEffect272::canCast(AIContext &aiContext, CharacterData *target, int hpdelta)
	{
		if (hpdelta < target->getHP())
			return false;

        if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, eEFFECT_FATAL_HURT_DO_EFFECT, 0) <= 0)
            return false;

		if (!AIAttackIgnoreNoDieEffect::checkExcute(aiContext, target))
		{
			if (AIContinuousEffectCaster::check_exert_Special_Effect(aiContext, *target, EffectType::eEFFECT_NO_DIE, 0) > 0)
				return false;
		}

		return true;
	}

	void AIEffect272::checkExecute(AIContext &aiContext, CharacterData *target)
	{
		auto iter = target->m_aiEffectMap.find(eEFFECT_FATAL_HURT_DO_EFFECT);
		if (iter == target->m_aiEffectMap.end())
			return;

		for (auto pEff : iter->second)
		{
			//aiContext.scheduleExcute(pEff->innerExec->targetAttribute, pEff->skillID, pEff->skillLevel, pEff->origin, pEff->projectile, target, 0);
			
			//// 累计触发次数
            //pEff->setChangeValue(pEff->getChangeValue() + 1);
            
            if (!pEff || pEff->isTempInvalid())
                continue;
            
            // 12.0 数值大小字段：触发后效果持续时间
            // 在这段时间内，效果不增加次数，不释放执行
            int lastdoCasterTick = pEff->getInnerData("lastdo_castertick");
            if (!pEff->hasInnerData("lastdo_castertick") || lastdoCasterTick + pEff->getInnerData("efftick") <= pEff->parentCaster->localTick)
            {
                auto theInnerEff = (AIEffect272 *)pEff;
                for (auto execId : theInnerEff->innerExec->params)
                    aiContext.scheduleExcute(execId, theInnerEff->skillID, theInnerEff->skillLevel, theInnerEff->origin, theInnerEff->projectile, target, 0);
                
                theInnerEff->_triggerCnt += 1;
                
                pEff->setInnerData("lastdo_castertick", pEff->parentCaster->localTick);
                if (theInnerEff->_triggerCnt >= pEff->innerExec->valueBase)
                {
                    pEff->parentCaster->scheduleInnerEff(pEff->parentCaster->localTick + pEff->getInnerData("efftick"), pEff);
                }
            }

			return;
		}
	}
	
	bool AIEffect272::checkZeroEffect(AIContext &aiContext, CharacterData *target)
	{
		auto iter = target->m_aiEffectMap.find(eEFFECT_FATAL_HURT_DO_EFFECT);
		if (iter == target->m_aiEffectMap.end())
			return false;

		//for (auto pEff : iter->second)
		//{
  //          if (!pEff || pEff->isTempInvalid()) continue;
  //          auto theInnerEff = (AIEffect272 *)pEff;
  //          if (theInnerEff->_triggerCnt >= pEff->innerExec->valueBase)
		//	{
		//		return true;
		//	}
		//}

		return false;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 AIEffect273 AIAbsorbAllTypeHurtShieldEffect
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	AIEffect273::AIEffect273(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) :AIInnerEffect(theParent, theTarget, theExec)
	{
	}

	AIEffect273::~AIEffect273()
	{
		// onExit();
	}

	bool AIEffect273::onEnter()
	{
		if (isDebuffed())
			return false;

		checkCleanSameTypeExec();

		int mA = innerExec->changeType;
		int mB = innerExec->targetAttribute;
		int mBlvUp = innerExec->valueBase;
		
		int mC = innerExec->changeValue;
		int mClvUp = innerExec->levelup;

		float mRatioA = 0.0f;
		float mRatioB = (float)(mB + mBlvUp * skillLevel) / 100.f;
		float mRatioC = (float)(mC + mClvUp * skillLevel) / 100.f;
		
		if (mA > 0 && mA < ATT_NUM)
		{
			mRatioA = target->attributes[mA];
		}

		float _mAbsorb = mRatioA * mRatioB + mRatioC;//  护盾吸收量 = a属性的值*系数b+固定值c
		_changeValue = getBoundedChangeValue(_mAbsorb, innerExec, skillLevel);

		AIContinuousEffectCaster::startEffectCD(*target, innerExec);

		return true;
	}

	void AIEffect273::onExit()
	{
		if (isBroken)
            return;
        if (getTargetSafe() == nullptr)
            return;
        
        int exec_max_cnt = innerExec->params.size();
        // 参数后5位：效果时间到触发执行
        if (exec_max_cnt < 6)
            return;
        
        for (int j = 5; j < exec_max_cnt; j++)
        {
            int anexecid = innerExec->params[j];
            if (anexecid > 0)
            {
                AIActionController::executeExec(aiContext, anexecid, skillID, skillLevel, origin, projectile, target, 0);
            }
        }
	}

	int AIEffect273::checkExecute(AIContext &aiContext, CharacterData *target, int HPDelta)
	{
		auto iter = target->m_aiEffectMap.find(eEFFECT_ABSORB_ALLTYPE_HURT_SHIELD);
		if (iter == target->m_aiEffectMap.end())
			return 0;

		int repelNum = 0;

//        for (auto effIter = iter->second.begin(); effIter != iter->second.end();)
//        {
//            auto theEff = *effIter;
//            if (!theEff || theEff->isTempInvalid())
//            {
//                ++effIter;
//                continue;
//            }
//
//            repelNum += theEff->getChangeValue();
//
//            if (repelNum <= HPDelta)
//            {
//                theEff->setChangeValue(0);
//                theEff->onEraseEffect();
//                effIter = iter->second.erase(effIter);
//            }
//            else
//            {
//                theEff->setChangeValue(theEff->getChangeValue() - HPDelta);
//                break;
//            }
//        }
//
//        if (iter->second.size() == 0)
//        {
//            target->m_aiEffectMap.erase(iter);
//        }
        
        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            AIEffect273 *theEff = (AIEffect273 *)(*effIter);
            if (!theEff || theEff->isTempInvalid())
            {
                continue;
            }

            repelNum += theEff->getChangeValue();

            if (repelNum <= HPDelta)
            {
                theEff->setChangeValue(0);
                theEff->onEraseEffect();
                theEff->setTempInvalid(true);
                theEff->isBroken = true;
                
                int exec_max_cnt = theEff->innerExec->params.size();
                // 参数前5位：破盾执行
                if (exec_max_cnt > 5)
                    exec_max_cnt = 5;
                
                for (int j = 0; j < exec_max_cnt; j++)
                {
                    int anexecid = theEff->innerExec->params[j];
                    if (anexecid > 0)
                    {
                        AIActionController::executeExec(aiContext, anexecid, theEff->skillID, theEff->skillLevel, theEff->origin, theEff->projectile, theEff->target, 0);
                    }
                }
            }
            else
            {
                theEff->setChangeValue(theEff->getChangeValue() - HPDelta);
                break;
            }
        }

		return repelNum;
	}

    AIEffect268::AIEffect268(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) :
        AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect268::~AIEffect268()
    {
        // onExit();
    }

    bool AIEffect268::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        //_changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        HeroDeadAction::ForceEnter(*target, aiContext);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIEffect268::onExit()
    {

    }

    AIEffect269::AIEffect269(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect269::~AIEffect269()
    {
        // onExit();
    }

    bool AIEffect269::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _undividedChangeValue = getBoundedUndividedChangeValue(calculateUndividedChangeValue());
        _changeValue = _undividedChangeValue / 100;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _ignoreHitLabel = true;

        return true;
    }

    void AIEffect269::onExit()
    {

    }

    int AIEffect269::getExtraHurt(AIContext &aiContext, CharacterData *theTarget, int theHpDelta)
    {
        if (HeroActionController::isDeadAction(theTarget->currentAction))
            return 0;

        auto iter = theTarget->m_aiEffectMap.find(eEFFECT_CAUSE_HURT_BY_CURR_HP);
        if (iter == theTarget->m_aiEffectMap.end())
            return 0;

        int theExtraHurt = 0;
        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto theEff = (AIEffect269 *)(*effIter);
            if (!theEff || theEff->isTempInvalid()) continue;
            int theEffType = theEff->innerExec->params[0];
            int theEffAlphaValue = theEff->innerExec->params[1];
            int theEffMinValue = theEff->innerExec->params[2];
            float theRatio = 0;
            if (theTarget->getHPTop() != 0)
            {
                if (theEffType == 0)
                {
                    //0血量越高伤害越高
                    //总伤害 = 最终伤害*(1 + max(效果系数最小值, 当前血量百分比*a))
                    theRatio = theEffAlphaValue * theTarget->getHP() / theTarget->getHPTop();
                }
                else if (theEffType == 1)
                {
                    //1血量越低伤害越高
                    //总伤害 = 最终伤害*(1 + max(效果系数最小值, （已损失血量百分比)*a))
                    theRatio = theEffAlphaValue * (theTarget->getHPTop() - theTarget->getHP()) / theTarget->getHPTop();
                }
            }

            if (theRatio < theEffMinValue)
                theRatio = theEffMinValue;
            theRatio = theRatio / 10000.0f;
            theExtraHurt += (float)theHpDelta * theRatio;
        }

        return theExtraHurt;
    }

    int AIEffect269::getHitLabel(AIContext &aiContext, CharacterData *theTarget)
    {
        auto iter = theTarget->m_aiEffectMap.find(eEFFECT_CAUSE_HURT_BY_CURR_HP);
        if (iter == theTarget->m_aiEffectMap.end())
            return 0;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto theEff = (AIEffect269 *)(*effIter);
            if (!theEff || theEff->isTempInvalid()) continue;
            if (theEff->innerExec->label)
            {
                return theEff->innerExec->label;
            }
        }

        return 0;
    }

    AIEffect274::AIEffect274(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect274::~AIEffect274()
    {
        // onExit();
    }

    bool AIEffect274::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _changeValue = getBoundedChangeValue(caculateChangeValue(), innerExec, skillLevel);

        auto theCopyTarget = AIActionController::getRandomCharacter(aiContext, *target, target->getOriginRivals(), true, true);
        if (!theCopyTarget)
            return false;

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        if (aiContext.animationProxy)
            aiContext.animationProxy->enterCopyCharacter(aiContext, target, theCopyTarget);

        if (innerExec->params.size() >= 3)
        {
            _transType = innerExec->params[0]; //转移规则 0-不转移 1-效果目标受到的伤害转移给拷贝对象 拷贝对象受到的治疗转移给效果目标 2-只转移伤害 3-只转移治疗
            _transLimit = innerExec->params[1]; //转移限制规则 0-没有限制 1-双方任一人处于魅惑 决斗中时都不转移
            _transExecType = innerExec->params[2]; //转移执行类型 0-伤害执行和效果执行 1-伤害执行 2-效果执行
            _transInterval = innerExec->params[3]; //转移时间间隔
        }

        if (_transType != 0)
        {
            _copyCharacterMyId = theCopyTarget->myID;
            if (_transType == 1 || _transType == 2)
            {
                //目标转移伤害给拷贝对象
                target->addHurtTransFunc(getUID(), [this](AIContext &theAiContext, CharacterData *theHurtTransfer, int theHpDelta, int theExecId)->CharacterData*{
                    if (theHpDelta <= 0) //模拟计算时负值为加血
                        return nullptr;
                    if (_transExecType == 1)
                    {
                        if (!IsHurtExec(theExecId))
                            return nullptr;
                    }
                    else if (_transExecType == 2)
                    {
                        if (!IsEffectExec(theExecId))
                            return nullptr;
                    }
                    auto theHurtReceiver = theAiContext.getCharacterByID(_copyCharacterMyId);
                    if (!theHurtReceiver)
                        return nullptr;
                    if (_transLimit != 0)
                    {
                        if (theHurtTransfer)
                        {
                            if (theHurtTransfer->isInCharmEffect() || theHurtTransfer->isInHalfCharmEffect() || theHurtTransfer->hasEffect(eEFFECT_TAUNT_EXTRA) || theHurtTransfer->hasTag("summonincharm253"))
                                return nullptr;
                        }

                        if (theHurtReceiver->isInCharmEffect() || theHurtReceiver->isInHalfCharmEffect() || theHurtReceiver->hasEffect(eEFFECT_TAUNT_EXTRA) || theHurtReceiver->hasTag("summonincharm253"))
                            return nullptr;
                    }
                    if (_transInterval > 0)
                    {
                        if (_lastTransParentLocalTick >= 0 && parentCaster->localTick <= _lastTransParentLocalTick + _transInterval)
                        {
                            return nullptr;
                        }
                    }
                    if (aiContext.localRandom->nextInt(0, 100) < _changeValue)
                    {
                        _lastTransParentLocalTick = parentCaster->localTick;
                        return theHurtReceiver;
                    }

                    return nullptr;
                });
            }

            if (_transType == 1 || _transType == 2)
            {
                //拷贝对象转移治疗给目标
                theCopyTarget->addHurtTransFunc(getUID(), [this](AIContext &theAiContext, CharacterData *theHealTransfer, int theHpDelta, int theExecId)->CharacterData*{
                    if (theHpDelta >= 0)
                        return nullptr;
                    if (_transExecType == 1)
                    {
                        if (!IsHurtExec(theExecId))
                            return nullptr;
                    }
                    else if (_transExecType == 2)
                    {
                        if (!IsEffectExec(theExecId))
                            return nullptr;
                    }
                    auto theHealReceiver = getTargetSafe();
                    if (!theHealReceiver)
                        return nullptr;
                    if (_transLimit != 0)
                    {
                        if (theHealTransfer)
                        {
                            if (theHealTransfer->isInCharmEffect() || theHealTransfer->isInHalfCharmEffect() || theHealTransfer->hasEffect(eEFFECT_TAUNT_EXTRA) || theHealTransfer->hasTag("summonincharm253"))
                                return nullptr;
                        }

                        if (theHealReceiver->isInCharmEffect() || theHealReceiver->isInHalfCharmEffect() || theHealReceiver->hasEffect(eEFFECT_TAUNT_EXTRA) || theHealReceiver->hasTag("summonincharm253"))
                            return nullptr;
                    }
                    if (_transInterval > 0)
                    {
                        if (_lastTransParentLocalTick >= 0 && parentCaster->localTick <= _lastTransParentLocalTick + _transInterval)
                        {
                            return nullptr;
                        }
                    }

                    if (aiContext.localRandom->nextInt(0, 100) < _changeValue)
                    {
                        _lastTransParentLocalTick = parentCaster->localTick;
                        return theHealReceiver;
                    }
                    return nullptr;
                });
            }
        }

        _ignoreHitLabel = true;

        return true;
    }

    void AIEffect274::onExit()
    {
        auto theSafeTarget = getTargetSafe();
        if (aiContext.animationProxy && theSafeTarget)
        {
            aiContext.animationProxy->exitCopyCharacter(aiContext, theSafeTarget);
        }

        if (_transType != 0)
        {
            theSafeTarget->invalidateHurtTransFunc(getUID());
            auto theCopyTarget = aiContext.getCharacterByID(_copyCharacterMyId);
            if (theCopyTarget)
                theCopyTarget->invalidateHurtTransFunc(getUID());
        }
    }

    AIEffect276::AIEffect276(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect276::~AIEffect276()
    {
        // onExit();
    }

    bool AIEffect276::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        _undividedChangeValue = calculateUndividedChangeValue();
        if (innerExec->changeType == 1)
        {
            double teamHp = 0;
            auto theFriends = aiContext.getClusterOrigin(target->teamSide, target->clusterNum);
            for (auto aFriend : *theFriends)
            {
                if (aFriend->isSummon() || aFriend->isCaptain())
                    continue;
                teamHp += aFriend->getHPTop();
            }
            double ratio = (double)_undividedChangeValue / 10000.0;
            _changeValue = teamHp * ratio;
        }
        else
        {
            _changeValue = _undividedChangeValue / 100.0f;
        }
        _changeValue = getBoundedChangeValue(_changeValue, innerExec, skillLevel);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        aiContext.addTeamHPLoseListener(target->teamSide, target->myID);

        return true;
    }

    void AIEffect276::onExit()
    {
        auto theSafeTarget = getTargetSafe();
        if (theSafeTarget)
        {
            aiContext.invalidateTeamHPLoseListenerFromBegin(theSafeTarget->teamSide, theSafeTarget->myID);
        }
    }

    int AIEffect276::getTeamHpLose()
    {
        return _teamHpLose;
    }

    void AIEffect276::addTeamHpLose(int hplose)
    {
        _teamHpLose += hplose;
    }

    void AIEffect276::resetTeamHpLose()
    {
        _teamHpLose = 0;
    }

    void AIEffect276::triggerEffect()
    {
        for (auto execId : innerExec->params)
        {
            AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }

    void AIEffect276::checkExecute(AIContext &aiContext, CharacterData *theTarget, CharacterData *theTrigger, int hplose)
    {
        auto iter = theTarget->m_aiEffectMap.find(eEFFECT_CAST_BY_TEAM_LOSE_HP);
        if (iter == theTarget->m_aiEffectMap.end())
            return;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto theEff = (AIEffect276 *)(*effIter);
            if (!theEff || theEff->isTempInvalid()) continue;
            theEff->addTeamHpLose(hplose);
            if (theEff->getChangeValue() > 0 && theEff->getTeamHpLose() >= theEff->getChangeValue())
            {
                theEff->resetTeamHpLose();
                theEff->triggerEffect();
            }
        }
    }

    AIEffect277::AIEffect277(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect277::~AIEffect277()
    {
        // onExit();
    }

    bool AIEffect277::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        if (innerExec->params.size() < 3)
            return false;

        _undividedChangeValue = calculateUndividedChangeValue();
        int theAffectSkillId = innerExec->params[2];
        auto theSkillData = AIDataHelper::getSkillData(theAffectSkillId);
        if (innerExec->changeType == 1)
        {
            if (theSkillData)
                _changeValue = theSkillData->castParams[0] * _undividedChangeValue / 10000.f;
        }
        else
        {
            _changeValue = _undividedChangeValue / 100.0f;
        }

        _changeValue = getBoundedChangeValue(_changeValue, innerExec, skillLevel);

        int theAffectTeamSideRelationship = innerExec->params[0]; //0-友军 1-敌军 2-全体
        int theAffectCharacterId = innerExec->params[1];
        if (theAffectTeamSideRelationship == 0 || theAffectTeamSideRelationship == 2)
        {
            auto theFriends = target->getOriginFriends();
            for (auto aFriend : *theFriends)
            {
                if (aFriend->getID() == theAffectCharacterId)
                    aFriend->addAlternateAccuMaxTick(theAffectSkillId, getUID(), _changeValue);
            }
        }

        if (theAffectTeamSideRelationship == 1 || theAffectTeamSideRelationship == 2)
        {
            auto theRivals = target->getOriginRivals();
            for (auto aRival : *theRivals)
            {
                if (aRival->getID() == theAffectCharacterId)
                    aRival->addAlternateAccuMaxTick(theAffectSkillId, getUID(), _changeValue);
            }
        }

        //target->addAlternateAccuMaxTick(theAffectSkillId, getUID(), _changeValue);

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIEffect277::onExit()
    {
        if (aiContext.isBeingFinalClear())
            return;
        if (innerExec->exitClear == 1)
        {
            auto theSafeTarget = getTargetSafe();
            if (theSafeTarget)
            {
                int theAffectTeamSideRelationship = innerExec->params[0]; //0-友军 1-敌军 2-全体
                int theAffectCharacterId = innerExec->params[1];
                int theAffectSkillId = innerExec->params[2];
                if (theAffectTeamSideRelationship == 0 || theAffectTeamSideRelationship == 2)
                {
                    auto theFriends = target->getOriginFriends();
                    for (auto aFriend : *theFriends)
                    {
                        if (aFriend->getID() == theAffectCharacterId)
                            aFriend->invalidateAlternateAccuMaxTick(theAffectSkillId, getUID());
                    }
                }

                if (theAffectTeamSideRelationship == 1 || theAffectTeamSideRelationship == 2)
                {
                    auto theRivals = target->getOriginRivals();
                    for (auto aRival : *theRivals)
                    {
                        if (aRival->getID() == theAffectCharacterId)
                            aRival->invalidateAlternateAccuMaxTick(theAffectSkillId, getUID());
                    }
                }
            }
        }
    }

    AIEffect278::AIEffect278(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect278::~AIEffect278()
    {
        // onExit();
    }

    bool AIEffect278::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        return true;
    }

    void AIEffect278::onExit()
    {

    }

    int AIEffect278::getExtraHpSuck(AIContext &aiContext, CharacterData *theAttacker, CharacterData *theTarget)
    {
        if (HeroActionController::isDeadAction(theAttacker->currentAction))
            return 0;

        auto iter = theAttacker->m_aiEffectMap.find(eEFFECT_CHANGE_HPADD1_BY_MARK);
        if (iter == theAttacker->m_aiEffectMap.end())
            return 0;

        int theExtraHpSuck = 0;
        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto theEff = (AIEffect278 *)(*effIter);
            if (!theEff || theEff->isTempInvalid()) continue;
            int theBasicExtra = theEff->innerExec->params[0]; //基础提升的吸血等级
            int theMarkId = theEff->innerExec->params[1]; //指定标记ID
            int theExtraAddPerMark = theEff->innerExec->params[2]; //每层标记额外增加的吸血等级

            theExtraHpSuck += theBasicExtra;
            for (auto markIter = theTarget->markCasts.begin(); markIter != theTarget->markCasts.end(); ++markIter)
            {
                auto aMark = *markIter;
                if (aMark->markID == theMarkId)
                {
                    theExtraHpSuck += aMark->getMarkLevel() * theExtraAddPerMark;
                }
            }
        }

        return theExtraHpSuck;
    }

    AIEffect279::AIEffect279(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect279::~AIEffect279()
    {
        // onExit();
    }

    bool AIEffect279::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _innerTriggerCntLowerLimit = innerExec->changeValue;
        _innerTriggerCd = innerExec->levelup;
        _innerInterval = innerExec->valueBase;

        if (target->currentAction == innerExec->targetAttribute)
        {
            onStartTrigger();
        }

        return true;
    }

    void AIEffect279::onExit()
    {
        auto theSafeTarget = getTargetSafe();
        if (theSafeTarget)
        {
            theSafeTarget->invalidateExtraUpdateFunc(getUID());
        }
    }

    void AIEffect279::onStartTrigger()
    {
        if (_innerTriggerCd > 0)
        {
            if (_innerLastTrigParentTick >= 0 && parentCaster->localTick <= _innerLastTrigParentTick + _innerTriggerCd)
            {
                return;
            }
        }

        _innerPreTriggerCnt += 1;
        if (_innerTriggerCntLowerLimit > 0 && _innerPreTriggerCnt < _innerTriggerCntLowerLimit)
        {
            return;
        }

        castInnerExec();

        _innerPreTriggerCnt = 0;
        _innerLastTrigParentTick = parentCaster->localTick;
        _innerLocalTick = 0;
        target->invalidateExtraUpdateFunc(getUID());
        if (_innerInterval > 0)
        {
            target->addExtraUpdateFunc(getUID(), [this](AIContext &theAiContext){
                _innerLocalTick += 1;
                if (_innerInterval > 0 && _innerLocalTick % _innerInterval == 0)
                {
                    castInnerExec();
                }
            });
        }
    }

    void AIEffect279::castInnerExec()
    {
        for (auto execId : innerExec->params)
        {
            if (execId > 0)
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
        }
    }

    void AIEffect279::checkExecuteEnterAction(AIContext &aiContext, CharacterData *theTarget, int theAction)
    {
        if (HeroActionController::isDeadAction(theTarget->currentAction))
            return;

        auto iter = theTarget->m_aiEffectMap.find(eEFFECT_CAST_BY_STATE);
        if (iter == theTarget->m_aiEffectMap.end())
            return;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto anEff = (AIEffect279 *)(*effIter);
            if (!anEff || anEff->isTempInvalid()) continue;
            if (theAction == anEff->innerExec->targetAttribute)
            {
                anEff->onStartTrigger();
            }
        }
    }

    void AIEffect279::checkExecuteExitAction(AIContext &aiContext, CharacterData *theTarget, int theAction)
    {
        auto iter = theTarget->m_aiEffectMap.find(eEFFECT_CAST_BY_STATE);
        if (iter == theTarget->m_aiEffectMap.end())
            return;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto anEff = (AIEffect279 *)(*effIter);
            theTarget->invalidateExtraUpdateFunc(anEff->getUID());
        }
    }


    AIEffect281::AIEffect281(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect281::~AIEffect281()
    {
        // onExit();
    }

    bool AIEffect281::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _casterMyIdsToNotifyOnExit.push_back(parentCaster->myID);

        if (innerExec->params.size() > 0)
        {
            auto theOldSummonCastCnt = target->summonCasts.size();
            auto theOldMoveCastCnt = target->moveCasts.size();
            auto theOldProjectileCastCnt = target->projectileCasts.size();
            auto theOldEffectCastCnt = target->effectCasts.size();
            auto theOldMarkCastCnt = target->effectCasts.size();

            // 效果参数预留前5位：效果中执行
            for (int i = 0; i < innerExec->params.size() && i < 5; i++)
            {
                auto anExecId = innerExec->params[i];
                if (anExecId > 0)
                {
                    AIActionController::executeExec(aiContext, anExecId, skillID, skillLevel, origin, projectile, target, 0, false, parentCaster->markID);
                }
            }

            if (target->summonCasts.size() > theOldSummonCastCnt)
            {
                int theCnt = 1;
                for (auto aCast : target->summonCasts)
                {
                    if (theCnt > theOldSummonCastCnt)
                    {
                        _casterMyIdsToNotifyOnExit.push_back(aCast->myID);
                    }

                    theCnt++;
                }
            }

            if (target->moveCasts.size() > theOldMoveCastCnt)
            {
                int theCnt = 1;
                for (auto aCast : target->moveCasts)
                {
                    if (theCnt > theOldMoveCastCnt)
                    {
                        _casterMyIdsToNotifyOnExit.push_back(aCast->myID);
                    }
                    theCnt++;
                }
            }

            if (target->projectileCasts.size() > theOldProjectileCastCnt)
            {
                int theCnt = 1;
                for (auto aCast : target->projectileCasts)
                {
                    if (theCnt > theOldProjectileCastCnt)
                    {
                        _casterMyIdsToNotifyOnExit.push_back(aCast->myID);
                    }
                    theCnt++;
                }
            }

            if (target->effectCasts.size() > theOldEffectCastCnt)
            {
                int theCnt = 1;
                for (auto aCast : target->effectCasts)
                {
                    if (theCnt > theOldEffectCastCnt)
                    {
                        _casterMyIdsToNotifyOnExit.push_back(aCast->myID);
                        _effectCastsToRemoveOnExit[aCast->myID] = true;
                    }
                    theCnt++;
                }
            }

            if (target->markCasts.size() > theOldMarkCastCnt)
            {
                int theCnt = 1;
                for (auto aCast : target->markCasts)
                {
                    if (theCnt > theOldMarkCastCnt)
                    {
                        _casterMyIdsToNotifyOnExit.push_back(aCast->myID);
                    }
                    theCnt++;
                }
            }
        }

        for (auto anExecId : _casterMyIdsToNotifyOnExit)
        {
            target->addCanCastMyIdInEffect281Action(anExecId);
        }

        _oldAction = target->currentAction;
        _oldActionRemainTick = target->totalTick - (target->localTick - target->startTick);

        _effectStateAnimation = innerExec->targetAttribute;
#ifndef SERVER_VERIFYING
        if (_effectStateAnimation > 0)
        {
            if (aiContext.animationProxy)
            {
                aiContext.animationProxy->enterStateAnimation(aiContext, target, _effectStateAnimation, parentCaster->staticData->red, parentCaster->staticData->green, parentCaster->staticData->blue, parentCaster->staticData->stateAnimationParam);
            }
        }
#endif
        return true;
    }

    void AIEffect281::onExit()
    {
        //if (innerExec->exitClear > 0)
        {
            // 12.0
            if (aiContext.isBeingFinalClear())
                return;

            auto theSafeTarget = getTargetSafe();
            if (!theSafeTarget)
                return;

            // 12.0
            if (!aiContext.getCharacterByID(getOriginMyId()))
                return;

#ifndef SERVER_VERIFYING
            if (aiContext.animationProxy && _effectStateAnimation > 0)
            {
                aiContext.animationProxy->exitStateAnimation(aiContext, theSafeTarget, _effectStateAnimation);
            }

            if (theSafeTarget->currentAction == EFFECT_ACTION)
            {
                auto theCurrentEffectId = HeroActionController::getEffectID(theSafeTarget->actionAtt);
                if (theCurrentEffectId > 0)
                {
                    auto theCurrentEffect = AIDataHelper::getEffectExec(theCurrentEffectId);
                    if (aiContext.animationProxy && theCurrentEffect && theCurrentEffect->stateAnimation > 0)
                    {
                        aiContext.animationProxy->enterStateAnimation(aiContext, theSafeTarget, theCurrentEffect->stateAnimation, theCurrentEffect->red, theCurrentEffect->green, theCurrentEffect->blue, theCurrentEffect->stateAnimationParam);
                    }
                }
            }
#endif

            if (_oldActionRemainTick > 0 && _oldAction >= 0 && _oldAction == theSafeTarget->currentAction)
            {
                theSafeTarget->startTick = theSafeTarget->localTick + _oldActionRemainTick - theSafeTarget->totalTick;
            }

            for (auto anExecId : _casterMyIdsToNotifyOnExit)
            {
                theSafeTarget->removeCanCastMyIdInEffect281Action(anExecId);
            }

            for (auto aCast : theSafeTarget->effectCasts)
            {
                if (_effectCastsToRemoveOnExit.find(aCast->myID) != _effectCastsToRemoveOnExit.end())
                {
                    aCast->shouldDeleteSelf = true;
                }
            }

            if (innerExec->params.size() > 5 && !HeroActionController::isDeadAction(theSafeTarget->currentAction)/*12.0*/)
            {
                // 效果参数预6~10位：效果结束时执行
                for (int i = 5; i < innerExec->params.size() && i < 10; i++)
                {
                    auto anExecId = innerExec->params[i];
                    if (anExecId > 0)
                        AIActionController::executeExec(aiContext, anExecId, skillID, skillLevel, origin, projectile, theSafeTarget, 0, false, parentCaster->markID);
                }
            }
        }
    }

    void AIEffect281::checkExecuteEnterAction(AIContext &aiContext, CharacterData *target)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_FREEZE_IN_EFFECT_ACTION);
        if (iter == target->m_aiEffectMap.end()) { return; }

        for (auto anEffIter = iter->second.begin(); anEffIter != iter->second.end(); ++anEffIter)
        {
            auto aEff = (AIEffect281 *)(*anEffIter);
            if (!aEff || aEff->isTempInvalid()) continue;
            aEff->_oldAction = target->currentAction;
            aEff->_oldActionRemainTick = target->totalTick - (target->localTick - target->startTick);
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            AIEffect508  28.7
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect508::AIEffect508(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect508::~AIEffect508()
    {
        // onExit();
    }

    bool AIEffect508::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _leftExecuteNum = innerExec->params[0];

        return true;
    }

    void AIEffect508::onExit()
    {

    }

    void AIEffect508::onScheduleTick() {
        _triggerHealCD--;
    }

    void AIEffect508::checkExecuteHeal(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget, int addValue)
    {
        if (this->isTempInvalid())
            return;
        // 1 CD 期间不能执行  pExec->ID , 2 剩余执行次数不足 3 超过可执行的tick 
        if (_triggerHealCD > 0 || _leftExecuteNum <= 0)
            return ;

        _leftExecuteNum--;

        for (int i = 2; i < innerExec->params.size(); ++i) {
            int execId = innerExec->params[i];
            if (execId > 0)
            {
                AIActionController::executeExec(aiContext, execId, skillID, skillLevel, origin, projectile, target, 0);
            }
        }

        AIContinuousEffectCaster::checkShieldEffectAdd(aiContext, tmpSource, tmpTarget, addValue);

        //剩余触发次数用完，删除本效果
        if (_leftExecuteNum <= 0) {
            //AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, innerExec->ID);
            return;
        }

       
        _triggerHealCD = innerExec->params[1];  //触发CD
        target->addExtraUpdateFunc(getUID(), [this](AIContext& theAiContext) {
            _triggerHealCD--;
            if (_triggerHealCD == 0) {
                target->invalidateExtraUpdateFunc(getUID());
            }
            });
        
        //_triggerHealCD = 20;
        
        //AIContinuousEffectCaster::startEffectCD(*target, innerExec);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                            AIEffect509  28.7
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   castInnerExec

    AIEffect509::AIEffect509(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect509::~AIEffect509()
    {
        // onExit();
    }

    bool AIEffect509::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();


        //判断和设置治疗溢出减伤盾

        auto iter =  target->m_aiEffectMap.find(eEFFECT_HEAL_SHIELD);
        if (iter == target->m_aiEffectMap.end()) {  //首次添加
            return true;
        }
            
        auto& effects = iter->second;
        for (auto iter = effects.begin(); iter != effects.end(); iter++)
        {
            auto pEff = static_cast<AIEffect509*>(*iter);
            //不同组的不执行,添加失败
            if (effectExec->groupID != pEff->effectExec->groupID) {
                return false;
            }
            //组相等时，累计之前的血量
            _shieldValue = pEff->getShieldValue();
            //不在执行消失效果
            pEff->setInnerData("is_broken", 1);
            //覆盖原有效果
            AIContinuousEffectCaster::cover_Special_Effect(aiContext, *target, innerExec->ID);
            return true;
        }
        //AIContinuousEffectCaster::startEffectCD(*target, innereExec);

        return true;
    }

    void AIEffect509::onExit()
    {
        if (getInnerData("is_broken") == 1) {
            return;
        }
        AIActionController::executeExec(aiContext, innerExec->params[1], skillID, skillLevel, origin, projectile, target, 0);
    }

    void AIEffect509::checkCleanSameTypeExec()
    {

        if (innerExec->onlySameType != 1)
            return;

    }

    bool AIEffect509::commonCheckVaild()
    {
        if (this->isTempInvalid())
        {
            return false;
        }
           
        // 1 CD 期间不能执行  pExec->ID 
        if (target->getEffectRemainCD(innerExec->ID) > 0)
        {
            return false;
        }
        return true;
    }
    /*
       检测增加护盾值
       params:
       0:护盾的被打破触发执行id
       1:护盾持续时间结束执行的id

       2:物理伤害开关
       3:魔法伤害开关
       4:纯粹伤害开关

       5:释放执行的施法者

    
    */
    void AIEffect509::checkShieldAdd(int addValue)
    {
        float per = float(innerExec->changeValue + innerExec->levelup * skillLevel) / 10000;
        _shieldValue += addValue* per; //; //护盾增加值， 是否会随技能等级而提升 cui_to_do

        /*
           a: limitOnOff
           b: limitValLowerBase
           c: limitValLowerGrowth
           d: limitValUpperBase
        */

        int maxValue = (innerExec->limitValLowerGrowth + innerExec->limitValUpperBase * skillLevel)/100;
        if (innerExec->limitOnOff == 1)//percentage
        {
            int valueBase = innerExec->limitValLowerBase;
            if (valueBase > 0 && valueBase < ATT_NUM)
            {
                auto realTarget = target;
                if (innerExec->params[5] > 0) {
                    realTarget = getOriginSafe();
                }
                if (realTarget) {
                    auto tempValue = realTarget->attributes[valueBase];
                    maxValue = tempValue *(float(innerExec->limitValLowerGrowth + innerExec->limitValUpperBase * skillLevel) / 10000);
                }
            }
        }

        _shieldValue = std::min(_shieldValue, maxValue);
        //_shieldValue = this->getBoundedChangeValue(_shieldValue, innerExec, skillLevel);
        this->setShieldValue(_shieldValue);
        
    }

    int AIEffect509::doAbsorbHurt(int hurt) 
    {
        int change = _shieldValue >= hurt ? hurt : _shieldValue;
        _shieldValue = _shieldValue - change;
        if (_shieldValue <= 0) { //被打破
            _shieldValue = 0;
            setInnerData("is_broken", 1);
            AIActionController::executeExec(aiContext, innerExec->params[0], skillID, skillLevel, origin, projectile, target, 0);
            //被打破清除自身
            AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, innerExec->ID);
        }

        return change;
    }

    //护盾值吸收伤害
    int AIEffect509::checkAbsorbHurt(AIContext& aiContext, CharacterData* target ,int hurt, int hurtType,bool isPureHurt)
    {

        auto iter = target->m_aiEffectMap.find(eEFFECT_HEAL_SHIELD);
        if (iter == target->m_aiEffectMap.end())
            return 0;

        for (auto effIter = iter->second.begin(); effIter != iter->second.end(); ++effIter)
        {
            auto anEff = (AIEffect509*)(*effIter);
            if (!anEff || anEff->isTempInvalid()) continue;

            if ((hurtType == HurtType::PHYSICAL_HURT_TYPE && anEff->innerExec->params[2])
                || (hurtType == HurtType::MAGICAL_HURT_TYPE && anEff->innerExec->params[3])
                || (isPureHurt && anEff->innerExec->params[4]))
            {
                return anEff->doAbsorbHurt(hurt);
            }
        }

        return  0;
            
    }


     //                                                            AIEffect511  28.7
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    AIEffect511::AIEffect511(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffect511::~AIEffect511()
    {
        // onExit();
    }

    bool AIEffect511::onEnter()
    {
        if (isDebuffed())
            return false;

        checkCleanSameTypeExec();

        AIContinuousEffectCaster::startEffectCD(*target, innerExec);

        _leftEffectTime = this->innerExec->params[0];

        return true;
    }

    void AIEffect511::onExit()
    {

    }
    /*
        [0] 生效次数
        [1] 效果id 用于获取被作用单位
        [...]生效的伤害类型
    */
    int AIEffect511::checkExecute(AIContext& aiContext, CharacterData* target, CharacterData* source, int HPDelta, int propetyType, bool isSimulate)
    {
        auto iter = target->m_aiEffectMap.find(eEFFECT_SPURTING_HURT);
        if (iter == target->m_aiEffectMap.end()) {  
            return true;
        }

        auto& effects = iter->second;
        for (auto iter = effects.begin(); iter != effects.end(); iter++)
        {
            auto pEff = static_cast<AIEffect511*>(*iter);
            pEff->checkAndDoEfect(aiContext, target, source, HPDelta, propetyType,isSimulate);
            return true;
        }
    }
    void AIEffect511::checkAndDoEfect(AIContext& aiContext, CharacterData* target, CharacterData* source, int HPDelta, int propetyType, bool isSimulate)
    {
        if (this->_leftEffectTime == 0) {
            return;
        }
        bool canPlay = false;
        for (int i = 2; i < innerExec->params.size(); ++i) {
            if (innerExec->params[i] == propetyType)
            {
                canPlay = true;
                break;
            }
        }
        if (canPlay) {
            //AIDataHelper::getHurtExec
            HurtExec* staticData = AIDataHelper::getHurtExec(innerExec->params[1]);


            auto currentTargets = AIActionController::searchTarget(*target, target->target, nullptr, aiContext, 2, staticData->filterX1, staticData->filterX2, staticData->filterZ1, staticData->filterZ2, staticData->filterZ1, staticData->filterZ2, &staticData->filterParams, *target, false, skillID);
            //对应目标被溅射
            for (auto iter = currentTargets->begin(); iter != currentTargets->end(); ++iter)
            {
                CharacterData* tempTarget = (*iter);
                if (tempTarget == target) {
                    continue;
                }
                //  
                if (tempTarget->hasEffect(EffectType::eEFFECT_HP_LOCK) ||
                    tempTarget->hasEffect(EffectType::eEFFECT_KO_BY_COUNT) ) {
                    continue;
                }

          

                float changeValue = innerExec->changeValue/100.0f;
                changeValue = (float)(changeValue + innerExec->levelup * skillLevel) / 100.0f;
                changeValue = int(changeValue * HPDelta);

                if (tempTarget->hasEffect(EffectType::eEFFECT_FLIMSY_BURYING)) {
                    changeValue = AIFlimsyBuryingEffect::activateFilmsyBuringEffect(tempTarget, changeValue, isSimulate);
                }



                //HeroHurtcalculate::HurtFuncHurtTarget(aiContext, theSource, theTarget, param, &result);
                int curHp = tempTarget->getHP();
                //tempTarget->onHurt(true, innerExec->ID, curHp, innerExec->hurtType - 1, origin, parentCaster->effectID);
                tempTarget->updateHP(curHp - changeValue);
                if (tempTarget->getHP() == 0) {
                    changeValue = curHp;
                    StatsFunc::AppendHeroHurt(*origin, HurtType::PHYSICAL_HURT_TYPE, curHp, *tempTarget);
                    tempTarget->onHPDelta(tempTarget->getHP() - curHp);
                    HeroHurtcalculate::characterDeadByHurt(aiContext, tempTarget, source, skillID, skillLevel, nullptr, innerExec);
                }
 
                //飘掉血数字
                if (changeValue) {
                    AISpecialEffectLabel::excute(NORMAL_HURT_LABEL, -changeValue, aiContext, tempTarget);
                } 
            }


            if (this->_leftEffectTime > 0) {
                this->_leftEffectTime--;
            }

            if (this->_leftEffectTime == 0) {
                AIContinuousEffectCaster::zero_Special_Effect(aiContext, *target, innerExec->ID);
            }
        }
        
    }
  
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    AIEffectScript::AIEffectScript(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec) : AIInnerEffect(theParent, theTarget, theExec)
    {

    }

    AIEffectScript::~AIEffectScript()
    {

    }

    bool AIEffectScript::onEnter()
    {
        return luahelper_innereffect_onenter(aiContext.getLuaState(), this);
    }

    void AIEffectScript::onExit()
    {
        luahelper_innereffect_onexit(aiContext.getLuaState(), this);
    }

    void AIEffectScript::onScheduleTick()
    {
        luahelper_innereffect_onscheduletick(aiContext.getLuaState(), this);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //                                                            EffectHelper
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace EffectHelper {

        void RestoreCharacterBAMAPower(CharacterData* hero)
        {
            if (!hero)
                return;

            auto attrCache = CharacterFunc::GetAttrValue(hero->getID(), ECharacterAttr::PHY_AT);
            if (attrCache.found)
                hero->setBAPower(attrCache.value);

            attrCache = CharacterFunc::GetAttrValue(hero->getID(), ECharacterAttr::MAG_AT);
            if (attrCache.found)
                hero->setMAPower(attrCache.value);
        }

        bool IsEffectStateAnimationSupportMix(int stateAnimation)
        {
            switch (stateAnimation)
            {
            case 4://4-待机扩展
            case 5://5-当前帧并且染色扩展
            case 7://7-当前帧+石化shader扩展
            case 8://8-当前帧,可叠加
                return true;
            default:break;
            }
            return false;
        }

        int applyNoneHPChange(CharacterData *pTarget, int attributeIndex, int changeValue)
        {
            if (attributeIndex == ECharacterAttr::ENG && changeValue > 0 && pTarget->hasTag("energyskill"))
                return 0;
            if (attributeIndex == ECharacterAttr::ENG && pTarget->hasEffect(315))
            {
                int absorben = luahelper_innereffect_315_trigger(AIContext::getInstance()->getLuaState(), pTarget, changeValue);
                if (changeValue > 0)
                {
                    changeValue -= absorben;
                    if (changeValue <= 0)
                    {
                        return 0;
                    }
                }
                else
                {
                    changeValue -= absorben;
                    if (changeValue >= 0)
                    {
                        return 0;
                    }
                }
            }
            int oldValue = pTarget->attributes[attributeIndex];
            int minValue = 0;
            if (attributeIndex == ECharacterAttr::ATT_INTER)
                minValue = 2;
            if (pTarget->attributes[attributeIndex] + changeValue > minValue)
                pTarget->attributes[attributeIndex] += changeValue;
            else
                pTarget->attributes[attributeIndex] = minValue;

			EffectHelper::limitAttr(AIContext::getInstance(), pTarget, attributeIndex);

            int attrDelta = pTarget->attributes[attributeIndex] - oldValue;

            if (attributeIndex == ECharacterAttr::ENG)
            {
                pTarget->onENDelta(attrDelta);
            }

            return attrDelta;
        }

        int applyHPChange(AIContext &aiContext, CharacterData *pSource, AIProjectile *pProjectile, CharacterData *pTarget, int skillID, int skillLevel, int attributeIndex, int changeValue, InnerEffectExec *pInnerExec, int markID, int casterID, AIInnerEffect *pInnerEffect)
        {
            if (AIScriptBinder::getInstance()->isInnerEffectApplyHpchangeByLua(aiContext.getLuaState()))
                return luahelper_innereffect_apply_hpchange(aiContext.getLuaState(), pSource, pProjectile, pTarget, skillID, skillLevel, attributeIndex, changeValue, markID, casterID, pInnerEffect);
            return applyHPChangeNative(aiContext, pSource, pProjectile, pTarget, skillID, skillLevel, attributeIndex, changeValue, pInnerExec, markID, casterID, pInnerEffect);
        }

        int applyHPChangeNative(AIContext &aiContext, CharacterData *pSource, AIProjectile *pProjectile, CharacterData *pTarget, int skillID, int skillLevel, int attributeIndex, int changeValue, InnerEffectExec *pInnerExec, int markID, int casterID, AIInnerEffect *pInnerEffect)
        {
            int realChangeValue = 0;

            if (changeValue > 0)
            {
                int oldHP = pTarget->getHP();
                HeroHurtcalculate::HurtFuncHealTarget(aiContext, *pSource, *pTarget, changeValue, 0 /*击中文字会在外部统一处理*/, skillID, false, 0);
                realChangeValue = pTarget->getHP() - oldHP;

				if (realChangeValue <= 0 && pTarget->getHP() <= 0)
				{
					HeroHurtcalculate::characterDeadByHurt(aiContext, pTarget, pSource, skillID, skillLevel, nullptr, pInnerExec);
				}
            }
            else
            {
                int oldValue = pTarget->getHP();
                int hurtExecType = pInnerExec->hurtType - 1;

                HurtCalcParam param(pProjectile, skillID, skillLevel, -changeValue, hurtExecType, casterID);
                param.ignoreHurtLabel = true;
                param.isFromEffectCaster = true;
                param.alternateHitLabel = pInnerExec->label;
                param.markID = markID;
                param.innerEffect = pInnerEffect;

                //cui_2023_02_08 28.7
                if (pInnerExec->ID == 11 && pInnerExec->hurtType == 0 && pInnerExec->changeValue < 0) {
                    param.isPureHurt = true;
                }
				
				// 215效果
                if (pInnerExec->ID == eEFFECT_CREATE_HURT_ADD_VALUE)
				{
					param.isRawHurt = true;
				}
				// 267 效果
				if (pInnerExec->ID == eEFFECT_CREATE_HURT_ADD_VALUE_NEW)
				{
					param.isRawHurt = true;
				}
				// 270 效果
				if (pInnerExec->ID == eEFFECT_CONSUME_HP_ADD_CRUITHURT)
				{
					param.isRawHurt = true;
				}

                if (pInnerExec->targetAttribute == ECharacterAttr::HP && pInnerExec->params.size() > 0)
                    param.hurtProperty = pInnerExec->params[0];

                //auto result = HeroHurtcalculate::HurtFuncHurtTargetScript(aiContext, pSource, pTarget, param);
                HitResult result;
                HeroHurtcalculate::HurtFuncHurtTarget(aiContext, pSource, pTarget, param, &result);

                if (result.blockResult != HitResult::BlockType::HIT_BLOCKED)
                    pTarget->onHurt(true, pInnerExec->ID, param.HPDelta, param.hurtType, pSource, casterID);

                // OP2 2.4: 先记录效果执行造成的实际伤害,防止后续触发标记的伤害量也被加到击中文字里
                realChangeValue = pTarget->getHP() - oldValue;

                if (pTarget->getHP() <= 0)
                {
                    HeroHurtcalculate::characterDeadByHurt(aiContext, pTarget, pSource, skillID, skillLevel, nullptr, pInnerExec);
                }
                else
                {
					if (param.hurtProperty > 0 && (result.hitResult == HitResult::HitResultType::HIT_HASHURT && result.blockResult != HitResult::BlockType::HIT_BLOCKED))
                    {
						if (param.hurtType == -1)
						{
							param.hurtType = 3; // 效果伤害里的纯粹伤害类型-1 ->> 标记执行纯粹伤害配置 类型：-1所有 0-物理 1-法术 3-纯粹伤害
						}
                        HeroHurtcalculate::ToTriggerMarkCasterFun(aiContext, *pTarget, 0, skillLevel, param.hurtType, param.hurtProperty, param.HPDelta, param.hitType, pSource);
                    }

                    HeroSkillController::checkReleaseHPSkill(aiContext, pTarget);
                }

                // MARK:防止效果执行击杀目标但因103不死效果而出现加血的击中文字 -- sync from lua
                if ((pTarget->getHP() - oldValue == 1) && pTarget->hasEffect(103) && pInnerEffect)
                {
                    realChangeValue = 0;
                }
            }

            return realChangeValue;
        }
        
        void applyAttrSwapEffect(int theInnerEffectUid, int theAttrIndex, CharacterData *theTarget, int theSwapperAttrValue)
        {
            AIAttributeEffectResult aResult(theInnerEffectUid, theAttrIndex);
            aResult.applySwapResult(theTarget, theSwapperAttrValue);
            theTarget->attrEffectResults[theAttrIndex].push_back(aResult);
        }
        
        void deapplyAttrEffect(int theInnerEffectUid, int theAttrIndex, CharacterData *theTarget)
        {
            if (!theTarget)
                return;
            
            auto &&effectResults = theTarget->attrEffectResults[theAttrIndex];
            bool startReapply = false;
            for (auto iter = effectResults.begin(); iter != effectResults.end();)
            {
                auto &&anEffectResult = *iter;
                // 正确的逻辑不可能出现同一内部效果对同一目标的同一属性值作用多次的情况
                if (anEffectResult.innerEffectUid == theInnerEffectUid)
                {
                    startReapply = true;
                    anEffectResult.deapply();
                    iter = effectResults.erase(iter);
                    continue;
                }
                else if (startReapply)
                {
                    anEffectResult.reapply();
                }
                
                ++iter;
            }
        }

        int checkExcuteCauseAddPropertyHurtResult(CharacterData *target, int hurtProperty, int param, bool isSimulate)
        {
            int etype = eEFFECT_CAUSE_ADD_PROPERTY_HURT;
            auto iter = target->m_aiEffectMap.find(etype);

            if (iter == target->m_aiEffectMap.end() || iter->second.size() <= 0 || target->getEffectRemainCD(etype) > 0)
                return 0;

            if (!isSimulate)
                AIContinuousEffectCaster::startEffectCD(*target, iter->second[0]->innerExec);

            int changeValue = 0;
            for (auto pEff : iter->second)
            {
                if (!pEff || pEff->isTempInvalid()) continue;
                if (pEff->innerExec->targetAttribute == hurtProperty)
                {
                    if (pEff->innerExec->changeType == 1)
                        changeValue += pEff->getChangeValue() * param / 100 | 0;
                    else
                        changeValue += pEff->getChangeValue();
                }
            }

            return changeValue;
        }

        bool isAnyInnerExecInCD(CharacterData *target, int effectExecID) {
            EffectExec *pExec = AIDataHelper::getEffectExec(effectExecID);
            if (!pExec) return false;
            for (auto &&innerExec : pExec->execs) {
                if (target->getEffectRemainCD(innerExec.ID) > 0)
                    return true;
            }
            return false;
        }

        int getExtendBuffTick(CharacterData *pTarget, int buffType, int oldTick) {
            auto iter = pTarget->m_aiEffectMap.find(eEFFECT_EXTEND_BUFF_TICK);
            if (iter == pTarget->m_aiEffectMap.end())
                return 0;
            int extendTick = 0;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid())
                    continue;
                if (pEff->innerExec->targetAttribute == buffType) {
                    if (pEff->innerExec->changeType == 1)
                        extendTick += oldTick * pEff->getChangeValue() / 100;
                    else
                        extendTick += pEff->getChangeValue();
                }
            }
            return extendTick;
        }

        bool isDebuffedByType(CharacterData *pTarget, int buffType) {
            auto iter = pTarget->m_aiEffectMap.find(eEFFECT_CLEAR_BUFF);
            if (iter == pTarget->m_aiEffectMap.end())
                return false;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                auto &&params = pEff->innerExec->params;
                auto findIter = std::find(params.begin(), params.end(), buffType);
                if (findIter != params.end())
                    return true;
            }
            return false;
        }

        int checkActivatePropHurtAdd(CharacterData *pTarget, int hurtProp, int hpdelta) {
            auto iter = pTarget->m_aiEffectMap.find(eEFFECT_PROP_HURT_ADD);
            if (iter == pTarget->m_aiEffectMap.end())
                return 0;
            int retdelta = 0;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                if (pEff->innerExec->targetAttribute != hurtProp)
                    continue;
                if (pEff->innerExec->changeType == 1)
                {
                    // 12.0
                    float addval = (float)hpdelta * (float)pEff->getChangeValue() / 100.0f;
                    retdelta += addval;
                }
                else
                    retdelta += pEff->getChangeValue();
            }
            return retdelta;
        }

        

        float checkExcuteFloatEffect(CharacterData *pTarget, int etype, float param) {
            auto iter = pTarget->m_aiEffectMap.find(etype);
            if (iter == pTarget->m_aiEffectMap.end() || iter->second.size() <= 0 || pTarget->getEffectRemainCD(etype) > 0)
                return 0;

            auto &effs = iter->second;
            for (auto &&pEff : effs)
            {
                //auto pEff = effs[0];
                if (dynamic_cast<AIFloatStateEffect *>(pEff) == nullptr || pEff->isTempInvalid())
                    continue;
    
                int changeType = pEff->innerExec->changeType;
                float changeValue = AIContinuousEffectCaster::getEffectValue(pTarget, etype);
                if (changeType == 1)
                    return changeValue * param / 10000.0f;
                else
                    return changeValue / 100.0f;
            }

            return 0;
        }

        void applyConvertAttrChange(AIContext &aiContext, CharacterData *pTarget, int firstAttrIndex, int firstAttrChange, int theInnerEffectUid, bool isDeapply) {
            int oldhp = pTarget->getHP();
            int oldhptop = pTarget->getHPTop();
            std::map<int, int> attrChangeValues;

            AIDataHelper::loadConverAttrListTable();
            auto &&attrVec = AIDataHelper::getConvertAttrVec();
            for (int i = 0; i < attrVec.size(); ++i) {
                ConvertAttrParams& convertAttrParams = attrVec[i];
                if (convertAttrParams.param0 != firstAttrIndex)
                    continue;

                int convertChange = firstAttrChange * convertAttrParams.param2;
                if (!convertChange)
                    continue;

                int convertIndex = convertAttrParams.param1;
//#if COCOS2D_DEBUG
//                if (convertIndex != ECharacterAttr::HP) {
//                    int oldConvertValue = pTarget->attributes[convertIndex];
//                    CCLOG("first attr:%d, old value:%d, new value:%d", convertIndex, oldConvertValue, oldConvertValue + convertChange);
//                }
//#endif
                if (convertIndex == ECharacterAttr::ENG && convertChange > 0 && pTarget->hasTag("energyskill"))
                    continue;
                if (convertIndex == ECharacterAttr::ENG && pTarget->hasEffect(315))
                {
                    int absorben = luahelper_innereffect_315_trigger(aiContext.getLuaState(), pTarget, convertChange);
                    if (convertChange > 0)
                    {
                        convertChange -= absorben;
                        if (convertChange <= 0)
                        {
                            continue;
                        }
                    }
                    else
                    {
                        convertChange -= absorben;
                        if (convertChange >= 0)
                        {
                            continue;
                        }
                    }
                }

                if (convertIndex == ECharacterAttr::HP)
                {
                    if ((HeroActionController::isDeadAction(pTarget->currentAction) || pTarget->hasTag("deadattacking")))
                        continue;
                    if (oldhp <= 0 && convertChange < 0)
                        continue;
                }

                if (convertIndex != PHY_AT && convertIndex != MAG_AT)
                {
                    pTarget->attributes[convertIndex] += convertChange;

					EffectHelper::limitAttr(AIContext::getInstance(), pTarget, convertIndex);

                    if (convertIndex == ECharacterAttr::ENG)
                    {
                        pTarget->onENDelta(convertChange);
                    }
                }
                else
                {
                    // MARK: 9.0 impl
                    if (!isDeapply)
                    {
                        pTarget->attributes[convertIndex] += convertChange;

						EffectHelper::limitAttr(AIContext::getInstance(), pTarget, convertIndex);
                        
                        AIAttributeEffectResult aResult(theInnerEffectUid, convertIndex);
                        aResult.changeType = AIAttributeEffectResult::ATTR_CHANGE_RAW;
                        aResult.target = pTarget;
                        aResult.attrValueOld = pTarget->attributes[convertIndex] - convertChange;
                        aResult.attrValueNew = pTarget->attributes[convertIndex];
                        pTarget->attrEffectResults[convertIndex].push_back(aResult);
                    }
                    else
                    {
                        EffectHelper::deapplyAttrEffect(theInnerEffectUid, convertIndex, pTarget);
                    }
                }

                if (attrChangeValues.find(convertIndex) == attrChangeValues.end())
                    attrChangeValues[convertIndex] = 0;
                attrChangeValues[convertIndex] += convertChange;
            }

            if (attrChangeValues.find(ECharacterAttr::MAX_HP) != attrChangeValues.end())
            {
                int maxhpChange = attrChangeValues[ECharacterAttr::MAX_HP];
                if (pTarget->getHPTop() <= 0) {
                    pTarget->setHPTop(1);
                    maxhpChange = pTarget->getHPTop() - oldhptop;
                    attrChangeValues[ECharacterAttr::MAX_HP] = maxhpChange;
                }

                if (!HeroActionController::isDeadAction(pTarget->currentAction))
                {
                    if (maxhpChange > 0)
                        AISpecialEffectLabel::excute(MAX_HP_INCREASE, maxhpChange, aiContext, pTarget);
                    else if (maxhpChange < 0)
                        AISpecialEffectLabel::excute(MAX_HP_DECREASE, maxhpChange, aiContext, pTarget);
                }
            }

            if (attrChangeValues.find(ECharacterAttr::HP) != attrChangeValues.end() && !HeroActionController::isDeadAction(pTarget->currentAction) && !pTarget->hasTag("deadattacking") && oldhp > 0) {
                // 由于1级属性变化导致的血量降低不能致死
                if (pTarget->getHP() <= 0)
                    pTarget->setHP(1);
                pTarget->updateHP(pTarget->getHP());
                int hpChange = pTarget->getHP() - oldhp;
                attrChangeValues[ECharacterAttr::HP] = hpChange;
                if (hpChange)
                    AISpecialEffectLabel::excute(hpChange > 0 ? NORMAL_HEAL_LABEL : NORMAL_HURT_LABEL, hpChange, aiContext, pTarget);
            }

        }

        int checkExcuteMixChangeTypeEff(CharacterData *pTarget, int efftype, int param) {
            auto iter = pTarget->m_aiEffectMap.find(efftype);
            if (iter == pTarget->m_aiEffectMap.end())
                return 0;
            
            int retvalue = 0;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                if (pEff->innerExec->changeType == 1) {
                    retvalue += pEff->getChangeValue() * param / 100;
                }
                else {
                    retvalue += pEff->getChangeValue();
                }
            }
            return retvalue;
        }
		
        void activateResistNReboundEff(AIContext &aiContext, CharacterData *target, CharacterData *reboundedTarget, int skillID, int skillLevel) {
            auto iter = target->m_aiEffectMap.find(eEFFECT_RESIST_N_REBOUND);
            if (iter == target->m_aiEffectMap.end())
                return;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                auto execID = pEff->innerExec->targetAttribute;
                if (execID > 0) {
                    ExecExcuteInfo execInfo(execID, skillID, skillLevel, pEff->origin, nullptr, target, 0);
                    aiContext.scheduleExcuteExec(execInfo);
                }
            }
        }

        int getAddHurtOnDistChangeValue(AIContext &aiContext, CharacterData *source, CharacterData *target, int hpdelta) {
            auto iter = source->m_aiEffectMap.find(eEFFECT_ADD_HURT_ON_DIST);
            if (iter == source->m_aiEffectMap.end())
                return 0;
            int changeValue = 0;
            int dist = HeroActionController::distanceToCharacter(source, target);
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                auto distInterval = pEff->innerExec->valueBase;
                if (pEff->innerExec->changeType == 1)
                    changeValue += pEff->getChangeValue() * hpdelta * dist / distInterval / 100;
                else
                    changeValue += pEff->getChangeValue() * dist / distInterval;
            }
            return changeValue;
        }

        int getExtendHitdownNFlyChangeValue(AIContext &aiContext, CharacterData *source, CharacterData *target, int tickLen) {
            auto iter = target->m_aiEffectMap.find(eEFFECT_EXTEND_HIT_FLYNDOWN);
            if (iter == target->m_aiEffectMap.end())
                return 0;

            /*
            公式：变化值百分比 = (施法者属性a - 目标属性a) * 系数C

            目标属性：比较的属性类型a
            数值计算基础：是否使用效果施法者与目标比较  0效果施法者  1攻击者
            效果数值：通用配置，支持上下限，代表公式中系数C，使用时除以100使用（例如填5，则代表0.05，注意计算时的精度）

            1.百分比情况下，变化值算出来要再除以100作为最终结果。例如算出来结果是12，则代表延长12%
            2.变化值直接加上去，用参数C的正负来控制加减；
            3.若施法者属性比目标属性小，当C大于等于0时，则不论是百分比还是数值，结果最小取30，即最少延长1.5秒；
            4.若施法者属性比目标属性小，当C小于0时，则不论是百分比还是数值，结果变化值最短让目标的击飞击倒时间减至0.5秒
            */
            int changeValue = 0;
            for (auto &&pEff : iter->second) {
                if (!pEff || pEff->isTempInvalid()) continue;
                auto attrIdx = pEff->innerExec->targetAttribute;
                int attrDiff = 0;
                if (pEff->innerExec->valueBase == 0)
                    attrDiff = pEff->origin->attributes[attrIdx] - target->attributes[attrIdx];
                else if (pEff->innerExec->valueBase == 1)
                    attrDiff = source->attributes[attrIdx] - target->attributes[attrIdx];

                //非百分比：时间延长值 = 属性差值 x 效果数值 / 100
                //百分比：时间延长值 = 原持续时间 x （属性差值 x 效果数值 / 100） / 100

                int theChangeValue = 0;

                if (pEff->innerExec->changeType == 1) {
                    theChangeValue = tickLen * attrDiff * pEff->getUndividedChangeValue() / 10000;
                }
                else {
                    theChangeValue = attrDiff * pEff->getUndividedChangeValue() / 100;
                }

                theChangeValue = pEff->getBoundedUndividedChangeValue(theChangeValue);
                theChangeValue /= 100;

                changeValue += theChangeValue;
            }

            if (changeValue > 100)
                changeValue = 100;
            else if (changeValue < 0 && changeValue + tickLen < 10)
                changeValue = 10 - tickLen;

            return changeValue;
        }

        void checkActivateReviveEffect(AIContext &aiContext, CharacterData *target)
        {
            auto iter = target->m_aiEffectMap.find(eEFFECT_REVIVE);
            if (iter == target->m_aiEffectMap.end())
                return;

            for (auto &&pEff : iter->second)
            {
                if (!pEff || pEff->isTempInvalid()) continue;
                for (auto execID : pEff->innerExec->params)
                {
                    aiContext.scheduleExcute(execID, pEff->skillID, pEff->skillLevel, pEff->origin, pEff->projectile, target, 0);
                }
            }
        }
        
        void reshowUIEffectsWhenCharacterRevive(AIContext &aiContext, CharacterData *t)
        {
            for (auto iter = t->m_aiEffectMap.begin(); iter != t->m_aiEffectMap.end(); ++iter) {
                for (auto anEff : iter->second) {
                    int effID = anEff->innerExec->ID;
                    if ((effID == 200 || effID == 201 || effID == 999) && anEff->parentCaster->staticData->stateAnimation > 0)
                        continue;
                    int remainTick = anEff->parentCaster->totalTick - anEff->parentCaster->localTick + anEff->parentCaster->startTick;
                    if (aiContext.seffProxy)
                        aiContext.seffProxy->notify(aiContext, anEff->getUID());
                    anEff->castUIEffect(remainTick);
                }
            }
        }

		void limitAttr(AIContext *ai, CharacterData *c, int idx)
		{
			if (c->hasEffect(325))
			{
				luahelper_innereffect_325_checkdo(ai->getLuaState(), c, idx);
			}
		}

		void collect_innereffect_record(AIInnerEffect *eff)
		{
			CharacterData *c = eff->target;
			int effid = eff->innerExec->ID;
			eff->parentCaster->removeInnerEffRecord(c->myID, effid);
			eff->onEraseEffect();

			std::map<int, std::vector<AIInnerEffect *>>::iterator iter = c->m_aiEffectMap.find(effid);
			if (iter != c->m_aiEffectMap.end()) {
				for (std::vector<AIInnerEffect *>::iterator it = iter->second.begin(); it != iter->second.end(); ++it) {
					if (*it == eff) {
						iter->second.erase(it);
						break;
					}
				}

				if (c->m_aiEffectMap.size() == 0)
					c->m_aiEffectMap.erase(iter);
			}
		}

    } // namespace EffectHelper

} // namespace OnePiece
